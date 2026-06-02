// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Vehicles/NIOVehicleBase.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "NomiRacingPlus.h"

ANIOVehicleBase::ANIOVehicleBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create state manager
	StateManager = CreateDefaultSubobject<UVehicleStateManager>(TEXT("StateManager"));

	// Note: NIOVehicleMovementComponent should be set in Blueprint
	// as the default movement component for WheeledVehiclePawn

	// Create headlight components
	LeftHeadlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("LeftHeadlight"));
	LeftHeadlight->SetupAttachment(GetMesh());
	LeftHeadlight->SetIntensity(0.0f); // Off by default
	LeftHeadlight->SetOuterConeAngle(45.0f);
	LeftHeadlight->SetInnerConeAngle(30.0f);
	LeftHeadlight->SetAttenuationRadius(5000.0f);
	LeftHeadlight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.8f)); // Warm white

	RightHeadlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("RightHeadlight"));
	RightHeadlight->SetupAttachment(GetMesh());
	RightHeadlight->SetIntensity(0.0f);
	RightHeadlight->SetOuterConeAngle(45.0f);
	RightHeadlight->SetInnerConeAngle(30.0f);
	RightHeadlight->SetAttenuationRadius(5000.0f);
	RightHeadlight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.8f));

	// Create taillight components
	LeftTaillight = CreateDefaultSubobject<UPointLightComponent>(TEXT("LeftTaillight"));
	LeftTaillight->SetupAttachment(GetMesh());
	LeftTaillight->SetIntensity(0.0f);
	LeftTaillight->SetAttenuationRadius(1000.0f);
	LeftTaillight->SetLightColor(FLinearColor(1.0f, 0.0f, 0.0f)); // Red

	RightTaillight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RightTaillight"));
	RightTaillight->SetupAttachment(GetMesh());
	RightTaillight->SetIntensity(0.0f);
	RightTaillight->SetAttenuationRadius(1000.0f);
	RightTaillight->SetLightColor(FLinearColor(1.0f, 0.0f, 0.0f));
}

void ANIOVehicleBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeComponents();

	UE_LOG(LogNomiVehicle, Log, TEXT("NIO Vehicle initialized: %s"), *GetVehicleDisplayName());
}

void ANIOVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateLights(DeltaTime);
	UpdateAudio(DeltaTime);
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

const FVehicleState& ANIOVehicleBase::GetVehicleState() const
{
	if (StateManager)
	{
		return StateManager->GetVehicleState();
	}

	static FVehicleState EmptyState;
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
		const FVehicleState& State = StateManager->GetVehicleState();
		float TaillightIntensity = MaxTaillightIntensity * 0.3f; // Dim when driving
		TaillightIntensity += MaxTaillightIntensity * 0.7f * State.BrakeInput; // Bright when braking
		LeftTaillight->SetIntensity(TaillightIntensity);
	}

	if (RightTaillight && StateManager)
	{
		const FVehicleState& State = StateManager->GetVehicleState();
		float TaillightIntensity = MaxTaillightIntensity * 0.3f;
		TaillightIntensity += MaxTaillightIntensity * 0.7f * State.BrakeInput;
		RightTaillight->SetIntensity(TaillightIntensity);
	}
}

void ANIOVehicleBase::UpdateAudio(float DeltaTime)
{
	// Motor sound would be handled by MetaSound in the actual implementation
	// This is a placeholder for the audio system integration
}
