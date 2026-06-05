// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NIOVehicleBase.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "NomiRacingPlus.h"

ANIOVehicleBase::ANIOVehicleBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UNIOVehicleMovementComponent>(AWheeledVehiclePawn::VehicleMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// Create state manager
	StateManager = CreateDefaultSubobject<UVehicleStateManager>(TEXT("StateManager"));

	// Create camera system
	CameraSystemComponent = CreateDefaultSubobject<UCameraSystem>(TEXT("CameraSystem"));

	// Get the NIO movement component (created by parent via ObjectInitializer)
	NIOMovement = Cast<UNIOVehicleMovementComponent>(GetVehicleMovement());

	// Clear wheel bone names - we don't have a skeletal mesh with wheel bones.
	// The vehicle will use simplified movement when no mesh is present.
	if (NIOMovement)
	{
		NIOMovement->WheelSetups.SetNum(4);
		NIOMovement->WheelSetups[0].BoneName = NAME_None;
		NIOMovement->WheelSetups[1].BoneName = NAME_None;
		NIOMovement->WheelSetups[2].BoneName = NAME_None;
		NIOMovement->WheelSetups[3].BoneName = NAME_None;
	}

	// Create vehicle display mesh component (visual-only, no collision — physics handled by root)
	// NOTE: Parent class AWheeledVehiclePawn already owns "VehicleMesh" (SkeletalMeshComponent), so we use a unique name.
	VehicleDisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleDisplayMesh"));
	VehicleDisplayMesh->SetupAttachment(GetMesh() ? Cast<USceneComponent>(GetMesh()) : GetRootComponent());
	VehicleDisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VehicleDisplayMesh->SetVisibility(true);

	// Create headlight components (attach to mesh if available, otherwise to root)
	USceneComponent* AttachTarget = GetMesh() ? Cast<USceneComponent>(GetMesh()) : GetRootComponent();

	LeftHeadlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("LeftHeadlight"));
	LeftHeadlight->SetupAttachment(AttachTarget);
	LeftHeadlight->SetIntensity(0.0f); // Off by default
	LeftHeadlight->SetOuterConeAngle(45.0f);
	LeftHeadlight->SetInnerConeAngle(30.0f);
	LeftHeadlight->SetAttenuationRadius(5000.0f);
	LeftHeadlight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.8f)); // Warm white

	RightHeadlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("RightHeadlight"));
	RightHeadlight->SetupAttachment(AttachTarget);
	RightHeadlight->SetIntensity(0.0f);
	RightHeadlight->SetOuterConeAngle(45.0f);
	RightHeadlight->SetInnerConeAngle(30.0f);
	RightHeadlight->SetAttenuationRadius(5000.0f);
	RightHeadlight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.8f));

	// Create taillight components
	LeftTaillight = CreateDefaultSubobject<UPointLightComponent>(TEXT("LeftTaillight"));
	LeftTaillight->SetupAttachment(AttachTarget);
	LeftTaillight->SetIntensity(0.0f);
	LeftTaillight->SetAttenuationRadius(1000.0f);
	LeftTaillight->SetLightColor(FLinearColor(1.0f, 0.0f, 0.0f)); // Red

	RightTaillight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RightTaillight"));
	RightTaillight->SetupAttachment(AttachTarget);
	RightTaillight->SetIntensity(0.0f);
	RightTaillight->SetAttenuationRadius(1000.0f);
	RightTaillight->SetLightColor(FLinearColor(1.0f, 0.0f, 0.0f));
}

void ANIOVehicleBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeComponents();

	// Check if Chaos Vehicle has a valid skeletal mesh
	if (NIOMovement)
	{
		USkeletalMeshComponent* SkelMesh = GetMesh();
		bChaosVehicleValid = SkelMesh && SkelMesh->GetSkeletalMeshAsset() != nullptr;
	}

	if (!bChaosVehicleValid)
	{
		UE_LOG(LogNomiVehicle, Warning, TEXT("No skeletal mesh - using simplified movement"));

		// Enable physics on root component for simple movement
		if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetRootComponent()))
		{
			Root->SetSimulatePhysics(true);
			Root->SetEnableGravity(true);
			Root->SetLinearDamping(0.5f);
			Root->SetAngularDamping(2.0f);
		}
	}

	// Manage vehicle display mesh visibility:
	// - When Chaos Vehicle has a SkeletalMesh, hide our StaticMesh (redundant)
	// - When no SkeletalMesh (GLB import), show the StaticMesh for visual feedback
	if (VehicleDisplayMesh)
	{
		VehicleDisplayMesh->SetVisibility(!bChaosVehicleValid);
	}

	// Bind collision event for race statistics
	OnActorHit.AddDynamic(this, &ANIOVehicleBase::OnVehicleHit);

	UE_LOG(LogNomiVehicle, Log, TEXT("NIO Vehicle initialized: %s (ChaosVehicle: %s)"),
		*GetVehicleDisplayName(), bChaosVehicleValid ? TEXT("Yes") : TEXT("No"));
}

void ANIOVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateLights(DeltaTime);
	UpdateAudio(DeltaTime);

	// Use simple movement if Chaos Vehicle is not available
	if (!bChaosVehicleValid)
	{
		UpdateSimpleMovement(DeltaTime);
	}

	// Track race statistics
	if (StateManager)
	{
		float CurrentSpeed = StateManager->GetVehicleState().SpeedKmh;
		if (CurrentSpeed > MaxSpeedKmh)
		{
			MaxSpeedKmh = CurrentSpeed;
		}

		// Track distance (speed in km/h * time in hours * 1000 = meters)
		DistanceDriven += CurrentSpeed * DeltaTime / 3600.0f * 1000.0f;
	}
}

void ANIOVehicleBase::ResetRaceStats()
{
	CollisionCount = 0;
	MaxSpeedKmh = 0.0f;
	DistanceDriven = 0.0f;
	OvertakeCount = 0;
}

void ANIOVehicleBase::UpdateSimpleMovement(float DeltaTime)
{
	UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(GetRootComponent());
	if (!Root)
	{
		return;
	}

	// Get forward direction
	FVector Forward = GetActorForwardVector();
	FVector Right = GetActorRightVector();

	// Calculate forces
	float AccelerationForce = 300000.0f;  // Adjust for feel
	float BrakeForce = 500000.0f;
	float TurnRate = 2.0f;

	// Apply throttle
	if (FMath::Abs(ThrottleInput) > 0.01f)
	{
		FVector Force = Forward * ThrottleInput * AccelerationForce * DeltaTime;
		Root->AddForce(Force);
	}

	// Apply brake
	if (BrakeInput > 0.01f)
	{
		FVector Velocity = Root->GetComponentVelocity();
		FVector BrakeForceVec = -Velocity.GetSafeNormal() * BrakeInput * BrakeForce * DeltaTime;
		Root->AddForce(BrakeForceVec);
	}

	// Apply steering (rotate the actor)
	if (FMath::Abs(SteeringInput) > 0.01f)
	{
		float Speed = Root->GetComponentVelocity().Size();
		if (Speed > 100.0f)  // Only steer when moving
		{
			float YawRotation = SteeringInput * TurnRate * DeltaTime * FMath::Clamp(Speed / 1000.0f, 0.3f, 1.0f);
			AddActorLocalRotation(FRotator(0.0f, YawRotation, 0.0f));
		}
	}

	// Handbrake
	if (bHandbrake)
	{
		FVector Velocity = Root->GetComponentVelocity();
		FVector LateralVelocity = Right * FVector::DotProduct(Velocity, Right);
		Root->AddForce(-LateralVelocity * 0.5f);  // Kill lateral sliding
	}
}

void ANIOVehicleBase::OnVehicleHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
	// Count collisions for race statistics
	if (OtherActor && OtherActor != this)
	{
		CollisionCount++;
		UE_LOG(LogNomiVehicle, Verbose, TEXT("Vehicle collision #%d with %s"), CollisionCount, *OtherActor->GetName());
	}
}

void ANIOVehicleBase::InitializeComponents()
{
	// Configure state manager
	if (StateManager)
	{
		StateManager->SetVehicleType(VehicleType);
	}

	// Find and configure NIO movement component
	NIOMovement = FindComponentByClass<UNIOVehicleMovementComponent>();
	if (NIOMovement)
	{
		NIOMovement->ConfigureForNIOVehicle(VehicleType);
	}
	else
	{
		UE_LOG(LogNomiVehicle, Warning, TEXT("NIO Vehicle missing NIOVehicleMovementComponent!"));
	}
}

const FNIOVehicleState& ANIOVehicleBase::GetVehicleState() const
{
	if (StateManager)
	{
		return StateManager->GetVehicleState();
	}

	static FNIOVehicleState EmptyState;
	return EmptyState;
}

FString ANIOVehicleBase::GetVehicleDisplayName() const
{
	if (StateManager)
	{
		return StateManager->GetVehicleDisplayName();
	}
	return TEXT("Unknown Vehicle");
}

bool ANIOVehicleBase::IsNIOVehicle() const
{
	return VehicleType != ENIOVehicleType::Custom;
}

const FNIOPerformanceConfig& ANIOVehicleBase::GetPerformanceConfig() const
{
	if (StateManager)
	{
		return StateManager->GetPerformanceConfig();
	}

	static FNIOPerformanceConfig EmptyConfig;
	return EmptyConfig;
}

void ANIOVehicleBase::SetPerformanceConfig(const FNIOPerformanceConfig& InConfig)
{
	if (StateManager)
	{
		StateManager->SetPerformanceConfig(InConfig);
	}
}

void ANIOVehicleBase::PlayHorn()
{
	if (HornSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HornSound, GetActorLocation());
	}
}

void ANIOVehicleBase::ToggleHeadlights()
{
	bHeadlightsOn = !bHeadlightsOn;

	if (bHeadlightsOn)
	{
		HeadlightBrightness = 1.0f;
	}
	else
	{
		HeadlightBrightness = 0.0f;
	}
}

void ANIOVehicleBase::SetHeadlightBrightness(float Brightness)
{
	HeadlightBrightness = FMath::Clamp(Brightness, 0.0f, 1.0f);
	bHeadlightsOn = HeadlightBrightness > 0.0f;
}

void ANIOVehicleBase::UpdateLights(float DeltaTime)
{
	const float MaxHeadlightIntensity = 50000.0f;
	const float MaxTaillightIntensity = 500.0f;

	// Update headlights
	if (LeftHeadlight)
	{
		LeftHeadlight->SetIntensity(bHeadlightsOn ? MaxHeadlightIntensity * HeadlightBrightness : 0.0f);
	}

	if (RightHeadlight)
	{
		RightHeadlight->SetIntensity(bHeadlightsOn ? MaxHeadlightIntensity * HeadlightBrightness : 0.0f);
	}

	// Update taillights (brighter when braking)
	if (LeftTaillight && StateManager)
	{
		const FNIOVehicleState& State = StateManager->GetVehicleState();
		float TaillightIntensity = MaxTaillightIntensity * 0.3f; // Dim when driving
		TaillightIntensity += MaxTaillightIntensity * 0.7f * State.BrakeInput; // Bright when braking
		LeftTaillight->SetIntensity(TaillightIntensity);
	}

	if (RightTaillight && StateManager)
	{
		const FNIOVehicleState& State = StateManager->GetVehicleState();
		float TaillightIntensity = MaxTaillightIntensity * 0.3f;
		TaillightIntensity += MaxTaillightIntensity * 0.7f * State.BrakeInput;
		RightTaillight->SetIntensity(TaillightIntensity);
	}
}

void ANIOVehicleBase::UpdateAudio(float DeltaTime)
{
	if (!StateManager)
	{
		return;
	}

	const FNIOVehicleState& State = StateManager->GetVehicleState();

	// Smooth RPM transitions for natural audio feel
	const float RPMInterpSpeed = 5.0f;
	CachedMotorRPM = FMath::FInterpTo(CachedMotorRPM, State.RPM, DeltaTime, RPMInterpSpeed);

	if (MotorSound)
	{
		// Lazily create the motor audio component
		if (!MotorAudioComponent)
		{
			MotorAudioComponent = NewObject<UAudioComponent>(this, TEXT("MotorAudio"));
			if (MotorAudioComponent)
			{
				MotorAudioComponent->SetSound(MotorSound);
				MotorAudioComponent->bAutoActivate = true;
				MotorAudioComponent->bAutoDestroy = false;
				MotorAudioComponent->SetVolumeMultiplier(0.0f);
				MotorAudioComponent->RegisterComponent();
				MotorAudioComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			}
		}

		if (MotorAudioComponent && MotorAudioComponent->IsPlaying())
		{
			// Map RPM to pitch: idle ~0.8, redline ~1.6 (electric motor whine)
			const float MinPitch = 0.8f;
			const float MaxPitch = 1.6f;
			// Use fixed max RPM for consistent pitch mapping (don't use live RPM as denominator)
			const float MaxMotorRPM = 8000.0f;
			float RPMRatio = FMath::Clamp(CachedMotorRPM / MaxMotorRPM, 0.0f, 1.0f);
			// Use speed-based ratio instead when RPM data is unreliable
			if (CachedMotorRPM < 1.0f)
			{
				RPMRatio = FMath::Clamp(State.SpeedKmh / 250.0f, 0.0f, 1.0f);
			}
			MotorAudioComponent->SetPitchMultiplier(FMath::Lerp(MinPitch, MaxPitch, RPMRatio));

			// Volume based on throttle input: louder under acceleration, quieter when coasting
			const float MinVolume = 0.15f; // Quiet idle/whine
			const float MaxVolume = 1.0f;
			float VolumeTarget = FMath::Lerp(MinVolume, MaxVolume, FMath::Abs(State.ThrottleInput));
			MotorAudioComponent->SetVolumeMultiplier(VolumeTarget);
		}
		else if (MotorAudioComponent && !MotorAudioComponent->IsPlaying() && MotorSound)
		{
			MotorAudioComponent->Play();
		}
	}
	else if (MotorAudioComponent)
	{
		// Motor sound asset removed - clean up the component
		MotorAudioComponent->Stop();
		MotorAudioComponent->DestroyComponent();
		MotorAudioComponent = nullptr;
	}
}
