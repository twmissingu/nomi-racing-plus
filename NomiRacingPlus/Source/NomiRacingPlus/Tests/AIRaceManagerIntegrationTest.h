// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "AI/AICarController.h"
#include "AI/AIBehaviorTree.h"
#include "AI/AISensorSystem.h"
#include "AI/AIOvertakeEvaluator.h"
#include "AI/AIDefensiveEvaluator.h"
#include "AI/AISlipstreamSystem.h"
#include "AI/AIRubberBandScaler.h"
#include "Race/RaceManager.h"

/**
 * AI-Race Manager Communication Integration Tests
 * Tests the interaction between AI systems and the race manager:
 * AI difficulty propagation, rubber band scaling with race positions,
 * behavior tree decisions based on race state, and sensor-driven race events.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIRaceStateResponseTest,
	"NomiRacingPlus.Integration.AIRace.StateResponse",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIRubberBandRacePositionTest,
	"NomiRacingPlus.Integration.AIRace.RubberBandPosition",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIDifficultyRaceImpactTest,
	"NomiRacingPlus.Integration.AIRace.DifficultyImpact",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIBehaviorTreeRaceFactorsTest,
	"NomiRacingPlus.Integration.AIRace.BehaviorTreeFactors",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIOvertakeRaceContextTest,
	"NomiRacingPlus.Integration.AIRace.OvertakeContext",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIDefenseRaceContextTest,
	"NomiRacingPlus.Integration.AIRace.DefenseContext",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAISlipstreamRaceContextTest,
	"NomiRacingPlus.Integration.AIRace.SlipstreamContext",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIMultiAgentRaceCoordinationTest,
	"NomiRacingPlus.Integration.AIRace.MultiAgentCoordination",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
