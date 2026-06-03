// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NOMI/CommentaryEngine.h"

/**
 * NOMI System Tests
 * Tests for commentary engine, comment matching, and queue management
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommentaryEngineInitTest,
	"NomiRacingPlus.NOMI.CommentaryEngineInit",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommentMatchingTest,
	"NomiRacingPlus.NOMI.CommentMatching",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommentQueueTest,
	"NomiRacingPlus.NOMI.CommentQueue",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommentCooldownTest,
	"NomiRacingPlus.NOMI.CommentCooldown",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNOMIEmotionTest,
	"NomiRacingPlus.NOMI.Emotion",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
