// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "AIOvertakeEvaluator.h"
#include "AIBehaviorTree.h"
#include "Kismet/KismetMathLibrary.h"
#include "NomiRacingPlus.h"

UAIOvertakeEvaluator::UAIOvertakeEvaluator()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAIOvertakeEvaluator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerPawn)
	{
		OwnerPawn = Cast<APawn>(GetOwner());
	}

	if (OvertakeCooldownTimer > 0.0f)
	{
		OvertakeCooldownTimer -= DeltaTime;
	}

	if (bIsOvertaking)
	{
		OvertakeTimer -= DeltaTime;
		OvertakeProgress = FMath::Clamp(1.0f - (OvertakeTimer / 3.0f), 0.0f, 1.0f);

		if (OvertakeTimer <= 0.0f)
		{
			bIsOvertaking = false;
			OvertakeProgress = 0.0f;
			OvertakeCooldownTimer = OvertakeCooldown;
		}
	}
}

FOvertakeOpportunity UAIOvertakeEvaluator::Evaluate(const FAISensorData& SensorData, float CurrentSpeedKmh)
{
	FOvertakeOpportunity Result;

	if (!OwnerPawn || OvertakeCooldownTimer > 0.0f)
	{
		return Result;
	}

	if (!SensorData.bHasVehicleAhead)
	{
		return Result;
	}

	const FAIDetectedVehicle& Target = SensorData.VehicleAhead;

	// Skip if target is too far or too close
	if (Target.Distance > 3000.0f || Target.Distance < 200.0f)
	{
		return Result;
	}

	// Speed advantage: how much faster we are than the target (positive = we're faster)
	float SpeedAdvantage = Target.RelativeSpeed;

	// Analyze gaps on both sides
	float GapInside = AnalyzeGapWidth(Target, true);
	float GapOutside = AnalyzeGapWidth(Target, false);

	// Select strategy
	EOvertakeStrategy Strategy = SelectStrategy(Target, CurrentSpeedKmh, GapInside, GapOutside);

	if (Strategy == EOvertakeStrategy::None)
	{
		return Result;
	}

	// Determine direction based on strategy
	float Direction = 0.0f;
	float GapWidth = 0.0f;

	switch (Strategy)
	{
	case EOvertakeStrategy::InsidePass:
	case EOvertakeStrategy::LateBrakePass:
		Direction = (Target.LateralOffset > 0) ? -1.0f : 1.0f;
		GapWidth = GapInside;
		break;

	case EOvertakeStrategy::OutsidePass:
	case EOvertakeStrategy::SlingshotPass:
		Direction = (Target.LateralOffset > 0) ? 1.0f : -1.0f;
		GapWidth = GapOutside;
		break;

	case EOvertakeStrategy::DraftAndPass:
		// Use the wider gap
		if (GapInside > GapOutside)
		{
			Direction = (Target.LateralOffset > 0) ? -1.0f : 1.0f;
			GapWidth = GapInside;
		}
		else
		{
			Direction = (Target.LateralOffset > 0) ? 1.0f : -1.0f;
			GapWidth = GapOutside;
		}
		break;

	default:
		break;
	}

	// Check track boundaries
	if (Direction < 0.0f && SensorData.bTrackEdgeLeft)
	{
		return Result;
	}
	if (Direction > 0.0f && SensorData.bTrackEdgeRight)
	{
		return Result;
	}

	// Calculate risk and confidence
	float Risk = CalculateRisk(Target, Strategy, GapWidth);
	float Confidence = CalculateConfidence(SpeedAdvantage, GapWidth, Risk, Strategy);

	// Apply personality modifiers
	Confidence *= Personality.FrequencyMultiplier;

	// Check against risk tolerance
	if (Risk > Personality.RiskTolerance)
	{
		Confidence *= 0.3f;
	}

	// Minimum confidence threshold
	if (Confidence < 0.3f)
	{
		return Result;
	}

	Result.bViable = true;
	Result.Strategy = Strategy;
	Result.TargetVehicle = Target.Vehicle;
	Result.Direction = Direction;
	Result.Confidence = Confidence;
	Result.Risk = Risk;
	Result.SpeedAdvantageNeeded = FMath::Max(0.0f, MinSpeedAdvantage - SpeedAdvantage);

	// Estimate time based on strategy
	switch (Strategy)
	{
	case EOvertakeStrategy::InsidePass:
		Result.EstimatedTime = 1.5f;
		break;
	case EOvertakeStrategy::OutsidePass:
		Result.EstimatedTime = 2.0f;
		break;
	case EOvertakeStrategy::SlingshotPass:
		Result.EstimatedTime = 2.5f;
		break;
	case EOvertakeStrategy::LateBrakePass:
		Result.EstimatedTime = 1.0f;
		break;
	case EOvertakeStrategy::DraftAndPass:
		Result.EstimatedTime = 3.0f;
		break;
	default:
		Result.EstimatedTime = 2.0f;
		break;
	}

	// Start overtake execution
	bIsOvertaking = true;
	OvertakeTimer = Result.EstimatedTime;
	OvertakeProgress = 0.0f;
	ActiveStrategy = Strategy;

	UE_LOG(LogNomiAI, Verbose, TEXT("Overtake opportunity: Strategy=%d, Direction=%.1f, Confidence=%.2f, Risk=%.2f"),
		(int32)Strategy, Direction, Confidence, Risk);

	return Result;
}

float UAIOvertakeEvaluator::AnalyzeGapWidth(const FAIDetectedVehicle& Target, bool bInside) const
{
	if (!OwnerPawn || !Target.Vehicle)
	{
		return 0.0f;
	}

	FVector TargetLocation = Target.Vehicle->GetActorLocation();
	FVector OwnerLocation = OwnerPawn->GetActorLocation();
	FVector Forward = OwnerPawn->GetActorForwardVector();
	FVector Right = OwnerPawn->GetActorRightVector();

	// Approximate vehicle width (typical car ~200cm)
	constexpr float VehicleHalfWidth = 100.0f;
	constexpr float SafetyMargin = 80.0f;

	// Calculate lateral distance to target
	float LateralDist = FMath::Abs(FVector::DotProduct(TargetLocation - OwnerLocation, Right));

	if (bInside)
	{
		// Inside gap = lateral distance minus vehicle widths
		return FMath::Max(0.0f, LateralDist - VehicleHalfWidth * 2.0f - SafetyMargin);
	}
	else
	{
		// Outside gap is generally wider (assume track is ~800cm wide)
		constexpr float TrackHalfWidth = 400.0f;
		float TargetLateralPos = FVector::DotProduct(TargetLocation - OwnerLocation, Right);
		float OutsideSpace = TrackHalfWidth - FMath::Abs(TargetLateralPos) - VehicleHalfWidth;
		return FMath::Max(0.0f, OutsideSpace - SafetyMargin);
	}
}

EOvertakeStrategy UAIOvertakeEvaluator::SelectStrategy(const FAIDetectedVehicle& Target, float CurrentSpeed, float GapWidthInside, float GapWidthOutside) const
{
	bool bInsideViable = GapWidthInside >= MinGapWidth;
	bool bOutsideViable = GapWidthOutside >= MinGapWidth;

	if (!bInsideViable && !bOutsideViable)
	{
		return EOvertakeStrategy::None;
	}

	// Check for draft and pass opportunity
	if (Target.Distance < 2000.0f && Target.RelativeSpeed < 5.0f && Personality.DraftUsage > 0.5f)
	{
		return EOvertakeStrategy::DraftAndPass;
	}

	// Check for late braking opportunity (target is braking or cornering)
	if (CanLateBrake(Target, CurrentSpeed) && bInsideViable)
	{
		return EOvertakeStrategy::LateBrakePass;
	}

	// Check for slingshot opportunity (on straights with speed advantage)
	if (!Target.bIsAhead && bOutsideViable && CurrentSpeed > 100.0f)
	{
		return EOvertakeStrategy::SlingshotPass;
	}

	// Apply side preference
	float InsideScore = GapWidthInside * (1.0f - Personality.SidePreference * 0.5f);
	float OutsideScore = GapWidthOutside * (1.0f + Personality.SidePreference * 0.5f);

	if (bInsideViable && InsideScore >= OutsideScore)
	{
		return EOvertakeStrategy::InsidePass;
	}

	if (bOutsideViable)
	{
		return EOvertakeStrategy::OutsidePass;
	}

	return bInsideViable ? EOvertakeStrategy::InsidePass : EOvertakeStrategy::None;
}

float UAIOvertakeEvaluator::CalculateRisk(const FAIDetectedVehicle& Target, EOvertakeStrategy Strategy, float GapWidth) const
{
	float BaseRisk = 0.0f;

	// Risk increases with proximity
	float ProximityRisk = FMath::Clamp(1.0f - (Target.Distance / 2000.0f), 0.0f, 0.4f);
	BaseRisk += ProximityRisk;

	// Risk decreases with wider gaps
	float GapRisk = FMath::Clamp(1.0f - (GapWidth / 500.0f), 0.0f, 0.3f);
	BaseRisk += GapRisk;

	// Strategy-specific risk
	switch (Strategy)
	{
	case EOvertakeStrategy::InsidePass:
		BaseRisk += 0.1f; // Moderate risk
		break;
	case EOvertakeStrategy::OutsidePass:
		BaseRisk += 0.05f; // Lower risk
		break;
	case EOvertakeStrategy::SlingshotPass:
		BaseRisk += 0.15f; // Higher risk
		break;
	case EOvertakeStrategy::LateBrakePass:
		BaseRisk += 0.25f; // Highest risk
		break;
	case EOvertakeStrategy::DraftAndPass:
		BaseRisk += 0.1f; // Moderate risk
		break;
	default:
		break;
	}

	return FMath::Clamp(BaseRisk, 0.0f, 1.0f);
}

bool UAIOvertakeEvaluator::CanLateBrake(const FAIDetectedVehicle& Target, float CurrentSpeed) const
{
	// Late braking viable if we're going faster and the target is close to a braking point
	if (Target.RelativeSpeed < -5.0f) // We're slower, not a good opportunity
	{
		return false;
	}

	// Target must be relatively close
	if (Target.Distance > 1500.0f)
	{
		return false;
	}

	// Need sufficient speed
	return CurrentSpeed > 80.0f;
}

float UAIOvertakeEvaluator::CalculateConfidence(float SpeedAdvantage, float GapWidth, float Risk, EOvertakeStrategy Strategy) const
{
	float Confidence = 0.0f;

	// Speed advantage contribution
	float SpeedConfidence = FMath::Clamp(SpeedAdvantage / MinSpeedAdvantage, 0.0f, 1.0f);
	Confidence += SpeedConfidence * 0.3f;

	// Gap width contribution
	float GapConfidence = FMath::Clamp(GapWidth / (MinGapWidth * 2.0f), 0.0f, 1.0f);
	Confidence += GapConfidence * 0.3f;

	// Risk penalty
	Confidence -= Risk * 0.3f;

	// Strategy bonus
	switch (Strategy)
	{
	case EOvertakeStrategy::DraftAndPass:
		Confidence += 0.1f * Personality.DraftUsage;
		break;
	case EOvertakeStrategy::LateBrakePass:
		Confidence += 0.05f * Personality.RiskTolerance;
		break;
	default:
		break;
	}

	return FMath::Clamp(Confidence, 0.0f, 1.0f);
}

void UAIOvertakeEvaluator::CancelOvertake()
{
	bIsOvertaking = false;
	OvertakeProgress = 0.0f;
	OvertakeCooldownTimer = OvertakeCooldown;
	ActiveStrategy = EOvertakeStrategy::None;
}
