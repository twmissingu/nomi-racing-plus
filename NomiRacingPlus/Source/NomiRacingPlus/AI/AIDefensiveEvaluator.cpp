// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "AI/AIDefensiveEvaluator.h"
#include "Kismet/KismetMathLibrary.h"
#include "NomiRacingPlus.h"

UAIDefensiveEvaluator::UAIDefensiveEvaluator()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAIDefensiveEvaluator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerPawn)
	{
		OwnerPawn = Cast<APawn>(GetOwner());
	}

	if (DefenseTimer > 0.0f)
	{
		DefenseTimer -= DeltaTime;
		if (DefenseTimer <= 0.0f)
		{
			bIsDefending = false;
			ActiveStrategy = EDefensiveStrategy::None;
		}
	}
}

FAIDefensiveAction UAIDefensiveEvaluator::Evaluate(const FAISensorData& SensorData, float CurrentSpeedKmh, int32 RacePosition)
{
	FAIDefensiveAction Result;

	if (!OwnerPawn)
	{
		return Result;
	}

	// Only defend if we have a vehicle behind us
	if (!SensorData.bHasVehicleBehind)
	{
		if (bIsDefending && DefenseTimer <= 0.0f)
		{
			StopDefending();
		}
		return Result;
	}

	const FAIDetectedVehicle& Attacker = SensorData.VehicleBehind;

	// Only defend if attacker is within activation range
	if (Attacker.Distance > DefenseActivationDistance)
	{
		return Result;
	}

	// Calculate threat level
	float ThreatLevel = CalculateThreatLevel(Attacker, CurrentSpeedKmh);

	// Apply awareness modifier - higher awareness detects threats earlier
	float AwarenessAdjustedDistance = DefenseActivationDistance * (0.5f + Personality.Awareness * 0.5f);
	if (Attacker.Distance > AwarenessAdjustedDistance && ThreatLevel < MinThreatLevel * 1.5f)
	{
		return Result;
	}

	if (ThreatLevel < MinThreatLevel)
	{
		return Result;
	}

	// Select defensive strategy
	EDefensiveStrategy Strategy = SelectStrategy(Attacker, ThreatLevel);

	if (Strategy == EDefensiveStrategy::None)
	{
		return Result;
	}

	// Determine defense direction
	float DefenseDirection = 0.0f;
	bool bAdjustSpeed = false;
	float SpeedAdjustment = 1.0f;

	switch (Strategy)
	{
	case EDefensiveStrategy::BlockInside:
		// Move to block the inside line
		DefenseDirection = GetAttackSide(Attacker);
		break;

	case EDefensiveStrategy::BlockOutside:
		// Move to block the outside line
		DefenseDirection = -GetAttackSide(Attacker);
		break;

	case EDefensiveStrategy::MirrorMovement:
		{
			// Track attacker's lateral movement and mirror it
			float CurrentLateral = Attacker.LateralOffset;
			float LateralDelta = CurrentLateral - PreviousAttackerLateral;
			DefenseDirection = FMath::Sign(LateralDelta);
			PreviousAttackerLateral = CurrentLateral;
		}
		break;

	case EDefensiveStrategy::HoldLine:
		// Stay on current line, don't move
		DefenseDirection = 0.0f;
		break;

	case EDefensiveStrategy::SqueezeOff:
		// Move toward the attacker to squeeze them
		DefenseDirection = GetAttackSide(Attacker) * Personality.SqueezeTendency;
		break;

	case EDefensiveStrategy::DefensiveBrake:
		// Brake earlier to force attacker to brake too
		DefenseDirection = GetAttackSide(Attacker) * 0.3f;
		bAdjustSpeed = true;
		SpeedAdjustment = 0.9f;
		break;

	default:
		break;
	}

	// Check track boundaries
	if (DefenseDirection < -0.5f && SensorData.bTrackEdgeLeft)
	{
		DefenseDirection = 0.0f;
	}
	if (DefenseDirection > 0.5f && SensorData.bTrackEdgeRight)
	{
		DefenseDirection = 0.0f;
	}

	Result.bShouldDefend = true;
	Result.Strategy = Strategy;
	Result.ThreatVehicle = Attacker.Vehicle;
	Result.DefenseDirection = DefenseDirection;
	Result.Urgency = ThreatLevel * Personality.Aggressiveness;
	Result.bAdjustSpeed = bAdjustSpeed;
	Result.SpeedAdjustment = SpeedAdjustment;

	bIsDefending = true;
	ActiveStrategy = Strategy;
	DefenseTimer = 2.0f;
	PreviousAttacker = Attacker.Vehicle;

	UE_LOG(LogNomiAI, Verbose, TEXT("Defense: Strategy=%d, Threat=%.2f, Direction=%.1f"),
		(int32)Strategy, ThreatLevel, DefenseDirection);

	return Result;
}

float UAIDefensiveEvaluator::CalculateThreatLevel(const FAIDetectedVehicle& Attacker, float CurrentSpeed) const
{
	float Threat = 0.0f;

	// Proximity threat - closer = higher threat
	float ProximityThreat = FMath::Clamp(1.0f - (Attacker.Distance / DefenseActivationDistance), 0.0f, 0.4f);
	Threat += ProximityThreat;

	// Speed threat - attacker closing in = higher threat
	if (Attacker.RelativeSpeed < 0.0f) // Negative means attacker is faster
	{
		float SpeedThreat = FMath::Clamp(FMath::Abs(Attacker.RelativeSpeed) / 30.0f, 0.0f, 0.3f);
		Threat += SpeedThreat;
	}

	// Position threat - centered behind = higher threat (likely preparing to pass)
	float Centeredness = 1.0f - FMath::Abs(Attacker.LateralOffset);
	Threat += Centeredness * 0.2f;

	// Aggressiveness factor
	Threat *= (0.5f + Personality.Aggressiveness * 0.5f);

	return FMath::Clamp(Threat, 0.0f, 1.0f);
}

EDefensiveStrategy UAIDefensiveEvaluator::SelectStrategy(const FAIDetectedVehicle& Attacker, float ThreatLevel) const
{
	float AttackSide = GetAttackSide(Attacker);

	// If attacker is very close and closing fast, consider defensive braking
	if (Attacker.Distance < 500.0f && Attacker.RelativeSpeed < -10.0f && Personality.BrakeDefensiveness > 0.5f)
	{
		return EDefensiveStrategy::DefensiveBrake;
	}

	// If attacker is off to one side, block that side
	if (FMath::Abs(Attacker.LateralOffset) > 0.3f)
	{
		bool bCanBlockAttackerSide = CanBlockSide(Attacker.LateralOffset, false, false);

		if (bCanBlockAttackerSide)
		{
			// Apply block preference
			if (Personality.BlockPreference > 0.3f)
			{
				return EDefensiveStrategy::BlockOutside;
			}
			else if (Personality.BlockPreference < -0.3f)
			{
				return EDefensiveStrategy::BlockInside;
			}
			return EDefensiveStrategy::BlockInside;
		}
	}

	// If attacker is directly behind and centered, mirror their movements
	if (FMath::Abs(Attacker.LateralOffset) < 0.3f)
	{
		if (PreviousAttacker == Attacker.Vehicle)
		{
			return EDefensiveStrategy::MirrorMovement;
		}
		return EDefensiveStrategy::HoldLine;
	}

	// If we're aggressive enough, try to squeeze
	if (ThreatLevel > 0.6f && Personality.SqueezeTendency > 0.5f)
	{
		return EDefensiveStrategy::SqueezeOff;
	}

	return EDefensiveStrategy::HoldLine;
}

float UAIDefensiveEvaluator::GetAttackSide(const FAIDetectedVehicle& Attacker) const
{
	// Returns -1 for left, 1 for right
	return FMath::Sign(Attacker.LateralOffset);
}

bool UAIDefensiveEvaluator::CanBlockSide(float Direction, bool bTrackEdgeLeft, bool bTrackEdgeRight) const
{
	// Can't block toward track edges
	if (Direction < -0.5f && bTrackEdgeLeft)
	{
		return false;
	}
	if (Direction > 0.5f && bTrackEdgeRight)
	{
		return false;
	}
	return true;
}

void UAIDefensiveEvaluator::StopDefending()
{
	bIsDefending = false;
	ActiveStrategy = EDefensiveStrategy::None;
	DefenseTimer = 0.0f;
	PreviousAttacker = nullptr;
}
