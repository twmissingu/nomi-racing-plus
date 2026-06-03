// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "UI/NIOColorTheme.h"
#include "UI/AccessibilityManager.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"

UNIOThemeSubsystem::UNIOThemeSubsystem()
{
	// Initialize default theme values

	// ── Typography Defaults ─────────────────────────────────────────────
	Theme.Typography.H1 = { 96, ENIOFontWeight::Bold, 1.1f, 2.0f };
	Theme.Typography.H2 = { 48, ENIOFontWeight::SemiBold, 1.2f, 1.5f };
	Theme.Typography.H3 = { 36, ENIOFontWeight::SemiBold, 1.3f, 1.0f };
	Theme.Typography.H4 = { 24, ENIOFontWeight::Medium, 1.4f, 0.5f };
	Theme.Typography.Body = { 18, ENIOFontWeight::Regular, 1.5f, 0.0f };
	Theme.Typography.BodySmall = { 16, ENIOFontWeight::Regular, 1.5f, 0.0f };
	Theme.Typography.Small = { 14, ENIOFontWeight::Regular, 1.4f, 0.25f };
	Theme.Typography.Caption = { 12, ENIOFontWeight::Light, 1.4f, 0.5f };
	Theme.Typography.Overline = { 10, ENIOFontWeight::Medium, 1.6f, 1.5f };
	Theme.Typography.SpeedFontSize = 96;
	Theme.Typography.SpeedFontWeight = ENIOFontWeight::Bold;
	Theme.Typography.SpeedLetterSpacing = 2.0f;
	Theme.Typography.SpeedWarningThresholdKmh = 150;
	Theme.Typography.SpeedDangerThresholdKmh = 250;
	Theme.Typography.TimerFontSize = 28;
	Theme.Typography.TimerLetterSpacing = 1.0f;

	// ── Spacing Defaults ────────────────────────────────────────────────
	Theme.Spacing.BaseUnit = 4;
	Theme.Spacing.Xs = 4;
	Theme.Spacing.Sm = 8;
	Theme.Spacing.Md = 16;
	Theme.Spacing.Lg = 24;
	Theme.Spacing.Xl = 32;
	Theme.Spacing.Xxl = 48;
	Theme.Spacing.Xxxl = 64;
	Theme.Spacing.ButtonPaddingH = 24;
	Theme.Spacing.ButtonPaddingV = 12;
	Theme.Spacing.CardPadding = 16;
	Theme.Spacing.PanelPadding = 24;
	Theme.Spacing.SectionGap = 32;
	Theme.Spacing.PageMargin = 48;

	// ── Border Radius Defaults ──────────────────────────────────────────
	Theme.BorderRadius.Sm = 4;
	Theme.BorderRadius.Md = 8;
	Theme.BorderRadius.Lg = 12;
	Theme.BorderRadius.Xl = 16;
	Theme.BorderRadius.Round = 999;

	// ── Animation Defaults ──────────────────────────────────────────────
	Theme.AnimDurations.Instant = 0.05f;
	Theme.AnimDurations.Fast = 0.15f;
	Theme.AnimDurations.Normal = 0.3f;
	Theme.AnimDurations.Slow = 0.5f;
	Theme.AnimDurations.Slower = 0.8f;
	Theme.AnimDurations.Glacial = 1.2f;

	Theme.ButtonHoverAnim = { 0.15f, ENIOEasingType::EaseOut, 0.0f };
	Theme.ButtonPressAnim = { 0.05f, ENIOEasingType::Linear, 0.0f };
	Theme.PanelSlideAnim = { 0.3f, ENIOEasingType::EaseOut, 0.0f };
	Theme.NOMICommentAnim = { 0.3f, ENIOEasingType::EaseOut, 0.0f };
	Theme.CountdownAnim = { 0.5f, ENIOEasingType::Bounce, 0.0f };

	// ── Effects Defaults ────────────────────────────────────────────────
	Theme.Effects.ShadowSmallBlur = 2.0f;
	Theme.Effects.ShadowMediumBlur = 8.0f;
	Theme.Effects.ShadowLargeBlur = 16.0f;
	Theme.Effects.GlowBlueSmall = 4.0f;
	Theme.Effects.GlowBlueMedium = 10.0f;
	Theme.Effects.GlowBlueLarge = 20.0f;
	Theme.Effects.GlowCyanMedium = 10.0f;
	Theme.Effects.BackgroundBlur = 10.0f;
	Theme.Effects.OverlayBlur = 20.0f;

	// ── Breakpoint Defaults ─────────────────────────────────────────────
	Theme.Layout.Mobile = { 768, 0.8f, 0.75f, 0.85f };
	Theme.Layout.Tablet = { 1280, 1.0f, 0.9f, 0.95f };
	Theme.Layout.Desktop = { 1920, 1.0f, 1.0f, 1.0f };
	Theme.Layout.Ultrawide = { 3440, 1.1f, 1.05f, 1.0f };
	Theme.Layout.FourK = { 0, 1.5f, 1.5f, 1.3f };

	// ── Safe Zone Defaults ──────────────────────────────────────────────
	Theme.Layout.TitleSafe = { 0.035f, 0.035f };
	Theme.Layout.ActionSafe = { 0.05f, 0.05f };
	Theme.Layout.HUDMarginTop = 40;
	Theme.Layout.HUDMarginRight = 40;
	Theme.Layout.HUDMarginBottom = 40;
	Theme.Layout.HUDMarginLeft = 40;

	// ── HUD Layout Defaults ─────────────────────────────────────────────
	Theme.Layout.SpeedDisplay = { ENIOAnchor::BottomCenter, 0.0f, -60.0f, 0.0f, 0.0f, 10 };
	Theme.Layout.PositionDisplay = { ENIOAnchor::TopRight, -40.0f, 40.0f, 0.0f, 0.0f, 10 };
	Theme.Layout.LapDisplay = { ENIOAnchor::TopLeft, 40.0f, 40.0f, 0.0f, 0.0f, 10 };
	Theme.Layout.TimerDisplay = { ENIOAnchor::TopCenter, 0.0f, 40.0f, 0.0f, 0.0f, 10 };
	Theme.Layout.NOMIComment = { ENIOAnchor::BottomLeft, 40.0f, -160.0f, 400.0f, 100.0f, 20 };
	Theme.Layout.Minimap = { ENIOAnchor::BottomRight, -40.0f, -40.0f, 200.0f, 200.0f, 10 };
	Theme.Layout.ThrottleBar = { ENIOAnchor::CenterRight, -40.0f, 40.0f, 8.0f, 120.0f, 10 };
	Theme.Layout.BrakeBar = { ENIOAnchor::CenterRight, -40.0f, 180.0f, 8.0f, 120.0f, 10 };
	Theme.Layout.BatteryBar = { ENIOAnchor::BottomRight, -40.0f, -260.0f, 200.0f, 6.0f, 10 };
	Theme.Layout.Countdown = { ENIOAnchor::Center, 0.0f, 0.0f, 0.0f, 0.0f, 100 };

	// ── Menu Layout Defaults ────────────────────────────────────────────
	Theme.Layout.MenuMaxWidth = 1200;
	Theme.Layout.MenuButtonHeight = 56;
	Theme.Layout.MenuButtonSpacing = 12;
}

ENIOBreakpoint UNIOThemeSubsystem::DetectBreakpoint() const
{
	if (!GEngine || !GEngine->GameViewport)
	{
		return ENIOBreakpoint::Desktop;
	}

	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize);
	const int32 Width = FMath::RoundToInt(ViewportSize.X);

	if (Width <= Theme.Layout.Mobile.MaxWidth)
	{
		return ENIOBreakpoint::Mobile;
	}
	if (Width <= Theme.Layout.Tablet.MaxWidth)
	{
		return ENIOBreakpoint::Tablet;
	}
	if (Width <= Theme.Layout.Desktop.MaxWidth)
	{
		return ENIOBreakpoint::Desktop;
	}
	if (Width <= Theme.Layout.Ultrawide.MaxWidth)
	{
		return ENIOBreakpoint::Ultrawide;
	}
	return ENIOBreakpoint::FourK;
}

float UNIOThemeSubsystem::GetCurrentScaleFactor() const
{
	switch (DetectBreakpoint())
	{
	case ENIOBreakpoint::Mobile:    return Theme.Layout.Mobile.ScaleFactor;
	case ENIOBreakpoint::Tablet:    return Theme.Layout.Tablet.ScaleFactor;
	case ENIOBreakpoint::Desktop:   return Theme.Layout.Desktop.ScaleFactor;
	case ENIOBreakpoint::Ultrawide: return Theme.Layout.Ultrawide.ScaleFactor;
	case ENIOBreakpoint::FourK:     return Theme.Layout.FourK.ScaleFactor;
	default:                        return 1.0f;
	}
}

FVector2D UNIOThemeSubsystem::CalculateHUDPosition(
	const FNIOHUDElementLayout& Element,
	const FVector2D& ScreenSize) const
{
	const float Scale = GetCurrentScaleFactor();
	const float MarginTop = Theme.Layout.HUDMarginTop * Scale;
	const float MarginRight = Theme.Layout.HUDMarginRight * Scale;
	const float MarginBottom = Theme.Layout.HUDMarginBottom * Scale;
	const float MarginLeft = Theme.Layout.HUDMarginLeft * Scale;

	const float OffsetX = Element.OffsetX * Scale;
	const float OffsetY = Element.OffsetY * Scale;
	const float ElemW = Element.Width * Scale;
	const float ElemH = Element.Height * Scale;

	float X = 0.0f;
	float Y = 0.0f;

	switch (Element.Anchor)
	{
	case ENIOAnchor::TopLeft:
		X = MarginLeft + OffsetX;
		Y = MarginTop + OffsetY;
		break;
	case ENIOAnchor::TopCenter:
		X = (ScreenSize.X - ElemW) * 0.5f + OffsetX;
		Y = MarginTop + OffsetY;
		break;
	case ENIOAnchor::TopRight:
		X = ScreenSize.X - MarginRight - ElemW + OffsetX;
		Y = MarginTop + OffsetY;
		break;
	case ENIOAnchor::CenterLeft:
		X = MarginLeft + OffsetX;
		Y = (ScreenSize.Y - ElemH) * 0.5f + OffsetY;
		break;
	case ENIOAnchor::Center:
		X = (ScreenSize.X - ElemW) * 0.5f + OffsetX;
		Y = (ScreenSize.Y - ElemH) * 0.5f + OffsetY;
		break;
	case ENIOAnchor::CenterRight:
		X = ScreenSize.X - MarginRight - ElemW + OffsetX;
		Y = (ScreenSize.Y - ElemH) * 0.5f + OffsetY;
		break;
	case ENIOAnchor::BottomLeft:
		X = MarginLeft + OffsetX;
		Y = ScreenSize.Y - MarginBottom - ElemH + OffsetY;
		break;
	case ENIOAnchor::BottomCenter:
		X = (ScreenSize.X - ElemW) * 0.5f + OffsetX;
		Y = ScreenSize.Y - MarginBottom - ElemH + OffsetY;
		break;
	case ENIOAnchor::BottomRight:
		X = ScreenSize.X - MarginRight - ElemW + OffsetX;
		Y = ScreenSize.Y - MarginBottom - ElemH + OffsetY;
		break;
	}

	return FVector2D(X, Y);
}

FNIOAnimTransition UNIOThemeSubsystem::GetTransition(const FString& TransitionName) const
{
	if (TransitionName == TEXT("ButtonHover"))   return Theme.ButtonHoverAnim;
	if (TransitionName == TEXT("ButtonPress"))   return Theme.ButtonPressAnim;
	if (TransitionName == TEXT("PanelSlide"))    return Theme.PanelSlideAnim;
	if (TransitionName == TEXT("NOMIComment"))   return Theme.NOMICommentAnim;
	if (TransitionName == TEXT("Countdown"))     return Theme.CountdownAnim;

	// Default fallback
	return { Theme.AnimDurations.Normal, ENIOEasingType::EaseOut, 0.0f };
}

// ── Accessibility Integration ──────────────────────────────────────────────

void UNIOThemeSubsystem::SetAccessibilityManager(UAccessibilityManager* InManager)
{
	AccessibilityManager = InManager;
}

FLinearColor UNIOThemeSubsystem::GetAccessibleColor(const FLinearColor& OriginalColor) const
{
	if (AccessibilityManager)
	{
		return AccessibilityManager->GetAdaptedColor(OriginalColor);
	}

	// Fallback: use color theme's own flags
	FLinearColor Result = OriginalColor;
	if (Theme.Colors.bColorblindMode)
	{
		// Simple red->orange, green->blue swap for known status colors
		if (Result.Equals(Theme.Colors.Success, 0.1f))
		{
			Result = Theme.Colors.ColorblindSuccess;
		}
		else if (Result.Equals(Theme.Colors.Danger, 0.1f))
		{
			Result = Theme.Colors.ColorblindDanger;
		}
	}
	return Result;
}

FLinearColor UNIOThemeSubsystem::GetAccessibleSuccessColor() const
{
	if (AccessibilityManager)
	{
		return AccessibilityManager->GetSuccessColor();
	}
	return Theme.Colors.bColorblindMode ? Theme.Colors.ColorblindSuccess : Theme.Colors.Success;
}

FLinearColor UNIOThemeSubsystem::GetAccessibleDangerColor() const
{
	if (AccessibilityManager)
	{
		return AccessibilityManager->GetDangerColor();
	}
	return Theme.Colors.bColorblindMode ? Theme.Colors.ColorblindDanger : Theme.Colors.Danger;
}

int32 UNIOThemeSubsystem::GetAccessibleFontSize(int32 BaseSize) const
{
	if (AccessibilityManager)
	{
		return AccessibilityManager->GetFontSize(BaseSize);
	}
	return BaseSize;
}
