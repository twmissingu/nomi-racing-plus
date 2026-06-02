// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "AISensorSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "NomiRacingPlus.h"

UAISensorSystem::UAISensorSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UAISensorSystem::BeginPlay()
{
	Super::BeginPlay();

	OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		IgnoredActors.Add(OwnerPawn);
	}
}

void UAISensorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerPawn)
	{
		return;
	}

	ScanTimer += DeltaTime;
	if (ScanTimer >= Config.UpdateInterval)
	{
		ScanTimer = 0.0f;
		PerformScan(DeltaTime);
	}
}

void UAISensorSystem::ForceScan()
{
	PerformScan(0.0f);
}

void UAISensorSystem::PerformScan(float DeltaTime)
{
	// Reset data
	SensorData = FAISensorData();

	ScanForward();
	ScanRear();
	ScanSides();
	DetectSlipstream();
	ScanTrackBoundaries();
}

bool UAISensorSystem::TraceForVehicle(const FVector& Start, const FVector& Direction, float Range, FAIDetectedVehicle& OutVehicle) const
{
	UWorld* World = GetWorld();
	if (!World || !OwnerPawn)
	{
		return false;
	}

	FHitResult HitResult;
	FVector End = Start + Direction * Range;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoredActors);
	QueryParams.bTraceComplex = false;

	bool bHit = World->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(50.0f), // 0.5m sphere for detection
		QueryParams
	);

	if (bHit && HitResult.GetActor())
	{
		APawn* HitPawn = Cast<APawn>(HitResult.GetActor());
		if (HitPawn && HitPawn != OwnerPawn)
		{
			float Distance = HitResult.Distance;
			FVector ToTarget = (HitPawn->GetActorLocation() - OwnerPawn->GetActorLocation());
			float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(
				FVector::DotProduct(OwnerPawn->GetActorForwardVector(), ToTarget.GetSafeNormal())
			));

			PopulateDetectedVehicle(HitPawn, Distance, AngleDeg, AngleDeg < 90.0f, OutVehicle);
			return true;
		}
	}

	return false;
}

void UAISensorSystem::PopulateDetectedVehicle(APawn* OtherVehicle, float Distance, float AngleDeg, bool bAhead, FAIDetectedVehicle& OutVehicle) const
{
	if (!OwnerPawn || !OtherVehicle)
	{
		return;
	}

	OutVehicle.Vehicle = OtherVehicle;
	OutVehicle.Distance = Distance;
	OutVehicle.AngleDeg = AngleDeg;
	OutVehicle.bIsAhead = bAhead;
	OutVehicle.bIsBehind = !bAhead;

	// Calculate lateral offset
	FVector ToOther = OtherVehicle->GetActorLocation() - OwnerPawn->GetActorLocation();
	FVector Right = OwnerPawn->GetActorRightVector();
	OutVehicle.LateralOffset = FVector::DotProduct(Right, ToOther.GetSafeNormal());

	// Calculate relative speed
	FVector OwnerVelocity = OwnerPawn->GetVelocity();
	FVector OtherVelocity = OtherVehicle->GetVelocity();
	FVector RelativeVelocity = OwnerVelocity - OtherVelocity;
	FVector Forward = OwnerPawn->GetActorForwardVector();
	OutVehicle.RelativeSpeed = FVector::DotProduct(RelativeVelocity, Forward);
}

void UAISensorSystem::ScanForward()
{
	if (!OwnerPawn)
	{
		return;
	}

	FVector Start = OwnerPawn->GetActorLocation();
	FVector Forward = OwnerPawn->GetActorForwardVector();
	FVector Right = OwnerPawn->GetActorRightVector();

	// Direct forward trace
	FAIDetectedVehicle DirectAhead;
	if (TraceForVehicle(Start, Forward, Config.ForwardRange, DirectAhead))
	{
		SensorData.VehicleAhead = DirectAhead;
		SensorData.bHasVehicleAhead = true;
		SensorData.NearbyVehicles.Add(DirectAhead);
	}

	// Swept forward traces for wider detection
	if (Config.ForwardRayCount > 1)
	{
		float AngleStep = (Config.ForwardSweepAngle * 2.0f) / (Config.ForwardRayCount - 1);

		for (int32 i = 0; i < Config.ForwardRayCount; ++i)
		{
			float Angle = -Config.ForwardSweepAngle + AngleStep * i;
			FVector SweepDir = Forward.RotateAngleAxis(Angle, FVector::UpVector);

			FAIDetectedVehicle SweptVehicle;
			if (TraceForVehicle(Start, SweepDir, Config.ForwardRange, SweptVehicle))
			{
				// Track the closest vehicle ahead
				if (!SensorData.bHasVehicleAhead || SweptVehicle.Distance < SensorData.VehicleAhead.Distance)
				{
					SensorData.VehicleAhead = SweptVehicle;
					SensorData.bHasVehicleAhead = true;
				}

				// Add to nearby if not duplicate
				bool bAlreadyTracked = false;
				for (const FAIDetectedVehicle& Existing : SensorData.NearbyVehicles)
				{
					if (Existing.Vehicle == SweptVehicle.Vehicle)
					{
						bAlreadyTracked = true;
						break;
					}
				}
				if (!bAlreadyTracked)
				{
					SensorData.NearbyVehicles.Add(SweptVehicle);
				}
			}
		}
	}
}

void UAISensorSystem::ScanRear()
{
	if (!OwnerPawn)
	{
		return;
	}

	FVector Start = OwnerPawn->GetActorLocation();
	FVector Backward = -OwnerPawn->GetActorForwardVector();

	FAIDetectedVehicle Behind;
	if (TraceForVehicle(Start, Backward, Config.RearRange, Behind))
	{
		SensorData.VehicleBehind = Behind;
		SensorData.bHasVehicleBehind = true;
		SensorData.NearbyVehicles.Add(Behind);
	}
}

void UAISensorSystem::ScanSides()
{
	if (!OwnerPawn)
	{
		return;
	}

	FVector Start = OwnerPawn->GetActorLocation();
	FVector Right = OwnerPawn->GetActorRightVector();
	FVector Left = -Right;

	// Right side
	FAIDetectedVehicle RightVehicle;
	if (TraceForVehicle(Start, Right, Config.SideRange, RightVehicle))
	{
		SensorData.VehicleRight = RightVehicle;
		SensorData.NearbyVehicles.Add(RightVehicle);
	}

	// Left side
	FAIDetectedVehicle LeftVehicle;
	if (TraceForVehicle(Start, Left, Config.SideRange, LeftVehicle))
	{
		SensorData.VehicleLeft = LeftVehicle;
		SensorData.NearbyVehicles.Add(LeftVehicle);
	}
}

void UAISensorSystem::DetectSlipstream()
{
	if (!OwnerPawn || !SensorData.bHasVehicleAhead)
	{
		SensorData.bSlipstreamAvailable = false;
		SensorData.SlipstreamStrength = 0.0f;
		return;
	}

	const FAIDetectedVehicle& Ahead = SensorData.VehicleAhead;

	// Check if vehicle ahead is within slipstream cone
	if (Ahead.Distance <= Config.SlipstreamMaxDistance && Ahead.AngleDeg <= Config.SlipstreamConeAngle)
	{
		float Strength = CalculateSlipstreamStrength(Ahead);
		if (Strength > 0.05f)
		{
			SensorData.bSlipstreamAvailable = true;
			SensorData.SlipstreamStrength = Strength;
			SensorData.SlipstreamTarget = Ahead;
		}
	}
	else
	{
		SensorData.bSlipstreamAvailable = false;
		SensorData.SlipstreamStrength = 0.0f;
	}
}

float UAISensorSystem::CalculateSlipstreamStrength(const FAIDetectedVehicle& Vehicle) const
{
	// Strength falls off with distance and angle
	float NormalizedDistance = 1.0f - FMath::Clamp(Vehicle.Distance / Config.SlipstreamMaxDistance, 0.0f, 1.0f);
	float NormalizedAngle = 1.0f - FMath::Clamp(Vehicle.AngleDeg / Config.SlipstreamConeAngle, 0.0f, 1.0f);

	// Optimal zone is close and centered
	float DistanceFactor = FMath::Pow(NormalizedDistance, 1.5f);
	float AngleFactor = FMath::Pow(NormalizedAngle, 2.0f);

	return DistanceFactor * AngleFactor;
}

void UAISensorSystem::ScanTrackBoundaries()
{
	if (!OwnerPawn)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector Start = OwnerPawn->GetActorLocation();
	FVector Right = OwnerPawn->GetActorRightVector();
	float BoundaryRange = 500.0f; // 5 meters

	FHitResult LeftHit, RightHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoredActors);

	// Check left boundary
	SensorData.bTrackEdgeLeft = World->LineTraceSingleByChannel(
		LeftHit, Start, Start - Right * BoundaryRange, ECC_WorldStatic, QueryParams
	);

	// Check right boundary
	SensorData.bTrackEdgeRight = World->LineTraceSingleByChannel(
		RightHit, Start, Start + Right * BoundaryRange, ECC_WorldStatic, QueryParams
	);
}

float UAISensorSystem::GetSlipstreamStrengthAt(const FVector& Direction) const
{
	if (!OwnerPawn || !SensorData.bSlipstreamAvailable || !SensorData.SlipstreamTarget.Vehicle)
	{
		return 0.0f;
	}

	// Check alignment with slipstream target
	FVector ToTarget = (SensorData.SlipstreamTarget.Vehicle->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal();
	float Alignment = FVector::DotProduct(Direction.GetSafeNormal(), ToTarget);

	return SensorData.SlipstreamStrength * FMath::Max(0.0f, Alignment);
}
