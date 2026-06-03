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

/**
 * AI System Tests
 * Tests for AI difficulty settings, behavior, overtaking, defending, slipstream, and rubber banding
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIDifficultySettingsTest,
	"NomiRacingPlus.AI.DifficultySettings",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIStateTransitionTest,
	"NomiRacingPlus.AI.StateTransitions",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIWaypointTest,
	"NomiRacingPlus.AI.Waypoints",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIBehaviorTreeTest,
	"NomiRacingPlus.AI.BehaviorTree",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIOvertakeEvaluatorTest,
	"NomiRacingPlus.AI.OvertakeEvaluator",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIDefensiveEvaluatorTest,
	"NomiRacingPlus.AI.DefensiveEvaluator",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAISlipstreamSystemTest,
	"NomiRacingPlus.AI.SlipstreamSystem",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIRubberBandScalerTest,
	"NomiRacingPlus.AI.RubberBandScaler",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAISensorSystemTest,
	"NomiRacingPlus.AI.SensorSystem",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
