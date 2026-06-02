// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "AIBehaviorTree.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "NomiRacingPlus.h"

UAIBehaviorTree::UAIBehaviorTree()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAIBehaviorTree::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Create and attach subsystem components
	SensorSystem = NewObject<UAISensorSystem>(Owner, TEXT("AISensorSystem"));
	SensorSystem->RegisterComponent();

	OvertakeEvaluator = NewObject<UAIOvertakeEvaluator>(Owner, TEXT("AIOvertakeEvaluator"));
	OvertakeEvaluator->RegisterComponent();

	DefensiveEvaluator = NewObject<UAIDefensiveEvaluator>(Owner, TEXT("AIDefensiveEvaluator"));
	DefensiveEvaluator->RegisterComponent();

	SlipstreamSystem = NewObject<UAISlipstreamSystem>(Owner, TEXT("AISlipstreamSystem"));
	SlipstreamSystem->RegisterComponent();

	RubberBandScaler = NewObject<UAIRubberBandScaler>(Owner, TEXT("AIRubberBandScaler"));
	RubberBandScaler->RegisterComponent();

	UE_LOG(LogNomiAI, Log, TEXT("AIBehaviorTree initialized with all subsystems"));
}

void UAIBehaviorTree::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentState == EAIBehaviorState::Idle || CurrentState == EAIBehaviorState::Finished)
	{
		return;
	}

	// Get sensor data
	if (!SensorSystem)
	{
		return;
	}

	const FAISensorData& SensorData = SensorSystem->GetSensorData();

	// Build decision factors from sensor data
	FAIDecisionFactors Factors;
	Factors.DistanceToVehicleAhead = SensorData.VehicleAhead.Distance;
	Factors.DistanceToVehicleBehind = SensorData.VehicleBehind.Distance;
	Factors.bSlipstreamAvailable = SensorData.bSlipstreamAvailable;
	Factors.SlipstreamStrength = SensorData.SlipstreamStrength;

	// Update slipstream system
	if (SlipstreamSystem)
	{
		SlipstreamSystem->UpdateFromSensorData(SensorData);
	}

	// Evaluate behavior
	EvaluateBehavior(Factors);
}

void UAIBehaviorTree::UpdateDecisions(const FAIDecisionFactors& Factors)
{
	EvaluateBehavior(Factors);
}

float UAIBehaviorTree::GetCombinedSpeedMultiplier() const
{
	float Multiplier = 1.0f;

	if (RubberBandScaler)
	{
		Multiplier *= RubberBandScaler->GetSpeedMultiplier();
	}

	if (SlipstreamSystem && SlipstreamSystem->IsDrafting())
	{
		Multiplier *= SlipstreamSystem->GetSpeedBoost();
	}

	return Multiplier;
}

void UAIBehaviorTree::EvaluateBehavior(const FAIDecisionFactors& Factors)
{
	// Priority-based behavior evaluation
	// 1. Stuck recovery (highest priority)
	// 2. Defensive driving
	// 3. Slipstream/drafting
	// 4. Overtaking
	// 5. Normal racing

	// Check for stuck state
	if (Factors.CurrentSpeed < 5.0f && ThrottleInput > 0.5f)
	{
		StuckTimer += 0.016f; // Approximate tick time
		if (StuckTimer > 3.0f)
		{
			TransitionToState(EAIBehaviorState::Stuck);
			return;
		}
	}
	else
	{
		StuckTimer = 0.0f;
	}

	// Evaluate rubber banding
	ApplyRubberBanding(Factors);

	// Evaluate defensive driving
	EvaluateDefend(Factors);
	if (CurrentDefensiveAction.bShouldDefend)
	{
		TransitionToState(EAIBehaviorState::Defending);
		IntegrateSubsystemResults();
		return;
	}

	// Evaluate overtaking
	EvaluateOvertake(Factors);
	if (CurrentOvertakeOpportunity.bViable)
	{
		TransitionToState(EAIBehaviorState::Overtaking);
		IntegrateSubsystemResults();
		return;
	}

	// Evaluate slipstream
	EvaluateSlipstream(Factors);
	if (Factors.bSlipstreamAvailable && Factors.SlipstreamStrength > 0.3f)
	{
		TransitionToState(EAIBehaviorState::Drafting);
		IntegrateSubsystemResults();
		return;
	}

	// Normal racing
	if (Factors.bIsInCorner)
	{
		TransitionToState(EAIBehaviorState::Cornering);
	}
	else if (Factors.CurrentSpeed > Factors.RecommendedSpeed * 1.1f)
	{
		TransitionToState(EAIBehaviorState::Braking);
	}
	else
	{
		TransitionToState(EAIBehaviorState::Accelerating);
	}

	CalculateThrottleBrake(Factors);
	CalculateSteering(Factors);
	IntegrateSubsystemResults();
}

void UAIBehaviorTree::CalculateThrottleBrake(const FAIDecisionFactors& Factors)
{
	float SpeedRatio = Factors.CurrentSpeed / FMath::Max(Factors.RecommendedSpeed, 1.0f);
	float CombinedMultiplier = GetCombinedSpeedMultiplier();

	if (Factors.bIsInCorner)
	{
		// Cornering: modulate based on sharpness
		float CornerSpeedFactor = 1.0f - Factors.CornerSharpness * 0.5f;
		if (SpeedRatio > CornerSpeedFactor)
		{
			ThrottleInput = 0.0f;
			BrakeInput = FMath::Clamp((SpeedRatio - CornerSpeedFactor) * 2.0f, 0.2f, 0.8f);
		}
		else
		{
			ThrottleInput = FMath::Clamp((CornerSpeedFactor - SpeedRatio) * 1.5f, 0.3f, 0.7f);
			BrakeInput = 0.0f;
		}
	}
	else if (SpeedRatio > 1.05f)
	{
		// Over speed: coast or brake
		ThrottleInput = 0.0f;
		BrakeInput = FMath::Clamp((SpeedRatio - 1.0f) * 1.5f, 0.0f, 0.5f);
	}
	else
	{
		// Under speed: accelerate
		ThrottleInput = FMath::Clamp((1.0f - SpeedRatio) * 2.0f, 0.4f, 1.0f) * CombinedMultiplier;
		BrakeInput = 0.0f;
	}
}

void UAIBehaviorTree::CalculateSteering(const FAIDecisionFactors& Factors)
{
	// Base steering toward waypoint would be calculated here
	// This is a placeholder - actual waypoint steering is in AICarController
	SteeringInput = 0.0f;
}

void UAIBehaviorTree::EvaluateOvertake(const FAIDecisionFactors& Factors)
{
	if (!OvertakeEvaluator || !SensorSystem)
	{
		return;
	}

	const FAISensorData& SensorData = SensorSystem->GetSensorData();
	CurrentOvertakeOpportunity = OvertakeEvaluator->Evaluate(SensorData, Factors.CurrentSpeed);

	if (CurrentOvertakeOpportunity.bViable)
	{
		OvertakeTimer = CurrentOvertakeOpportunity.EstimatedTime;
	}
}

void UAIBehaviorTree::EvaluateDefend(const FAIDecisionFactors& Factors)
{
	if (!DefensiveEvaluator || !SensorSystem)
	{
		return;
	}

	const FAISensorData& SensorData = SensorSystem->GetSensorData();
	CurrentDefensiveAction = DefensiveEvaluator->Evaluate(SensorData, Factors.CurrentSpeed, Factors.RacePosition);
}

void UAIBehaviorTree::EvaluateSlipstream(const FAIDecisionFactors& Factors)
{
	if (!SlipstreamSystem)
	{
		return;
	}

	// Slipstream is already updated from sensor data in TickComponent
	// Just check if we should adjust behavior for drafting
	if (SlipstreamSystem->IsDrafting())
	{
		// Adjust steering to maintain draft position
		float DraftSteering = SlipstreamSystem->GetDraftSteering();
		SteeringInput += DraftSteering * 0.3f;
	}
}

void UAIBehaviorTree::ApplyRubberBanding(const FAIDecisionFactors& Factors)
{
	if (!RubberBandScaler)
	{
		return;
	}

	// Rubber band scaler needs player distance and race positions
	// These would be provided by the race manager
	// For now, use the factors we have
	float DistanceToPlayer = Factors.bIsPlayerAhead ? -Factors.DistanceToVehicleAhead : Factors.DistanceToVehicleBehind;

	RubberBandScaler->UpdateState(
		DistanceToPlayer,
		Factors.RacePosition,
		1, // Player position (would come from race manager)
		Factors.TrackPosition
	);
}

void UAIBehaviorTree::IntegrateSubsystemResults()
{
	// Apply overtake steering
	if (CurrentState == EAIBehaviorState::Overtaking && CurrentOvertakeOpportunity.bViable)
	{
		SteeringInput += CurrentOvertakeOpportunity.Direction * 0.5f;
		ThrottleInput *= 1.1f; // Slight throttle boost during overtake
	}

	// Apply defensive steering
	if (CurrentState == EAIBehaviorState::Defending && CurrentDefensiveAction.bShouldDefend)
	{
		SteeringInput += CurrentDefensiveAction.DefenseDirection * 0.3f;

		if (CurrentDefensiveAction.bAdjustSpeed)
		{
			ThrottleInput *= CurrentDefensiveAction.SpeedAdjustment;
		}
	}

	// Apply rubber band adjustments
	if (RubberBandScaler && RubberBandScaler->IsActive())
	{
		const FAIRubberBandState& RBState = RubberBandScaler->GetState();

		if (CurrentState == EAIBehaviorState::Cornering)
		{
			ThrottleInput += RBState.CorneringBonus;
		}

		// Brake point adjustment affects when we start braking
		if (BrakeInput > 0.0f)
		{
			BrakeInput *= (1.0f - RBState.BrakePointOffset);
		}
	}

	// Clamp inputs
	ThrottleInput = FMath::Clamp(ThrottleInput, 0.0f, 1.0f);
	BrakeInput = FMath::Clamp(BrakeInput, 0.0f, 1.0f);
	SteeringInput = FMath::Clamp(SteeringInput, -1.0f, 1.0f);
}

void UAIBehaviorTree::TransitionToState(EAIBehaviorState NewState)
{
	if (CurrentState != NewState)
	{
		PreviousState = CurrentState;
		CurrentState = NewState;
		OnAIStateChanged.Broadcast(NewState);
	}
}
