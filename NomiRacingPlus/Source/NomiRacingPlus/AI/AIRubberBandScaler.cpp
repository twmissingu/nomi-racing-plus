// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "AI/AIRubberBandScaler.h"
#include "NomiRacingPlus.h"

UAIRubberBandScaler::UAIRubberBandScaler()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAIRubberBandScaler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Smooth the effect over time
	if (State.bActive)
	{
		SmoothedEffect = SmoothEffect(SmoothedEffect, State.EffectStrength, DeltaTime);
	}
	else
	{
		SmoothedEffect = SmoothEffect(SmoothedEffect, 0.0f, DeltaTime);
	}
}

void UAIRubberBandScaler::UpdateState(float DistanceToPlayer, int32 AIRacePosition, int32 PlayerRacePosition, float RaceProgress)
{
	State.DistanceToPlayer = DistanceToPlayer;
	State.PositionDelta = AIRacePosition - PlayerRacePosition;

	// Check if rubber banding should be active
	bool bShouldActivate = false;

	if (Config.Mode != ERubberBandMode::Disabled && RaceProgress >= Config.MinProgressToActivate)
	{
		switch (Config.Mode)
		{
		case ERubberBandMode::DistanceBased:
			bShouldActivate = FMath::Abs(DistanceToPlayer) > Config.ActivationDistance;
			break;

		case ERubberBandMode::PositionBased:
			bShouldActivate = State.PositionDelta != 0;
			break;

		case ERubberBandMode::Hybrid:
			bShouldActivate = FMath::Abs(DistanceToPlayer) > Config.ActivationDistance || State.PositionDelta != 0;
			break;

		default:
			break;
		}
	}

	State.bActive = bShouldActivate;

	if (!State.bActive)
	{
		State.EffectStrength = 0.0f;
		State.SpeedMultiplier = 1.0f;
		State.BrakePointOffset = 0.0f;
		State.CorneringBonus = 0.0f;
		return;
	}

	// Calculate effect based on mode
	float DistanceEffect = 0.0f;
	float PositionEffect = 0.0f;

	if (Config.Mode == ERubberBandMode::DistanceBased || Config.Mode == ERubberBandMode::Hybrid)
	{
		DistanceEffect = CalculateDistanceEffect(DistanceToPlayer);
	}

	if (Config.Mode == ERubberBandMode::PositionBased || Config.Mode == ERubberBandMode::Hybrid)
	{
		PositionEffect = CalculatePositionEffect(AIRacePosition, PlayerRacePosition);
	}

	// Combine effects based on mode
	switch (Config.Mode)
	{
	case ERubberBandMode::DistanceBased:
		State.EffectStrength = DistanceEffect;
		break;

	case ERubberBandMode::PositionBased:
		State.EffectStrength = PositionEffect;
		break;

	case ERubberBandMode::Hybrid:
		// Weighted blend: 60% distance, 40% position
		State.EffectStrength = DistanceEffect * 0.6f + PositionEffect * 0.4f;
		break;

	default:
		State.EffectStrength = 0.0f;
		break;
	}

	// EffectStrength: negative = AI is behind (needs boost), positive = AI is ahead (needs slowdown)
	// Clamp to valid range
	State.EffectStrength = FMath::Clamp(State.EffectStrength, -1.0f, 1.0f);

	// Convert effect to speed multiplier
	// Behind (negative effect) -> boost (multiplier > 1.0)
	// Ahead (positive effect) -> reduction (multiplier < 1.0)
	if (State.EffectStrength < 0.0f)
	{
		// Behind player: apply boost
		float BoostAmount = FMath::Abs(State.EffectStrength) * Config.CatchUpSpeedBoost;
		BoostAmount = FMath::Min(BoostAmount, Config.MaxBoost);
		State.SpeedMultiplier = 1.0f + BoostAmount;
	}
	else if (State.EffectStrength > 0.0f)
	{
		// Ahead of player: apply reduction
		float ReductionAmount = State.EffectStrength * Config.SlowDownWhenAhead;
		ReductionAmount = FMath::Min(ReductionAmount, Config.MaxReduction);
		State.SpeedMultiplier = 1.0f - ReductionAmount;
	}
	else
	{
		State.SpeedMultiplier = 1.0f;
	}

	// Brake point adjustment when behind
	if (State.EffectStrength < 0.0f)
	{
		State.BrakePointOffset = FMath::Abs(State.EffectStrength) * Config.BrakePointAdjustment;
	}
	else
	{
		State.BrakePointOffset = 0.0f;
	}

	// Cornering bonus when behind
	if (State.EffectStrength < 0.0f)
	{
		State.CorneringBonus = FMath::Abs(State.EffectStrength) * Config.CorneringBoostWhenBehind;
	}
	else
	{
		State.CorneringBonus = 0.0f;
	}

	UE_LOG(LogNomiAI, Verbose, TEXT("RubberBand: Effect=%.2f, SpeedMul=%.3f, BrakeAdj=%.2f, CornerBonus=%.2f"),
		State.EffectStrength, State.SpeedMultiplier, State.BrakePointOffset, State.CorneringBonus);
}

float UAIRubberBandScaler::CalculateDistanceEffect(float DistanceToPlayer) const
{
	// DistanceToPlayer: positive = AI is ahead, negative = AI is behind
	float AbsDistance = FMath::Abs(DistanceToPlayer);

	if (AbsDistance < Config.ActivationDistance)
	{
		return 0.0f;
	}

	// Normalize distance to 0-1 range
	float NormalizedDistance = FMath::Clamp(
		(AbsDistance - Config.ActivationDistance) / (Config.MaxEffectDistance - Config.ActivationDistance),
		0.0f,
		1.0f
	);

	// Apply easing curve for smoother transitions
	float EasedDistance = FMath::InterpEaseIn(0.0f, 1.0f, NormalizedDistance, 2.0f);

	// Sign: positive if ahead (reduce speed), negative if behind (boost speed)
	return (DistanceToPlayer > 0.0f) ? EasedDistance : -EasedDistance;
}

float UAIRubberBandScaler::CalculatePositionEffect(int32 AIPosition, int32 PlayerPosition) const
{
	int32 Delta = AIPosition - PlayerPosition;

	if (Delta == 0)
	{
		return 0.0f;
	}

	// Behind player (Delta < 0): negative effect (boost)
	// Ahead of player (Delta > 0): positive effect (reduce)
	float Effect = 0.0f;

	if (Delta > 0)
	{
		// AI is ahead: reduce speed
		Effect = Delta * Config.PositionReductionPerPlace;
		Effect = FMath::Min(Effect, Config.MaxReduction);
	}
	else
	{
		// AI is behind: boost speed
		Effect = Delta * Config.PositionBoostPerPlace;
		Effect = FMath::Max(Effect, -Config.MaxBoost);
	}

	return Effect;
}

float UAIRubberBandScaler::SmoothEffect(float CurrentEffect, float TargetEffect, float DeltaTime) const
{
	if (Config.SmoothingFactor <= 0.0f)
	{
		return TargetEffect;
	}

	float InterpSpeed = Config.SmoothingFactor;
	return FMath::FInterpTo(CurrentEffect, TargetEffect, DeltaTime, InterpSpeed);
}
