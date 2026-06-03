// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Tests/AITest.h"
#include "AI/AICarController.h"
#include "AI/AIBehaviorTree.h"
#include "AI/AISensorSystem.h"
#include "AI/AIOvertakeEvaluator.h"
#include "AI/AIDefensiveEvaluator.h"
#include "AI/AISlipstreamSystem.h"
#include "AI/AIRubberBandScaler.h"

/**
 * Test AI difficulty settings
 */
bool FAIDifficultySettingsTest::RunTest(const FString& Parameters)
{
	// Test 1: Create AI controller
	AAICarController* Controller = NewObject<AAICarController>();
	TestNotNull(TEXT("AI Controller should be created"), Controller);

	if (!Controller)
	{
		return false;
	}

	// Test 2: Verify initial difficulty
	TestEqual(TEXT("Initial difficulty should be Normal"), Controller->GetDifficulty(), EAIDifficulty::Normal);

	// Test 3: Set difficulty to Easy
	Controller->SetDifficulty(EAIDifficulty::Easy);
	TestEqual(TEXT("Difficulty should be Easy"), Controller->GetDifficulty(), EAIDifficulty::Easy);

	// Test 4: Set difficulty to Hard
	Controller->SetDifficulty(EAIDifficulty::Hard);
	TestEqual(TEXT("Difficulty should be Hard"), Controller->GetDifficulty(), EAIDifficulty::Hard);

	// Test 5: Verify difficulty settings structure
	FAIDifficultySettings Settings;
	Settings.SpeedMultiplier = 0.95f;
	Settings.ThrottleSmoothness = 0.8f;
	Settings.BrakeSmoothness = 0.7f;
	Settings.SteeringSmoothness = 0.6f;
	Settings.BrakingDistanceMultiplier = 1.1f;
	Settings.OvertakeAggressiveness = 0.5f;
	Settings.CorneringSpeedFactor = 0.8f;
	Settings.PathDeviation = 0.1f;
	Settings.ReactionTime = 0.3f;
	Settings.RubberBandStrength = 0.3f;

	TestTrue(TEXT("Speed multiplier should be valid"), Settings.SpeedMultiplier > 0.0f);
	TestTrue(TEXT("Reaction time should be positive"), Settings.ReactionTime > 0.0f);
	TestTrue(TEXT("Rubber band strength should be clamped"), Settings.RubberBandStrength >= 0.0f && Settings.RubberBandStrength <= 1.0f);

	return true;
}

/**
 * Test AI state transitions
 */
bool FAIStateTransitionTest::RunTest(const FString& Parameters)
{
	// Test 1: Verify AI states exist
	EAIState Idle = EAIState::Idle;
	EAIState Racing = EAIState::Racing;
	EAIState Overtaking = EAIState::Overtaking;
	EAIState Defending = EAIState::Defending;
	EAIState Recovering = EAIState::Recovering;
	EAIState Finished = EAIState::Finished;

	// Test 2: Verify states are distinct
	TestNotEqual(TEXT("Idle should differ from Racing"), (int32)Idle, (int32)Racing);
	TestNotEqual(TEXT("Racing should differ from Overtaking"), (int32)Racing, (int32)Overtaking);
	TestNotEqual(TEXT("Defending should differ from Recovering"), (int32)Defending, (int32)Recovering);

	// Test 3: Create controller and verify initial state
	AAICarController* Controller = NewObject<AAICarController>();
	TestNotNull(TEXT("AI Controller should be created"), Controller);

	if (Controller)
	{
		TestEqual(TEXT("Initial state should be Idle"), Controller->GetAIState(), EAIState::Idle);
	}

	return true;
}

/**
 * Test AI waypoint system
 */
bool FAIWaypointTest::RunTest(const FString& Parameters)
{
	// Test 1: Create waypoint structure
	FAIWaypoint Waypoint;
	Waypoint.Location = FVector(1000.0f, 2000.0f, 0.0f);
	Waypoint.RecommendedSpeed = 150.0f;
	Waypoint.bIsCorner = true;
	Waypoint.CornerSharpness = 0.7f;

	// Test 2: Verify waypoint values
	TestEqual(TEXT("Waypoint location X"), Waypoint.Location.X, 1000.0f);
	TestEqual(TEXT("Waypoint location Y"), Waypoint.Location.Y, 2000.0f);
	TestEqual(TEXT("Waypoint speed"), Waypoint.RecommendedSpeed, 150.0f);
	TestTrue(TEXT("Waypoint should be corner"), Waypoint.bIsCorner);
	TestEqual(TEXT("Corner sharpness"), Waypoint.CornerSharpness, 0.7f);

	// Test 3: Create waypoint array
	TArray<FAIWaypoint> Waypoints;
	Waypoints.Add(Waypoint);

	FAIWaypoint Waypoint2;
	Waypoint2.Location = FVector(3000.0f, 4000.0f, 0.0f);
	Waypoint2.RecommendedSpeed = 200.0f;
	Waypoint2.bIsCorner = false;
	Waypoints.Add(Waypoint2);

	// Test 4: Verify waypoint array
	TestEqual(TEXT("Waypoint array should have 2 elements"), Waypoints.Num(), 2);

	// Test 5: Set waypoints on controller
	AAICarController* Controller = NewObject<AAICarController>();
	TestNotNull(TEXT("AI Controller should be created"), Controller);

	if (Controller)
	{
		Controller->SetWaypoints(Waypoints);
		// Function should not crash
	}

	return true;
}

/**
 * Test AI Behavior Tree system
 */
bool FAIBehaviorTreeTest::RunTest(const FString& Parameters)
{
	// Test 1: Create behavior tree component
	UAIBehaviorTree* BehaviorTree = NewObject<UAIBehaviorTree>();
	TestNotNull(TEXT("Behavior Tree should be created"), BehaviorTree);

	if (!BehaviorTree)
	{
		return false;
	}

	// Test 2: Verify initial state
	TestEqual(TEXT("Initial state should be Idle"), BehaviorTree->GetBehaviorState(), EAIBehaviorState::Idle);

	// Test 3: Verify initial inputs are zero
	TestEqual(TEXT("Initial throttle should be 0"), BehaviorTree->GetThrottleInput(), 0.0f);
	TestEqual(TEXT("Initial brake should be 0"), BehaviorTree->GetBrakeInput(), 0.0f);
	TestEqual(TEXT("Initial steering should be 0"), BehaviorTree->GetSteeringInput(), 0.0f);

	// Test 4: Test difficulty setting
	BehaviorTree->SetDifficulty(0.7f);
	// Should not crash

	// Test 5: Test rubber band config
	FAIRubberBandConfig RBConfig;
	RBConfig.Mode = ERubberBandMode::Hybrid;
	RBConfig.CatchUpSpeedBoost = 0.02f;
	RBConfig.MaxBoost = 0.2f;
	BehaviorTree->SetRubberBandConfig(RBConfig);
	// Should not crash

	// Test 6: Test slipstream config
	FAISlipstreamConfig SlipConfig;
	SlipConfig.bEnabled = true;
	SlipConfig.DetectionDistance = 2000.0f;
	SlipConfig.MaxSpeedBoost = 0.15f;
	BehaviorTree->SetSlipstreamConfig(SlipConfig);
	// Should not crash

	// Test 7: Test decision factors
	FAIDecisionFactors Factors;
	Factors.CurrentSpeed = 100.0f;
	Factors.RecommendedSpeed = 120.0f;
	Factors.bIsOnStraight = true;
	Factors.bIsInCorner = false;
	Factors.DistanceToVehicleAhead = 1500.0f;
	Factors.bSlipstreamAvailable = false;

	BehaviorTree->UpdateDecisions(Factors);
	// Should not crash

	return true;
}

/**
 * Test AI Overtake Evaluator
 */
bool FAIOvertakeEvaluatorTest::RunTest(const FString& Parameters)
{
	// Test 1: Create overtake evaluator
	UAIOvertakeEvaluator* Evaluator = NewObject<UAIOvertakeEvaluator>();
	TestNotNull(TEXT("Overtake Evaluator should be created"), Evaluator);

	if (!Evaluator)
	{
		return false;
	}

	// Test 2: Set personality
	FOvertakePersonality Personality;
	Personality.FrequencyMultiplier = 1.0f;
	Personality.RiskTolerance = 0.5f;
	Personality.Patience = 0.5f;
	Personality.SidePreference = 0.0f;
	Personality.DraftUsage = 0.5f;
	Evaluator->SetPersonality(Personality);

	const FOvertakePersonality& RetrievedPersonality = Evaluator->GetPersonality();
	TestEqual(TEXT("Frequency multiplier should match"), RetrievedPersonality.FrequencyMultiplier, 1.0f);
	TestEqual(TEXT("Risk tolerance should match"), RetrievedPersonality.RiskTolerance, 0.5f);

	// Test 3: Evaluate with empty sensor data (should return no opportunity)
	FAISensorData EmptySensorData;
	FOvertakeOpportunity Result = Evaluator->Evaluate(EmptySensorData, 100.0f);
	TestFalse(TEXT("Should not find opportunity with no vehicles"), Result.bViable);

	// Test 4: Verify not overtaking initially
	TestFalse(TEXT("Should not be overtaking initially"), Evaluator->IsOvertaking());
	TestEqual(TEXT("Overtake progress should be 0"), Evaluator->GetOvertakeProgress(), 0.0f);

	// Test 5: Cancel overtake
	Evaluator->CancelOvertake();
	TestFalse(TEXT("Should not be overtaking after cancel"), Evaluator->IsOvertaking());

	return true;
}

/**
 * Test AI Defensive Evaluator
 */
bool FAIDefensiveEvaluatorTest::RunTest(const FString& Parameters)
{
	// Test 1: Create defensive evaluator
	UAIDefensiveEvaluator* Evaluator = NewObject<UAIDefensiveEvaluator>();
	TestNotNull(TEXT("Defensive Evaluator should be created"), Evaluator);

	if (!Evaluator)
	{
		return false;
	}

	// Test 2: Set personality
	FAIDefensivePersonality Personality;
	Personality.Aggressiveness = 0.5f;
	Personality.Awareness = 0.7f;
	Personality.BlockPreference = 0.0f;
	Personality.SqueezeTendency = 0.3f;
	Personality.BrakeDefensiveness = 0.2f;
	Evaluator->SetPersonality(Personality);

	// Test 3: Evaluate with empty sensor data (should return no defense needed)
	FAISensorData EmptySensorData;
	FAIDefensiveAction Result = Evaluator->Evaluate(EmptySensorData, 100.0f, 3);
	TestFalse(TEXT("Should not defend with no vehicles behind"), Result.bShouldDefend);

	// Test 4: Verify not defending initially
	TestFalse(TEXT("Should not be defending initially"), Evaluator->IsDefending());

	// Test 5: Stop defending
	Evaluator->StopDefending();
	TestFalse(TEXT("Should not be defending after stop"), Evaluator->IsDefending());

	return true;
}

/**
 * Test AI Slipstream System
 */
bool FAISlipstreamSystemTest::RunTest(const FString& Parameters)
{
	// Test 1: Create slipstream system
	UAISlipstreamSystem* Slipstream = NewObject<UAISlipstreamSystem>();
	TestNotNull(TEXT("Slipstream System should be created"), Slipstream);

	if (!Slipstream)
	{
		return false;
	}

	// Test 2: Verify initial state
	TestFalse(TEXT("Should not be drafting initially"), Slipstream->IsDrafting());
	TestEqual(TEXT("Initial speed boost should be 1.0"), Slipstream->GetSpeedBoost(), 1.0f);
	TestEqual(TEXT("Initial draft steering should be 0"), Slipstream->GetDraftSteering(), 0.0f);

	// Test 3: Test configuration
	FSlipstreamConfig Config;
	Config.bEnabled = true;
	Config.MaxDistance = 2500.0f;
	Config.MaxSpeedBoost = 1.12f;
	Config.MaxDragReduction = 0.35f;
	Slipstream->SetConfig(Config);

	const FSlipstreamConfig& RetrievedConfig = Slipstream->GetConfig();
	TestTrue(TEXT("Config should be enabled"), RetrievedConfig.bEnabled);
	TestEqual(TEXT("Max distance should match"), RetrievedConfig.MaxDistance, 2500.0f);

	// Test 4: Update with empty sensor data
	FAISensorData EmptySensorData;
	Slipstream->UpdateFromSensorData(EmptySensorData);
	TestFalse(TEXT("Should not be drafting with no slipstream"), Slipstream->IsDrafting());

	// Test 5: Verify effect state
	const FSlipstreamEffect& Effect = Slipstream->GetCurrentEffect();
	TestFalse(TEXT("Effect should show no slipstream"), Effect.bInSlipstream);
	TestEqual(TEXT("Effect strength should be 0"), Effect.Strength, 0.0f);

	return true;
}

/**
 * Test AI Rubber Band Scaler
 */
bool FAIRubberBandScalerTest::RunTest(const FString& Parameters)
{
	// Test 1: Create rubber band scaler
	UAIRubberBandScaler* Scaler = NewObject<UAIRubberBandScaler>();
	TestNotNull(TEXT("Rubber Band Scaler should be created"), Scaler);

	if (!Scaler)
	{
		return false;
	}

	// Test 2: Verify initial state
	TestEqual(TEXT("Initial speed multiplier should be 1.0"), Scaler->GetSpeedMultiplier(), 1.0f);
	TestEqual(TEXT("Initial brake point adjustment should be 0"), Scaler->GetBrakePointAdjustment(), 0.0f);
	TestEqual(TEXT("Initial cornering bonus should be 0"), Scaler->GetCorneringBonus(), 0.0f);
	TestFalse(TEXT("Should not be active initially"), Scaler->IsActive());

	// Test 3: Test configuration
	FAIRubberBandConfig Config;
	Config.Mode = ERubberBandMode::Hybrid;
	Config.CatchUpSpeedBoost = 0.15f;
	Config.MaxBoost = 0.25f;
	Config.MaxReduction = 0.15f;
	Config.ActivationDistance = 3000.0f;
	Scaler->SetConfig(Config);

	const FAIRubberBandConfig& RetrievedConfig = Scaler->GetConfig();
	TestEqual(TEXT("Mode should be Hybrid"), RetrievedConfig.Mode, ERubberBandMode::Hybrid);
	TestEqual(TEXT("Catch up boost should match"), RetrievedConfig.CatchUpSpeedBoost, 0.15f);

	// Test 4: Update with player nearby (within activation distance)
	Scaler->UpdateState(1000.0f, 2, 1, 0.5f); // 10m behind, position 2 vs player 1
	const FAIRubberBandState& State1 = Scaler->GetState();
	TestFalse(TEXT("Should not activate within activation distance"), State1.bActive);

	// Test 5: Update with player far behind
	Scaler->UpdateState(-5000.0f, 5, 1, 0.5f); // 50m behind, position 5 vs player 1
	const FAIRubberBandState& State2 = Scaler->GetState();
	TestTrue(TEXT("Should activate when far behind"), State2.bActive);
	TestTrue(TEXT("Speed multiplier should be boosted"), Scaler->GetSpeedMultiplier() > 1.0f);

	// Test 6: Update with player far ahead
	Scaler->UpdateState(5000.0f, 1, 5, 0.5f); // 50m ahead, position 1 vs player 5
	const FAIRubberBandState& State3 = Scaler->GetState();
	TestTrue(TEXT("Should activate when far ahead"), State3.bActive);
	TestTrue(TEXT("Speed multiplier should be reduced"), Scaler->GetSpeedMultiplier() < 1.0f);

	// Test 7: Verify clamping
	Scaler->UpdateState(-50000.0f, 10, 1, 0.5f); // Very far behind
	TestTrue(TEXT("Speed multiplier should be clamped"), Scaler->GetSpeedMultiplier() <= 1.25f);

	return true;
}

/**
 * Test AI Sensor System
 */
bool FAISensorSystemTest::RunTest(const FString& Parameters)
{
	// Test 1: Create sensor system
	UAISensorSystem* Sensor = NewObject<UAISensorSystem>();
	TestNotNull(TEXT("Sensor System should be created"), Sensor);

	if (!Sensor)
	{
		return false;
	}

	// Test 2: Verify initial sensor data
	const FAISensorData& Data = Sensor->GetSensorData();
	TestFalse(TEXT("Should not have vehicle ahead initially"), Data.bHasVehicleAhead);
	TestFalse(TEXT("Should not have vehicle behind initially"), Data.bHasVehicleBehind);
	TestFalse(TEXT("Should not have slipstream initially"), Data.bSlipstreamAvailable);
	TestEqual(TEXT("Initial slipstream strength should be 0"), Data.SlipstreamStrength, 0.0f);

	// Test 3: Test configuration
	FAISensorConfig Config;
	Config.ForwardRange = 3000.0f;
	Config.RearRange = 2000.0f;
	Config.SideRange = 1500.0f;
	Config.ForwardRayCount = 5;
	Config.ForwardSweepAngle = 25.0f;
	Sensor->SetSensorConfig(Config);
	// Should not crash

	// Test 4: Force scan (without world, should not crash)
	Sensor->ForceScan();
	// Should not crash even without valid world

	// Test 5: Get slipstream strength at direction
	float Strength = Sensor->GetSlipstreamStrengthAt(FVector::ForwardVector);
	TestEqual(TEXT("Slipstream strength should be 0 without source"), Strength, 0.0f);

	return true;
}
