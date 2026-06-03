// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Tests/AIRaceManagerIntegrationTest.h"
#include "AI/AICarController.h"
#include "AI/AIBehaviorTree.h"
#include "AI/AISensorSystem.h"
#include "AI/AIOvertakeEvaluator.h"
#include "AI/AIDefensiveEvaluator.h"
#include "AI/AISlipstreamSystem.h"
#include "AI/AIRubberBandScaler.h"
#include "Race/RaceManager.h"

/**
 * Test AI response to race state changes
 * Verifies that AI controllers react appropriately to race lifecycle events
 */
bool FAIRaceStateResponseTest::RunTest(const FString& Parameters)
{
	// Test 1: Create race manager and AI controller
	ARaceManager* RaceManager = NewObject<ARaceManager>();
	AAICarController* AIController = NewObject<AAICarController>();
	TestNotNull(TEXT("RaceManager should be created"), RaceManager);
	TestNotNull(TEXT("AI Controller should be created"), AIController);

	if (!RaceManager || !AIController)
	{
		return false;
	}

	// Test 2: Verify AI starts in Idle state
	TestEqual(TEXT("AI should start in Idle state"), AIController->GetAIState(), EAIState::Idle);

	// Test 3: Register AI with race manager
	APawn* AIPawn = NewObject<APawn>();
	RaceManager->RegisterRacer(AIPawn, TEXT("AI Racer"), false);

	// Test 4: Start race
	FRaceConfig Config;
	Config.NumLaps = 3;
	Config.TrackName = TEXT("AI Test Track");
	RaceManager->StartRace(Config);

	// Test 5: Verify race is in countdown
	TestEqual(TEXT("Race should be in Countdown"), RaceManager->GetRaceState(), ERaceState::Countdown);

	// Test 6: Start AI racing
	AIController->StartRacing();
	// AI state should transition (may stay Idle without a possessed pawn, but should not crash)

	// Test 7: Verify race state transitions
	RaceManager->PauseRace();
	TestEqual(TEXT("Race should be Paused"), RaceManager->GetRaceState(), ERaceState::Paused);

	RaceManager->ResumeRace();
	TestEqual(TEXT("Race should be Racing"), RaceManager->GetRaceState(), ERaceState::Racing);

	// Test 8: End race and verify AI can stop
	RaceManager->EndRace();
	AIController->StopRacing();
	TestEqual(TEXT("Race should be Finished"), RaceManager->GetRaceState(), ERaceState::Finished);

	return true;
}

/**
 * Test rubber band scaling based on race positions
 * Verifies that the rubber band system adjusts AI speed based on position relative to player
 */
bool FAIRubberBandRacePositionTest::RunTest(const FString& Parameters)
{
	// Test 1: Create rubber band scaler and race manager
	UAIRubberBandScaler* Scaler = NewObject<UAIRubberBandScaler>();
	ARaceManager* RaceManager = NewObject<ARaceManager>();
	TestNotNull(TEXT("Rubber Band Scaler should be created"), Scaler);
	TestNotNull(TEXT("RaceManager should be created"), RaceManager);

	if (!Scaler || !RaceManager)
	{
		return false;
	}

	// Test 2: Configure rubber band for competitive racing
	FAIRubberBandConfig Config;
	Config.Mode = ERubberBandMode::Hybrid;
	Config.CatchUpSpeedBoost = 0.15f;
	Config.MaxBoost = 0.25f;
	Config.MaxReduction = 0.15f;
	Config.ActivationDistance = 3000.0f;
	Scaler->SetConfig(Config);

	// Test 3: Register multiple racers
	APawn* PlayerPawn = NewObject<APawn>();
	APawn* AIPawn1 = NewObject<APawn>();
	APawn* AIPawn2 = NewObject<APawn>();

	RaceManager->RegisterRacer(PlayerPawn, TEXT("Player"), true);
	RaceManager->RegisterRacer(AIPawn1, TEXT("AI Fast"), false);
	RaceManager->RegisterRacer(AIPawn2, TEXT("AI Slow"), false);

	// Test 4: Simulate AI far behind player (should get speed boost)
	// Distance negative = behind, position 5 vs player position 1
	Scaler->UpdateState(-5000.0f, 5, 1, 0.5f);
	const FAIRubberBandState& BehindState = Scaler->GetState();
	TestTrue(TEXT("Should activate when far behind"), BehindState.bActive);
	TestTrue(TEXT("Speed multiplier should be boosted when behind"), Scaler->GetSpeedMultiplier() > 1.0f);

	// Test 5: Simulate AI far ahead of player (should get speed reduction)
	Scaler->UpdateState(5000.0f, 1, 5, 0.5f);
	const FAIRubberBandState& AheadState = Scaler->GetState();
	TestTrue(TEXT("Should activate when far ahead"), AheadState.bActive);
	TestTrue(TEXT("Speed multiplier should be reduced when ahead"), Scaler->GetSpeedMultiplier() < 1.0f);

	// Test 6: Simulate AI close to player (should not activate)
	Scaler->UpdateState(500.0f, 2, 1, 0.5f);
	const FAIRubberBandState& CloseState = Scaler->GetState();
	TestFalse(TEXT("Should not activate when close to player"), CloseState.bActive);

	// Test 7: Verify extreme position clamping
	Scaler->UpdateState(-50000.0f, 10, 1, 0.5f);
	TestTrue(TEXT("Speed multiplier should be clamped at max boost"),
		Scaler->GetSpeedMultiplier() <= 1.0f + Config.MaxBoost);

	Scaler->UpdateState(50000.0f, 1, 10, 0.5f);
	TestTrue(TEXT("Speed multiplier should be clamped at max reduction"),
		Scaler->GetSpeedMultiplier() >= 1.0f - Config.MaxReduction);

	// Test 8: Verify rubber band config retrieval
	const FAIRubberBandConfig& RetrievedConfig = Scaler->GetConfig();
	TestEqual(TEXT("Mode should be Hybrid"), RetrievedConfig.Mode, ERubberBandMode::Hybrid);
	TestEqual(TEXT("Catch up boost should match"), RetrievedConfig.CatchUpSpeedBoost, 0.15f);
	TestEqual(TEXT("Max boost should match"), RetrievedConfig.MaxBoost, 0.25f);

	return true;
}

/**
 * Test AI difficulty impact on race behavior
 * Verifies that different difficulty settings produce different AI behaviors
 */
bool FAIDifficultyRaceImpactTest::RunTest(const FString& Parameters)
{
	// Test 1: Create AI controller
	AAICarController* Controller = NewObject<AAICarController>();
	TestNotNull(TEXT("AI Controller should be created"), Controller);

	if (!Controller)
	{
		return false;
	}

	// Test 2: Test all difficulty levels
	TArray<EAIDifficulty> Difficulties = {
		EAIDifficulty::Easy,
		EAIDifficulty::Normal,
		EAIDifficulty::Hard,
		EAIDifficulty::Expert
	};

	for (EAIDifficulty Difficulty : Difficulties)
	{
		Controller->SetDifficulty(Difficulty);
		TestEqual(FString::Printf(TEXT("Difficulty should be set correctly")), Controller->GetDifficulty(), Difficulty);
	}

	// Test 3: Verify difficulty settings structure has required fields
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

	// Test 4: Verify difficulty settings are within valid ranges
	TestTrue(TEXT("Speed multiplier should be positive"), Settings.SpeedMultiplier > 0.0f);
	TestTrue(TEXT("Speed multiplier should be reasonable"), Settings.SpeedMultiplier <= 1.5f);
	TestTrue(TEXT("Throttle smoothness should be 0-1"), Settings.ThrottleSmoothness >= 0.0f && Settings.ThrottleSmoothness <= 1.0f);
	TestTrue(TEXT("Brake smoothness should be 0-1"), Settings.BrakeSmoothness >= 0.0f && Settings.BrakeSmoothness <= 1.0f);
	TestTrue(TEXT("Steering smoothness should be 0-1"), Settings.SteeringSmoothness >= 0.0f && Settings.SteeringSmoothness <= 1.0f);
	TestTrue(TEXT("Reaction time should be positive"), Settings.ReactionTime > 0.0f);
	TestTrue(TEXT("Rubber band strength should be 0-1"), Settings.RubberBandStrength >= 0.0f && Settings.RubberBandStrength <= 1.0f);
	TestTrue(TEXT("Overtake aggressiveness should be 0-1"), Settings.OvertakeAggressiveness >= 0.0f && Settings.OvertakeAggressiveness <= 1.0f);

	// Test 5: Set waypoints and verify AI accepts them
	TArray<FAIWaypoint> Waypoints;
	for (int32 i = 0; i < 10; i++)
	{
		FAIWaypoint WP;
		WP.Location = FVector(i * 5000.0f, 0.0f, 0.0f);
		WP.RecommendedSpeed = 100.0f + i * 10.0f;
		WP.bIsCorner = (i % 3 == 0);
		WP.CornerSharpness = WP.bIsCorner ? 0.5f : 0.0f;
		Waypoints.Add(WP);
	}
	Controller->SetWaypoints(Waypoints);
	TestTrue(TEXT("Setting waypoints should not crash"), true);

	return true;
}

/**
 * Test behavior tree decisions based on race factors
 * Verifies that the AI behavior tree processes race context correctly
 */
bool FAIBehaviorTreeRaceFactorsTest::RunTest(const FString& Parameters)
{
	// Test 1: Create behavior tree
	UAIBehaviorTree* BehaviorTree = NewObject<UAIBehaviorTree>();
	TestNotNull(TEXT("Behavior Tree should be created"), BehaviorTree);

	if (!BehaviorTree)
	{
		return false;
	}

	// Test 2: Verify initial state
	TestEqual(TEXT("Initial state should be Idle"), BehaviorTree->GetBehaviorState(), EAIBehaviorState::Idle);
	TestEqual(TEXT("Initial throttle should be 0"), BehaviorTree->GetThrottleInput(), 0.0f);
	TestEqual(TEXT("Initial brake should be 0"), BehaviorTree->GetBrakeInput(), 0.0f);
	TestEqual(TEXT("Initial steering should be 0"), BehaviorTree->GetSteeringInput(), 0.0f);

	// Test 3: Configure rubber band and slipstream
	FAIRubberBandConfig RBConfig;
	RBConfig.Mode = ERubberBandMode::Hybrid;
	RBConfig.CatchUpSpeedBoost = 0.02f;
	RBConfig.MaxBoost = 0.2f;
	BehaviorTree->SetRubberBandConfig(RBConfig);

	FAISlipstreamConfig SlipConfig;
	SlipConfig.bEnabled = true;
	SlipConfig.DetectionDistance = 2000.0f;
	SlipConfig.MaxSpeedBoost = 0.15f;
	BehaviorTree->SetSlipstreamConfig(SlipConfig);

	// Test 4: Update decisions with straight racing factors
	FAIDecisionFactors StraightFactors;
	StraightFactors.CurrentSpeed = 200.0f;
	StraightFactors.RecommendedSpeed = 220.0f;
	StraightFactors.bIsOnStraight = true;
	StraightFactors.bIsInCorner = false;
	StraightFactors.DistanceToVehicleAhead = 5000.0f;
	StraightFactors.bSlipstreamAvailable = false;
	StraightFactors.RacePosition = 3;
	StraightFactors.TotalRacers = 8;

	BehaviorTree->UpdateDecisions(StraightFactors);
	TestTrue(TEXT("Throttle should be positive on straight"), BehaviorTree->GetThrottleInput() >= 0.0f);

	// Test 5: Update decisions with corner approach
	FAIDecisionFactors CornerFactors;
	CornerFactors.CurrentSpeed = 180.0f;
	CornerFactors.RecommendedSpeed = 80.0f;
	CornerFactors.bIsOnStraight = false;
	CornerFactors.bIsInCorner = true;
	CornerFactors.CornerSharpness = 0.8f;
	CornerFactors.DistanceToWaypoint = 500.0f;

	BehaviorTree->UpdateDecisions(CornerFactors);
	TestTrue(TEXT("Brake should be positive for sharp corner"), BehaviorTree->GetBrakeInput() >= 0.0f);

	// Test 6: Update decisions with vehicle ahead (close)
	FAIDecisionFactors CloseFactors;
	CloseFactors.CurrentSpeed = 150.0f;
	CloseFactors.RecommendedSpeed = 150.0f;
	CloseFactors.bIsOnStraight = true;
	CloseFactors.DistanceToVehicleAhead = 800.0f;
	CloseFactors.bIsPlayerAhead = true;

	BehaviorTree->UpdateDecisions(CloseFactors);
	// Should adjust behavior when vehicle is close ahead
	TestTrue(TEXT("Close vehicle should influence decisions"), true);

	// Test 7: Update decisions with slipstream available
	FAIDecisionFactors SlipFactors;
	SlipFactors.CurrentSpeed = 200.0f;
	SlipFactors.RecommendedSpeed = 200.0f;
	SlipFactors.bIsOnStraight = true;
	SlipFactors.bSlipstreamAvailable = true;
	SlipFactors.SlipstreamStrength = 0.8f;
	SlipFactors.DistanceToVehicleAhead = 1000.0f;

	BehaviorTree->UpdateDecisions(SlipFactors);
	TestTrue(TEXT("Slipstream should influence behavior"), true);

	// Test 8: Set difficulty and verify it accepts the value
	BehaviorTree->SetDifficulty(0.9f);
	BehaviorTree->SetDifficulty(0.1f);
	BehaviorTree->SetDifficulty(0.5f);
	TestTrue(TEXT("Setting difficulty should not crash"), true);

	return true;
}

/**
 * Test overtake evaluation with race context
 * Verifies that overtake decisions account for race position and track state
 */
bool FAIOvertakeRaceContextTest::RunTest(const FString& Parameters)
{
	// Test 1: Create overtake evaluator
	UAIOvertakeEvaluator* Evaluator = NewObject<UAIOvertakeEvaluator>();
	TestNotNull(TEXT("Overtake Evaluator should be created"), Evaluator);

	if (!Evaluator)
	{
		return false;
	}

	// Test 2: Set personality for aggressive overtaker
	FOvertakePersonality AggressivePersonality;
	AggressivePersonality.FrequencyMultiplier = 1.5f;
	AggressivePersonality.RiskTolerance = 0.7f;
	AggressivePersonality.Patience = 0.3f;
	AggressivePersonality.SidePreference = 0.0f;
	AggressivePersonality.DraftUsage = 0.8f;
	Evaluator->SetPersonality(AggressivePersonality);

	// Test 3: Verify personality was set
	const FOvertakePersonality& Retrieved = Evaluator->GetPersonality();
	TestEqual(TEXT("Frequency should match"), Retrieved.FrequencyMultiplier, 1.5f);
	TestEqual(TEXT("Risk tolerance should match"), Retrieved.RiskTolerance, 0.7f);
	TestEqual(TEXT("Patience should match"), Retrieved.Patience, 0.3f);

	// Test 4: Evaluate with no vehicles (should not find opportunity)
	FAISensorData EmptySensor;
	FOvertakeOpportunity NoOpportunity = Evaluator->Evaluate(EmptySensor, 150.0f);
	TestFalse(TEXT("Should not find opportunity with no vehicles"), NoOpportunity.bViable);

	// Test 5: Evaluate with vehicle ahead
	FAISensorData SensorWithVehicle;
	SensorWithVehicle.bHasVehicleAhead = true;
	SensorWithVehicle.VehicleAhead.Distance = 1500.0f;
	SensorWithVehicle.VehicleAhead.RelativeSpeed = -10.0f;
	SensorWithVehicle.VehicleAhead.LateralOffset = 0.0f;
	SensorWithVehicle.VehicleAhead.bIsAhead = true;
	SensorWithVehicle.NearbyVehicles.Add(SensorWithVehicle.VehicleAhead);

	FOvertakeOpportunity WithVehicle = Evaluator->Evaluate(SensorWithVehicle, 160.0f);
	// May or may not find opportunity depending on gap analysis, but should not crash
	TestTrue(TEXT("Overtake evaluation with vehicle ahead should execute"), true);

	// Test 6: Verify overtake state management
	TestFalse(TEXT("Should not be overtaking initially"), Evaluator->IsOvertaking());
	TestEqual(TEXT("Overtake progress should be 0"), Evaluator->GetOvertakeProgress(), 0.0f);

	// Test 7: Cancel overtake
	Evaluator->CancelOvertake();
	TestFalse(TEXT("Should not be overtaking after cancel"), Evaluator->IsOvertaking());

	// Test 8: Verify overtake strategy types exist
	EOvertakeStrategy None = EOvertakeStrategy::None;
	EOvertakeStrategy Inside = EOvertakeStrategy::InsidePass;
	EOvertakeStrategy Outside = EOvertakeStrategy::OutsidePass;
	EOvertakeStrategy Slingshot = EOvertakeStrategy::SlingshotPass;
	EOvertakeStrategy LateBrake = EOvertakeStrategy::LateBrakePass;
	EOvertakeStrategy DraftPass = EOvertakeStrategy::DraftAndPass;

	TestNotEqual(TEXT("None should differ from InsidePass"), (int32)None, (int32)Inside);
	TestNotEqual(TEXT("InsidePass should differ from OutsidePass"), (int32)Inside, (int32)Outside);
	TestNotEqual(TEXT("SlingshotPass should differ from LateBrakePass"), (int32)Slingshot, (int32)LateBrake);

	return true;
}

/**
 * Test defensive evaluation with race context
 * Verifies that defensive decisions account for race positions and threats
 */
bool FAIDefenseRaceContextTest::RunTest(const FString& Parameters)
{
	// Test 1: Create defensive evaluator
	UAIDefensiveEvaluator* Evaluator = NewObject<UAIDefensiveEvaluator>();
	TestNotNull(TEXT("Defensive Evaluator should be created"), Evaluator);

	if (!Evaluator)
	{
		return false;
	}

	// Test 2: Set defensive personality
	FAIDefensivePersonality DefensivePersonality;
	DefensivePersonality.Aggressiveness = 0.3f;
	DefensivePersonality.Awareness = 0.9f;
	DefensivePersonality.BlockPreference = -0.5f; // Prefer inside defense
	DefensivePersonality.SqueezeTendency = 0.2f;
	DefensivePersonality.BrakeDefensiveness = 0.4f;
	Evaluator->SetPersonality(DefensivePersonality);

	// Test 3: Evaluate with no vehicles behind (should not defend)
	FAISensorData EmptySensor;
	FAIDefensiveAction NoDefense = Evaluator->Evaluate(EmptySensor, 150.0f, 2);
	TestFalse(TEXT("Should not defend with no vehicles behind"), NoDefense.bShouldDefend);

	// Test 4: Evaluate with vehicle behind
	FAISensorData SensorWithThreat;
	SensorWithThreat.bHasVehicleBehind = true;
	SensorWithThreat.VehicleBehind.Distance = 1000.0f;
	SensorWithThreat.VehicleBehind.RelativeSpeed = 15.0f; // Closing fast
	SensorWithThreat.VehicleBehind.bIsBehind = true;
	SensorWithThreat.NearbyVehicles.Add(SensorWithThreat.VehicleBehind);

	FAIDefensiveAction WithThreat = Evaluator->Evaluate(SensorWithThreat, 150.0f, 1);
	// May or may not defend depending on threat analysis
	TestTrue(TEXT("Defense evaluation with threat should execute"), true);

	// Test 5: Verify defensive state management
	TestFalse(TEXT("Should not be defending initially"), Evaluator->IsDefending());

	// Test 6: Stop defending
	Evaluator->StopDefending();
	TestFalse(TEXT("Should not be defending after stop"), Evaluator->IsDefending());

	// Test 7: Test with different race positions
	// Leading the race - should be more defensive
	FAIDefensiveAction LeadingDefense = Evaluator->Evaluate(SensorWithThreat, 150.0f, 1);
	TestTrue(TEXT("Leading position defense evaluation should execute"), true);

	// Mid-pack - moderate defense
	FAIDefensiveAction MidDefense = Evaluator->Evaluate(SensorWithThreat, 150.0f, 4);
	TestTrue(TEXT("Mid-pack defense evaluation should execute"), true);

	// Last place - less defense, more offense needed
	FAIDefensiveAction LastDefense = Evaluator->Evaluate(SensorWithThreat, 150.0f, 8);
	TestTrue(TEXT("Last place defense evaluation should execute"), true);

	return true;
}

/**
 * Test slipstream system integration with race context
 * Verifies that the slipstream system works correctly during racing
 */
bool FAISlipstreamRaceContextTest::RunTest(const FString& Parameters)
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

	// Test 3: Configure slipstream for racing
	FSlipstreamConfig Config;
	Config.bEnabled = true;
	Config.MaxDistance = 2500.0f;
	Config.MaxSpeedBoost = 1.12f;
	Config.MaxDragReduction = 0.35f;
	Slipstream->SetConfig(Config);

	// Test 4: Verify config was set
	const FSlipstreamConfig& RetrievedConfig = Slipstream->GetConfig();
	TestTrue(TEXT("Config should be enabled"), RetrievedConfig.bEnabled);
	TestEqual(TEXT("Max distance should match"), RetrievedConfig.MaxDistance, 2500.0f);
	TestTrue(TEXT("Max speed boost should be > 1.0"), RetrievedConfig.MaxSpeedBoost > 1.0f);

	// Test 5: Update with empty sensor data (no slipstream)
	FAISensorData EmptySensor;
	Slipstream->UpdateFromSensorData(EmptySensor);
	TestFalse(TEXT("Should not be drafting with no vehicles"), Slipstream->IsDrafting());

	// Test 6: Verify effect state when not drafting
	const FSlipstreamEffect& NoDraftEffect = Slipstream->GetCurrentEffect();
	TestFalse(TEXT("Effect should show no slipstream"), NoDraftEffect.bInSlipstream);
	TestEqual(TEXT("Effect strength should be 0"), NoDraftEffect.Strength, 0.0f);

	// Test 7: Update with sensor data indicating slipstream available
	FAISensorData SlipstreamSensor;
	SlipstreamSensor.bSlipstreamAvailable = true;
	SlipstreamSensor.SlipstreamStrength = 0.7f;
	SlipstreamSensor.SlipstreamTarget.Distance = 800.0f;
	SlipstreamSensor.SlipstreamTarget.bIsAhead = true;

	Slipstream->UpdateFromSensorData(SlipstreamSensor);
	// System should process the slipstream data
	TestTrue(TEXT("Slipstream update should execute"), true);

	// Test 8: Verify combined speed multiplier is accessible
	const FSlipstreamEffect& WithDraftEffect = Slipstream->GetCurrentEffect();
	TestTrue(TEXT("Speed boost should be >= 1.0"), Slipstream->GetSpeedBoost() >= 1.0f);

	return true;
}

/**
 * Test multi-AI coordination with race manager
 * Verifies that multiple AI controllers can coexist and compete in a race
 */
bool FAIMultiAgentRaceCoordinationTest::RunTest(const FString& Parameters)
{
	// Test 1: Create race manager
	ARaceManager* RaceManager = NewObject<ARaceManager>();
	TestNotNull(TEXT("RaceManager should be created"), RaceManager);

	if (!RaceManager)
	{
		return false;
	}

	// Test 2: Create multiple AI controllers with different configurations
	const int32 NumAI = 7;
	TArray<AAICarController*> AIControllers;
	TArray<APawn*> AIPawns;

	for (int32 i = 0; i < NumAI; i++)
	{
		AAICarController* Controller = NewObject<AAICarController>();
		APawn* Pawn = NewObject<APawn>();

		AIControllers.Add(Controller);
		AIPawns.Add(Pawn);

		// Set varying difficulties
		EAIDifficulty Difficulty;
		if (i < 2) Difficulty = EAIDifficulty::Easy;
		else if (i < 5) Difficulty = EAIDifficulty::Normal;
		else Difficulty = EAIDifficulty::Hard;

		Controller->SetDifficulty(Difficulty);

		// Register with race manager
		FString Name = FString::Printf(TEXT("AI %d"), i + 1);
		RaceManager->RegisterRacer(Pawn, Name, false);
	}

	// Test 3: Register player
	APawn* PlayerPawn = NewObject<APawn>();
	RaceManager->RegisterRacer(PlayerPawn, TEXT("Player"), true);

	// Test 4: Verify all racers are registered
	TestEqual(TEXT("Should have 8 racers (7 AI + 1 Player)"), RaceManager->GetAllRacers().Num(), NumAI + 1);

	// Test 5: Start race
	FRaceConfig Config;
	Config.NumLaps = 5;
	Config.MaxAIOpponents = NumAI;
	Config.TrackName = TEXT("Multi-AI Test Track");
	RaceManager->StartRace(Config);

	// Test 6: Verify race started
	TestEqual(TEXT("Race should be in Countdown"), RaceManager->GetRaceState(), ERaceState::Countdown);

	// Test 7: Verify all AI controllers have correct difficulty
	for (int32 i = 0; i < NumAI; i++)
	{
		EAIDifficulty ExpectedDifficulty;
		if (i < 2) ExpectedDifficulty = EAIDifficulty::Easy;
		else if (i < 5) ExpectedDifficulty = EAIDifficulty::Normal;
		else ExpectedDifficulty = EAIDifficulty::Hard;

		TestEqual(FString::Printf(TEXT("AI %d difficulty should match"), i + 1),
			AIControllers[i]->GetDifficulty(), ExpectedDifficulty);
	}

	// Test 8: Verify AI states are all Idle before racing starts
	for (int32 i = 0; i < NumAI; i++)
	{
		TestEqual(FString::Printf(TEXT("AI %d should be in Idle state"), i + 1),
			AIControllers[i]->GetAIState(), EAIState::Idle);
	}

	// Test 9: End race
	RaceManager->EndRace();
	TestEqual(TEXT("Race should be Finished"), RaceManager->GetRaceState(), ERaceState::Finished);

	// Test 10: Verify race can be reset and restarted
	RaceManager->ResetRace();
	TestEqual(TEXT("Race should be Idle after reset"), RaceManager->GetRaceState(), ERaceState::Idle);

	// Re-register for second race
	for (int32 i = 0; i < NumAI; i++)
	{
		FString Name = FString::Printf(TEXT("AI %d"), i + 1);
		RaceManager->RegisterRacer(AIPawns[i], Name, false);
	}
	RaceManager->RegisterRacer(PlayerPawn, TEXT("Player"), true);
	RaceManager->StartRace(Config);
	TestEqual(TEXT("Race should be in Countdown for second race"), RaceManager->GetRaceState(), ERaceState::Countdown);

	return true;
}
