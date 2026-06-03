// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NOMI/CommentaryEngine.h"
#include "NOMI/NOMIController.h"
#include "Race/RaceManager.h"

/**
 * NOMI-Event System Integration Tests
 * Tests the full pipeline from race events through the commentary engine
 * to NOMI controller expression updates.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNOMIEventToCommentPipelineTest,
	"NomiRacingPlus.Integration.NOMIEvent.EventToCommentPipeline",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNOMIEmotionStateMachineTest,
	"NomiRacingPlus.Integration.NOMIEvent.EmotionStateMachine",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNOMICommentPriorityTest,
	"NomiRacingPlus.Integration.NOMIEvent.CommentPriority",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNOMINIOVehicleCommentTest,
	"NomiRacingPlus.Integration.NOMIEvent.NIOVehicleComments",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNOMIRaceEventBridgingTest,
	"NomiRacingPlus.Integration.NOMIEvent.RaceEventBridging",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNOMIExpressionMappingTest,
	"NomiRacingPlus.Integration.NOMIEvent.ExpressionMapping",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNOMICommentVariableReplacementTest,
	"NomiRacingPlus.Integration.NOMIEvent.CommentVariableReplacement",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
