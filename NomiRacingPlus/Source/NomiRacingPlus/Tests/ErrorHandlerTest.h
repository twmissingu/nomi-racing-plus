// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

// NomiErrorHandler Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FErrorHandlerResultTest,
	"NomiRacingPlus.ErrorHandler.Result",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FErrorHandlerResultVoidTest,
	"NomiRacingPlus.ErrorHandler.ResultVoid",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FErrorHandlerCheckPointerTest,
	"NomiRacingPlus.ErrorHandler.CheckPointer",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FErrorHandlerValidateTest,
	"NomiRacingPlus.ErrorHandler.Validate",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FErrorHandlerSafeDivideTest,
	"NomiRacingPlus.ErrorHandler.SafeDivide",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FErrorHandlerValidateFileExistsTest,
	"NomiRacingPlus.ErrorHandler.ValidateFileExists",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

// ErrorToastWidget Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FToastShowAndDismissTest,
	"NomiRacingPlus.Toast.ShowAndDismiss",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FToastMaxVisibleTest,
	"NomiRacingPlus.Toast.MaxVisible",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FToastSeverityColorTest,
	"NomiRacingPlus.Toast.SeverityColor",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FToastEmptyMessageTest,
	"NomiRacingPlus.Toast.EmptyMessage",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

// CommentaryEngine TOptional Return Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommentaryEngineTOptionalReturnTest,
	"NomiRacingPlus.CommentaryEngine.TOptionalReturn",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommentaryEngineVariableReplacementTest,
	"NomiRacingPlus.CommentaryEngine.VariableReplacement",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCommentaryEngineQueueTest,
	"NomiRacingPlus.CommentaryEngine.Queue",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

// ProgressionSerializer CRC32 Checksum Tests
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionSerializerChecksumTest,
	"NomiRacingPlus.ProgressionSerializer.Checksum",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionSerializerRoundTripTest,
	"NomiRacingPlus.ProgressionSerializer.RoundTrip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FProgressionSerializerCorruptionTest,
	"NomiRacingPlus.ProgressionSerializer.Corruption",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
