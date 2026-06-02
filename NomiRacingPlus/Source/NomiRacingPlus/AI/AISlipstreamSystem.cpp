// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "AI/AISlipstreamSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "NomiRacingPlus.h"

UAISlipstreamSystem::UAISlipstreamSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAISlipstreamSystem::BeginPlay()
{
	Super::BeginPlay();

	OwnerPawn = Cast<APawn>(GetOwner());
}

void UAISlipstreamSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Config.bEnabled || !OwnerPawn)
	{
		return;
	}

	// Handle fade-out when leaving slipstream
	if (!CurrentEffect.bInSlipstream && bWasInSlipstream)
	{
		FadeTimer += DeltaTime;
		if (FadeTimer >= Config.WakeFadeTime)
		{
			CurrentEffect = FSlipstreamEffect();
			bWasInSlipstream = false;
			FadeTimer = 0.0f;
		}
		else
		{
			// Gradually reduce effect during fade
			float FadeFactor = 1.0f - (FadeTimer / Config.WakeFadeTime);
			CurrentEffect.Strength *= FadeFactor;
			CurrentEffect.SpeedBoostFactor = 1.0f + (CurrentEffect.SpeedBoostFactor - 1.0f) * FadeFactor;
			CurrentEffect.DragReduction *= FadeFactor;
		}
	}
}

void UAISlipstreamSystem::UpdateFromSensorData(const FAISensorData& SensorData)
{
	if (!Config.bEnabled || !OwnerPawn)
	{
		return;
	}

	// Check if there's a slipstream target
	if (!SensorData.bSlipstreamAvailable || !SensorData.SlipstreamTarget.Vehicle)
	{
		CurrentEffect.bInSlipstream = false;
		bWasInSlipstream = CurrentEffect.bInSlipstream;
		return;
	}

	// Build wake zone from the slipstream target
	FSlipstreamZone Zone = BuildWakeZone(SensorData.SlipstreamTarget);
	ActiveZone = Zone;

	// Calculate position in wake
	FVector OwnerPos = OwnerPawn->GetActorLocation();
	float WakeStrength = CalculateWakeStrength(OwnerPos, Zone);

	if (WakeStrength > 0.05f)
	{
		CurrentEffect.bInSlipstream = true;
		CurrentEffect.Strength = WakeStrength;
		CurrentEffect.SourceVehicle = SensorData.SlipstreamTarget.Vehicle;

		// Calculate speed boost: linear interpolation between 1.0 and max boost
		CurrentEffect.SpeedBoostFactor = 1.0f + (Config.MaxSpeedBoost - 1.0f) * WakeStrength;

		// Calculate drag reduction
		CurrentEffect.DragReduction = Config.MaxDragReduction * WakeStrength;

		// Calculate steering correction to stay in wake
		CurrentEffect.SteeringCorrection = CalculateSteeringCorrection(OwnerPos, Zone) * Config.LateralStability;

		bWasInSlipstream = true;
		FadeTimer = 0.0f;
	}
	else
	{
		CurrentEffect.bInSlipstream = false;
	}
}

FSlipstreamZone UAISlipstreamSystem::BuildWakeZone(const FAIDetectedVehicle& Vehicle) const
{
	FSlipstreamZone Zone;

	if (!Vehicle.Vehicle || !OwnerPawn)
	{
		return Zone;
	}

	Zone.SourceVehicle = Vehicle.Vehicle;
	Zone.SourceSpeed = Vehicle.Vehicle->GetVelocity().Size() / 100.0f * 3.6f; // cm/s to km/h

	FVector SourceForward = Vehicle.Vehicle->GetActorForwardVector();
	FVector SourceLocation = Vehicle.Vehicle->GetActorLocation();

	// Wake extends behind the vehicle
	Zone.WakeLength = FMath::Clamp(Zone.SourceSpeed * Config.SpeedWakeScale * 100.0f, 1000.0f, Config.MaxDistance);
	Zone.WakeWidth = 300.0f + Zone.SourceSpeed * 0.5f; // Wider at higher speeds

	// Center of wake is behind the source vehicle
	Zone.WakeCenter = SourceLocation - SourceForward * (Zone.WakeLength * 0.5f);

	// Strength scales with speed (stronger at higher speeds)
	Zone.MaxStrength = FMath::Clamp(Zone.SourceSpeed / 150.0f, 0.3f, 1.0f);

	return Zone;
}

float UAISlipstreamSystem::CalculateWakeStrength(const FVector& VehiclePos, const FSlipstreamZone& Zone) const
{
	if (!Zone.SourceVehicle)
	{
		return 0.0f;
	}

	FVector SourceLocation = Zone.SourceVehicle->GetActorLocation();
	FVector SourceForward = Zone.SourceVehicle->GetActorForwardVector();

	// Vector from source to our position
	FVector ToUs = VehiclePos - SourceLocation;
	float ForwardDist = FVector::DotProduct(ToUs, -SourceForward); // Positive = behind source
	float LateralDist = FVector::CrossProduct(ToUs, -SourceForward).Size();

	// Must be behind the source vehicle
	if (ForwardDist < 0.0f)
	{
		return 0.0f;
	}

	// Must be within wake length
	if (ForwardDist > Zone.WakeLength)
	{
		return 0.0f;
	}

	// Check cone angle
	float AngleRad = FMath::Atan2(LateralDist, ForwardDist);
	float AngleDeg = FMath::RadiansToDegrees(AngleRad);
	if (AngleDeg > Config.WakeConeAngle)
	{
		return 0.0f;
	}

	// Strength falls off with distance from source
	float DistanceFactor = 1.0f - FMath::Pow(ForwardDist / Zone.WakeLength, 1.5f);

	// Strength falls off with lateral distance from center line
	float MaxLateralAtDist = FMath::Tan(FMath::DegreesToRadians(Config.WakeConeAngle)) * ForwardDist;
	float LateralFactor = 1.0f - FMath::Pow(LateralDist / FMath::Max(MaxLateralAtDist, 1.0f), 2.0f);

	// Combine factors
	float Strength = Zone.MaxStrength * DistanceFactor * FMath::Max(0.0f, LateralFactor);

	return FMath::Clamp(Strength, 0.0f, 1.0f);
}

float UAISlipstreamSystem::CalculateSteeringCorrection(const FVector& VehiclePos, const FSlipstreamZone& Zone) const
{
	if (!Zone.SourceVehicle || !OwnerPawn)
	{
		return 0.0f;
	}

	FVector SourceLocation = Zone.SourceVehicle->GetActorLocation();
	FVector SourceForward = Zone.SourceVehicle->GetActorForwardVector();
	FVector SourceRight = Zone.SourceVehicle->GetActorRightVector();

	// Ideal draft position: directly behind the source vehicle
	FVector IdealPos = SourceLocation - SourceForward * (Zone.WakeLength * 0.3f);

	// Lateral offset from ideal position
	FVector Offset = IdealPos - VehiclePos;
	float LateralOffset = FVector::DotProduct(Offset, SourceRight);

	// Convert to steering correction (-1 to 1)
	float Correction = FMath::Clamp(LateralOffset / (Zone.WakeWidth * 0.5f), -1.0f, 1.0f);

	return Correction;
}
