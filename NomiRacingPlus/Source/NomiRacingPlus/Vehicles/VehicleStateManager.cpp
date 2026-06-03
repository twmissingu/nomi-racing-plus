// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "VehicleStateManager.h"
#include "NIOVehicleMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NomiRacingPlus.h"
#include "Race/RaceManager.h"
#include "Race/CheckpointSystem.h"
#include "Kismet/GameplayStatics.h"

UVehicleStateManager::UVehicleStateManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UVehicleStateManager::BeginPlay()
{
	Super::BeginPlay();

	// Find vehicle root component and cache component references
	if (AActor* Owner = GetOwner())
	{
		VehicleRoot = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
		CachedChaosVehicle = Owner->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
		CachedNIOMovement = Owner->FindComponentByClass<UNIOVehicleMovementComponent>();
		LastPosition = Owner->GetActorLocation();

		// Cache RaceManager for performance
		CachedRaceManager = Cast<ARaceManager>(UGameplayStatics::GetActorOfClass(this, ARaceManager::StaticClass()));
	}

	// Apply default performance config based on vehicle type
	switch (VehicleType)
	{
	case ENIOVehicleType::EP9:
		PerformanceConfig.MassKg = 1735.0f;
		PerformanceConfig.PowerKw = 1000.0f;
		PerformanceConfig.TorqueNm = 1480.0f;
		PerformanceConfig.DriveType = TEXT("AWD_quad_motor");
		PerformanceConfig.TopSpeedKph = 313.0f;
		PerformanceConfig.Acceleration0100 = 2.7f;
		PerformanceConfig.DownforceMaxKg = 2000.0f;
		PerformanceConfig.WheelbaseMm = 2750.0f;
		PerformanceConfig.BodyType = TEXT("hypercar");
		break;

	case ENIOVehicleType::ET7:
		PerformanceConfig.MassKg = 2379.0f;
		PerformanceConfig.PowerKw = 480.0f;
		PerformanceConfig.TorqueNm = 850.0f;
		PerformanceConfig.DriveType = TEXT("AWD_dual_motor");
		PerformanceConfig.TopSpeedKph = 250.0f;
		PerformanceConfig.Acceleration0100 = 3.8f;
		PerformanceConfig.WheelbaseMm = 3060.0f;
		PerformanceConfig.BodyType = TEXT("sedan");
		break;

	case ENIOVehicleType::ES7:
		PerformanceConfig.MassKg = 2400.0f;
		PerformanceConfig.PowerKw = 480.0f;
		PerformanceConfig.TorqueNm = 850.0f;
		PerformanceConfig.DriveType = TEXT("AWD_dual_motor");
		PerformanceConfig.TopSpeedKph = 200.0f;
		PerformanceConfig.Acceleration0100 = 3.9f;
		PerformanceConfig.WheelbaseMm = 2960.0f;
		PerformanceConfig.BodyType = TEXT("suv");
		break;

	case ENIOVehicleType::ET5:
		PerformanceConfig.MassKg = 2070.0f;
		PerformanceConfig.PowerKw = 360.0f;
		PerformanceConfig.TorqueNm = 700.0f;
		PerformanceConfig.DriveType = TEXT("AWD_dual_motor");
		PerformanceConfig.TopSpeedKph = 200.0f;
		PerformanceConfig.Acceleration0100 = 4.0f;
		PerformanceConfig.WheelbaseMm = 2888.0f;
		PerformanceConfig.BodyType = TEXT("sedan");
		break;

	default:
		break;
	}

	UE_LOG(LogNomiVehicle, Log, TEXT("VehicleStateManager initialized: %s (%s)"),
		*GetVehicleDisplayName(),
		*PerformanceConfig.BodyType);
}

void UVehicleStateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateStateFromPhysics(DeltaTime);
	CheckStuckAndFlip(DeltaTime);
}

namespace VehicleConstants
{
	constexpr float CmPerSecToKmPerHour = 0.036f;  // 1 cm/s = 0.036 km/h
	constexpr float DriftAngleThreshold = 15.0f;    // degrees
	constexpr float MinSpeedForSlipAngle = 5.0f;    // km/h
	constexpr float BatteryDrainRate = 0.01f;       // %/s at full throttle
	constexpr float BatteryRegenRate = 0.005f;      // %/s at full regen
}

void UVehicleStateManager::UpdateStateFromPhysics(float DeltaTime)
{
	if (!GetOwner())
	{
		return;
	}

	// Update position and rotation
	VehicleState.Position = GetOwner()->GetActorLocation();
	VehicleState.Rotation = GetOwner()->GetActorRotation();

	// Update velocity
	if (VehicleRoot)
	{
		VehicleState.Velocity = VehicleRoot->GetComponentVelocity();
		VehicleState.SpeedKmh = VehicleState.Velocity.Size() * VehicleConstants::CmPerSecToKmPerHour;
	}

	// Try to get Chaos Vehicle data (using cached reference)
	if (CachedChaosVehicle)
	{
		VehicleState.ThrottleInput = CachedChaosVehicle->GetThrottleInput();
		VehicleState.BrakeInput = CachedChaosVehicle->GetBrakeInput();
		VehicleState.SteeringInput = CachedChaosVehicle->GetSteeringInput();
		VehicleState.RPM = CachedChaosVehicle->GetEngineRotationSpeed();
		VehicleState.Gear = CachedChaosVehicle->GetCurrentGear();
	}

	// Calculate slip angle for drift detection
	VehicleState.SlipAngle = CalculateSlipAngle();

	// Drift detection
	VehicleState.bIsDrifting = FMath::Abs(VehicleState.SlipAngle) > VehicleConstants::DriftAngleThreshold;

	// Ground check (simplified - check if vehicle is not falling)
	VehicleState.bIsGrounded = VehicleRoot ? VehicleRoot->IsSimulatingPhysics() : true;

	// Battery simulation (slowly decrease over time, regenerate on braking)
	if (PerformanceConfig.bIsElectric)
	{
		float BatteryDrain = VehicleState.ThrottleInput * DeltaTime * VehicleConstants::BatteryDrainRate;
		float BatteryRegen = VehicleState.BrakeInput * PerformanceConfig.RegenBrakingStrength * DeltaTime * VehicleConstants::BatteryRegenRate;
		VehicleState.BatteryPercent = FMath::Clamp(VehicleState.BatteryPercent - BatteryDrain + BatteryRegen, 0.0f, 100.0f);
	}

	// Update tire telemetry from Pacejka model (using cached reference)
	if (CachedNIOMovement)
	{
		if (UTirePhysicsModel* TireModel = CachedNIOMovement->GetTirePhysicsModel())
		{
			VehicleState.AverageTireTemperature = TireModel->GetAverageTireTemperature();
			VehicleState.bAnyTireSlipping = TireModel->IsAnyTireSlipping(0.15f);

			// Calculate max slip values and average wear
			float TotalWear = 0.0f;
			float MaxSlipR = 0.0f;
			float MaxSlipA = 0.0f;
			const TArray<FTireState>& TireStates = TireModel->GetAllTireStates();

			for (const FTireState& TireState : TireStates)
			{
				TotalWear += TireState.Thermal.WearFactor;
				MaxSlipR = FMath::Max(MaxSlipR, FMath::Abs(TireState.SlipRatio));
				MaxSlipA = FMath::Max(MaxSlipA, FMath::Abs(TireState.SlipAngleDeg));
			}

			if (TireStates.Num() > 0)
			{
				VehicleState.AverageTireWear = TotalWear / static_cast<float>(TireStates.Num());
			}

			VehicleState.MaxSlipRatio = MaxSlipR;
			VehicleState.MaxSlipAngleDeg = MaxSlipA;

			// Update drift detection with tire model data
			VehicleState.bIsDrifting = VehicleState.bIsDrifting ||
				(MaxSlipA > VehicleConstants::DriftAngleThreshold && VehicleState.SpeedKmh > 20.0f);
		}
	}
}

float UVehicleStateManager::CalculateSlipAngle() const
{
	if (VehicleState.SpeedKmh < 5.0f || !GetOwner())
	{
		return 0.0f;
	}

	// Calculate angle between velocity direction and vehicle forward direction
	FVector ForwardVector = GetOwner()->GetActorForwardVector();
	FVector VelocityDirection = VehicleState.Velocity.GetSafeNormal();

	// Calculate signed angle in degrees
	float DotProduct = FVector::DotProduct(ForwardVector, VelocityDirection);
	float CrossProductZ = FVector::CrossProduct(ForwardVector, VelocityDirection).Z;

	return FMath::Atan2(CrossProductZ, DotProduct) * (180.0f / PI);
}

bool UVehicleStateManager::IsNIOVehicle() const
{
	return VehicleType != ENIOVehicleType::Custom;
}

FString UVehicleStateManager::GetVehicleDisplayName() const
{
	switch (VehicleType)
	{
	case ENIOVehicleType::EP9: return TEXT("NIO EP9");
	case ENIOVehicleType::ET7: return TEXT("NIO ET7");
	case ENIOVehicleType::ES7: return TEXT("NIO ES7");
	case ENIOVehicleType::ET5: return TEXT("NIO ET5");
	case ENIOVehicleType::SU7Ultra: return TEXT("Xiaomi SU7 Ultra");
	default: return TEXT("Custom Vehicle");
	}
}

void UVehicleStateManager::CheckStuckAndFlip(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld())
	{
		return;
	}

	const FVector CurrentPos = Owner->GetActorLocation();

	// --- Stuck detection ---
	if (FVector::Dist(CurrentPos, LastPosition) < StuckDistanceThreshold)
	{
		StuckTimer += DeltaTime;
	}
	else
	{
		StuckTimer = 0.0f;
	}

	const bool bWasStuck = bIsStuck;
	if (StuckTimer > StuckThreshold)
	{
		bIsStuck = true;
	}

	// --- Flip detection ---
	const FRotator Rotation = Owner->GetActorRotation();
	const bool bAngleExceeded =
		FMath::Abs(Rotation.Roll) > FlipAngleThreshold ||
		FMath::Abs(Rotation.Pitch) > FlipAngleThreshold;

	if (bAngleExceeded)
	{
		FlipTimer += DeltaTime;
	}
	else
	{
		FlipTimer = 0.0f;
	}

	const bool bWasFlipped = bIsFlipped;
	if (FlipTimer > 1.0f)
	{
		bIsFlipped = true;
	}

	// --- Update last position ---
	LastPosition = CurrentPos;

	// --- Broadcast events on state change ---
	if ((!bWasStuck && bIsStuck) || (!bWasFlipped && bIsFlipped))
	{
		ARaceManager* RaceManager = Cast<ARaceManager>(
			UGameplayStatics::GetActorOfClass(this, ARaceManager::StaticClass()));
		if (RaceManager)
		{
			FRacerData RacerData;
			RacerData.VehiclePawn = Cast<APawn>(Owner);

			if (!bWasStuck && bIsStuck)
			{
				RaceManager->OnRaceEvent.Broadcast(ERaceEvent::VehicleStuck, RacerData);
				UE_LOG(LogNomiVehicle, Warning, TEXT("Vehicle stuck detected: %s"), *GetVehicleDisplayName());
			}
			if (!bWasFlipped && bIsFlipped)
			{
				RaceManager->OnRaceEvent.Broadcast(ERaceEvent::VehicleFlipped, RacerData);
				UE_LOG(LogNomiVehicle, Warning, TEXT("Vehicle flipped detected: %s"), *GetVehicleDisplayName());
			}
		}
	}
}

void UVehicleStateManager::ResetVehicle()
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld())
	{
		return;
	}

	// Find nearest checkpoint to teleport to
	FVector ResetLocation = Owner->GetActorLocation();
	FRotator ResetRotation = FRotator(0.0f, Owner->GetActorRotation().Yaw, 0.0f);

	// Use cached RaceManager if available, otherwise fall back to GetAllActorsOfClass
	TArray<AActor*> Checkpoints;
	if (CachedRaceManager.IsValid())
	{
		// Get checkpoints from RaceManager's checkpoint system
		if (UCheckpointSystem* CheckpointSys = CachedRaceManager->FindComponentByClass<UCheckpointSystem>())
		{
			CheckpointSys->GetAllCheckpoints(Checkpoints);
		}
	}

	if (Checkpoints.Num() == 0)
	{
		UGameplayStatics::GetAllActorsOfClass(this, ACheckpoint::StaticClass(), Checkpoints);
	}

	if (Checkpoints.Num() > 0)
	{
		// Find the nearest checkpoint
		float BestDistSqr = TNumericLimits<float>::Max();
		ACheckpoint* NearestCheckpoint = nullptr;

		for (AActor* Actor : Checkpoints)
		{
			const float DistSqr = FVector::DistSquared(Owner->GetActorLocation(), Actor->GetActorLocation());
			if (DistSqr < BestDistSqr)
			{
				BestDistSqr = DistSqr;
				NearestCheckpoint = Cast<ACheckpoint>(Actor);
			}
		}

		if (NearestCheckpoint)
		{
			ResetLocation = NearestCheckpoint->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);

			// Orient along track: find the next checkpoint to compute forward direction
			ACheckpoint* NextCheckpoint = nullptr;
			for (AActor* Actor : Checkpoints)
			{
				if (ACheckpoint* CP = Cast<ACheckpoint>(Actor))
				{
					if (CP->CheckpointIndex == NearestCheckpoint->CheckpointIndex + 1)
					{
						NextCheckpoint = CP;
						break;
					}
				}
			}

			if (NextCheckpoint)
			{
				const FVector Forward = (NextCheckpoint->GetActorLocation() - NearestCheckpoint->GetActorLocation()).GetSafeNormal();
				ResetRotation = Forward.Rotation();
				ResetRotation.Pitch = 0.0f;
				ResetRotation.Roll = 0.0f;
			}
		}
	}
	else
	{
		// Fallback: ground trace at current location
		FHitResult HitResult;
		FVector TraceStart = ResetLocation + FVector(0.0f, 0.0f, 500.0f);
		FVector TraceEnd = ResetLocation - FVector(0.0f, 0.0f, 2000.0f);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
		{
			ResetLocation = HitResult.ImpactPoint + FVector(0.0f, 0.0f, 100.0f);
		}
	}

	// Teleport vehicle
	Owner->SetActorLocation(ResetLocation);
	Owner->SetActorRotation(ResetRotation);

	// Zero out velocity
	if (VehicleRoot && VehicleRoot->IsSimulatingPhysics())
	{
		VehicleRoot->SetPhysicsLinearVelocity(FVector::ZeroVector);
		VehicleRoot->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	// Reset recovery state
	StuckTimer = 0.0f;
	FlipTimer = 0.0f;
	bIsStuck = false;
	bIsFlipped = false;

	UE_LOG(LogNomiVehicle, Log, TEXT("Vehicle reset to nearest waypoint: %s"), *GetVehicleDisplayName());
}
