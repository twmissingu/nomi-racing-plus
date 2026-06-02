// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NIOColorTheme.generated.h"

// ─── Animation Easing Curves ───────────────────────────────────────────────

/**
 * Easing function types for UI animations
 */
UENUM(BlueprintType)
enum class ENIOEasingType : uint8
{
	Linear        UMETA(DisplayName = "Linear"),
	EaseIn        UMETA(DisplayName = "Ease In"),
	EaseOut       UMETA(DisplayName = "Ease Out"),
	EaseInOut     UMETA(DisplayName = "Ease In Out"),
	EaseOutBack   UMETA(DisplayName = "Ease Out Back (Overshoot)"),
	EaseInBack    UMETA(DisplayName = "Ease In Back (Anticipate)"),
	Bounce        UMETA(DisplayName = "Bounce"),
	Spring        UMETA(DisplayName = "Spring"),
	Sharp         UMETA(DisplayName = "Sharp"),
	Smooth        UMETA(DisplayName = "Smooth")
};

// ─── Animation Duration Presets ────────────────────────────────────────────

/**
 * Animation duration presets in seconds
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIODurationPresets
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duration")
	float Instant = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duration")
	float Fast = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duration")
	float Normal = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duration")
	float Slow = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duration")
	float Slower = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Duration")
	float Glacial = 1.2f;
};

// ─── Animation Transition Config ──────────────────────────────────────────

/**
 * Defines an animation transition with duration and easing
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOAnimTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float Duration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	ENIOEasingType Easing = ENIOEasingType::EaseOut;

	/** Optional delay before animation starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float Delay = 0.0f;
};

// ─── Primary Brand Colors ─────────────────────────────────────────────────

/**
 * NIO Brand Color Theme
 * Complete color system based on NIO's design language
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOColorTheme
{
	GENERATED_BODY()

	// ── Primary NIO Colors ──────────────────────────────────────────────

	/** NIO Blue - primary brand color (#00A1E0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Primary")
	FLinearColor NIOBlue = FLinearColor(0.0f, 0.631f, 0.878f, 1.0f);

	/** NIO Cyan - accent/highlight color (#00D4FF) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Primary")
	FLinearColor NIOCyan = FLinearColor(0.0f, 0.831f, 1.0f, 1.0f);

	/** NIO Blue Dark - pressed/active states (#0078B4) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Primary")
	FLinearColor NIOBlueDark = FLinearColor(0.0f, 0.471f, 0.706f, 1.0f);

	/** NIO Blue Light - hover states (#4DC9F6) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Primary")
	FLinearColor NIOBlueLight = FLinearColor(0.302f, 0.788f, 0.965f, 1.0f);

	/** NIO Blue Pale - subtle backgrounds (#B3E5FC) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Primary")
	FLinearColor NIOBluePale = FLinearColor(0.702f, 0.898f, 0.988f, 1.0f);

	// ── Background Colors ───────────────────────────────────────────────

	/** Main background - deep blue-black (#0A0E1A) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Background")
	FLinearColor Background = FLinearColor(0.039f, 0.055f, 0.102f, 1.0f);

	/** Panel background (#0F1629) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Background")
	FLinearColor BackgroundPanel = FLinearColor(0.059f, 0.086f, 0.161f, 0.9f);

	/** Card background (#141B2D) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Background")
	FLinearColor BackgroundCard = FLinearColor(0.078f, 0.106f, 0.176f, 0.8f);

	/** Elevated surface (#1A2340) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Background")
	FLinearColor BackgroundElevated = FLinearColor(0.102f, 0.137f, 0.251f, 0.95f);

	/** Overlay with 85% opacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Background")
	FLinearColor BackgroundOverlay = FLinearColor(0.039f, 0.055f, 0.102f, 0.85f);

	/** Scrim/dim layer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Background")
	FLinearColor Scrim = FLinearColor(0.0f, 0.0f, 0.0f, 0.5f);

	// ── Text Colors ─────────────────────────────────────────────────────

	/** Primary text - near white (#F0F0F0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Text")
	FLinearColor TextPrimary = FLinearColor(0.941f, 0.941f, 0.941f, 1.0f);

	/** Secondary text - gray (#8899AA) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Text")
	FLinearColor TextSecondary = FLinearColor(0.533f, 0.6f, 0.667f, 1.0f);

	/** Muted text (#556677) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Text")
	FLinearColor TextMuted = FLinearColor(0.333f, 0.4f, 0.467f, 1.0f);

	/** Accent text - cyan (#00D4FF) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Text")
	FLinearColor TextAccent = FLinearColor(0.0f, 0.831f, 1.0f, 1.0f);

	/** Inverse text - dark (#0A0E1A) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Text")
	FLinearColor TextInverse = FLinearColor(0.039f, 0.055f, 0.102f, 1.0f);

	// ── Status Colors ───────────────────────────────────────────────────

	/** Success/positive (#7FFF00) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Status")
	FLinearColor Success = FLinearColor(0.5f, 1.0f, 0.0f, 1.0f);

	/** Warning (#FFB800) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Status")
	FLinearColor Warning = FLinearColor(1.0f, 0.722f, 0.0f, 1.0f);

	/** Danger/error (#FF2244) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Status")
	FLinearColor Danger = FLinearColor(1.0f, 0.133f, 0.267f, 1.0f);

	/** Info (#00A1E0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Status")
	FLinearColor Info = FLinearColor(0.0f, 0.631f, 0.878f, 1.0f);

	/** Success background (15% opacity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Status")
	FLinearColor SuccessBg = FLinearColor(0.5f, 1.0f, 0.0f, 0.15f);

	/** Warning background (15% opacity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Status")
	FLinearColor WarningBg = FLinearColor(1.0f, 0.722f, 0.0f, 0.15f);

	/** Danger background (15% opacity) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Status")
	FLinearColor DangerBg = FLinearColor(1.0f, 0.133f, 0.267f, 0.15f);

	// ── Button Colors ───────────────────────────────────────────────────

	/** Button normal state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Button")
	FLinearColor ButtonNormal = FLinearColor(0.0f, 0.631f, 0.878f, 0.2f);

	/** Button hover state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Button")
	FLinearColor ButtonHover = FLinearColor(0.0f, 0.631f, 0.878f, 0.35f);

	/** Button pressed state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Button")
	FLinearColor ButtonPressed = FLinearColor(0.0f, 0.631f, 0.878f, 0.5f);

	/** Button disabled state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Button")
	FLinearColor ButtonDisabled = FLinearColor(0.333f, 0.4f, 0.467f, 0.2f);

	// ── Vehicle Identity Colors ─────────────────────────────────────────

	/** EP9 vehicle color (#FF4444) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Vehicle")
	FLinearColor VehicleEP9 = FLinearColor(1.0f, 0.267f, 0.267f, 1.0f);

	/** ET7 vehicle color (#4488FF) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Vehicle")
	FLinearColor VehicleET7 = FLinearColor(0.267f, 0.533f, 1.0f, 1.0f);

	/** ES7 vehicle color (#44CC88) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Vehicle")
	FLinearColor VehicleES7 = FLinearColor(0.267f, 0.8f, 0.533f, 1.0f);

	/** ET5 vehicle color (#FFB844) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Vehicle")
	FLinearColor VehicleET5 = FLinearColor(1.0f, 0.722f, 0.267f, 1.0f);

	// ── Race Position Colors ────────────────────────────────────────────

	/** 1st place - Gold (#FFD700) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Race")
	FLinearColor FirstPlace = FLinearColor(1.0f, 0.843f, 0.0f, 1.0f);

	/** 2nd place - Silver (#C0C0C0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Race")
	FLinearColor SecondPlace = FLinearColor(0.753f, 0.753f, 0.753f, 1.0f);

	/** 3rd place - Bronze (#CD7F32) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Race")
	FLinearColor ThirdPlace = FLinearColor(0.804f, 0.498f, 0.196f, 1.0f);

	/** Player indicator (#00D4FF) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Race")
	FLinearColor PlayerColor = FLinearColor(0.0f, 0.831f, 1.0f, 1.0f);

	/** AI opponent indicator (#FF6B6B) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Race")
	FLinearColor AIColor = FLinearColor(1.0f, 0.42f, 0.42f, 1.0f);

	// ── Accessibility Colors ────────────────────────────────────────────

	/** Colorblind-safe success (blue, replaces green) (#0077BB) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Accessibility")
	FLinearColor ColorblindSuccess = FLinearColor(0.0f, 0.467f, 0.733f, 1.0f);

	/** Colorblind-safe danger (orange, replaces red) (#EE7733) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Accessibility")
	FLinearColor ColorblindDanger = FLinearColor(0.933f, 0.467f, 0.2f, 1.0f);

	/** Whether to use colorblind-safe palette */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Accessibility")
	bool bColorblindMode = false;

	/** Whether to use high-contrast mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors|Accessibility")
	bool bHighContrastMode = false;
};

// ─── Font Specification ───────────────────────────────────────────────────

/**
 * Font weight presets
 */
UENUM(BlueprintType)
enum class ENIOFontWeight : uint8
{
	Light       UMETA(DisplayName = "Light (300)"),
	Regular     UMETA(DisplayName = "Regular (400)"),
	Medium      UMETA(DisplayName = "Medium (500)"),
	SemiBold    UMETA(DisplayName = "SemiBold (600)"),
	Bold        UMETA(DisplayName = "Bold (700)")
};

/**
 * Typography scale entry
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOTypeScale
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography")
	int32 SizePx = 18;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography")
	ENIOFontWeight Weight = ENIOFontWeight::Regular;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography")
	float LineHeight = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography")
	float LetterSpacing = 0.0f;
};

/**
 * Complete typography specification
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOTypography
{
	GENERATED_BODY()

	// Typography scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Scale")
	FNIOTypeScale H1; // 96px, Bold - Speed display, countdown

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Scale")
	FNIOTypeScale H2; // 48px, SemiBold - Position, section headers

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Scale")
	FNIOTypeScale H3; // 36px, SemiBold - Track name, lap counter

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Scale")
	FNIOTypeScale H4; // 24px, Medium - Subheadings, menu items

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Scale")
	FNIOTypeScale Body; // 18px, Regular - NOMI comments, descriptions

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Scale")
	FNIOTypeScale BodySmall; // 16px, Regular - Secondary body text

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Scale")
	FNIOTypeScale Small; // 14px, Regular - Labels, helper text

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Scale")
	FNIOTypeScale Caption; // 12px, Light - Captions, fine print

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Scale")
	FNIOTypeScale Overline; // 10px, Medium - Category labels, badges

	// Speed display specific
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Speed")
	int32 SpeedFontSize = 96;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Speed")
	ENIOFontWeight SpeedFontWeight = ENIOFontWeight::Bold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Speed")
	float SpeedLetterSpacing = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Speed")
	int32 SpeedWarningThresholdKmh = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Speed")
	int32 SpeedDangerThresholdKmh = 250;

	// Timer display specific
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Timer")
	int32 TimerFontSize = 28;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography|Timer")
	float TimerLetterSpacing = 1.0f;
};

// ─── Spacing System ───────────────────────────────────────────────────────

/**
 * Spacing system based on 4px grid
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOSpacing
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
	int32 BaseUnit = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
	int32 Xs = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
	int32 Sm = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
	int32 Md = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
	int32 Lg = 24;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
	int32 Xl = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
	int32 Xxl = 48;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
	int32 Xxxl = 64;

	// Component-specific spacing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing|Component")
	int32 ButtonPaddingH = 24;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing|Component")
	int32 ButtonPaddingV = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing|Component")
	int32 CardPadding = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing|Component")
	int32 PanelPadding = 24;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing|Component")
	int32 SectionGap = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing|Component")
	int32 PageMargin = 48;
};

// ─── Border System ────────────────────────────────────────────────────────

/**
 * Border radius presets
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOBorderRadius
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
	int32 None = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
	int32 Sm = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
	int32 Md = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
	int32 Lg = 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
	int32 Xl = 16;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Border")
	int32 Round = 999;
};

// ─── Responsive Layout ────────────────────────────────────────────────────

/**
 * Screen size breakpoint categories
 */
UENUM(BlueprintType)
enum class ENIOBreakpoint : uint8
{
	Mobile      UMETA(DisplayName = "Mobile (< 768px)"),
	Tablet      UMETA(DisplayName = "Tablet (769-1280px)"),
	Desktop     UMETA(DisplayName = "Desktop (1281-1920px)"),
	Ultrawide   UMETA(DisplayName = "Ultrawide (1921-3440px)"),
	FourK       UMETA(DisplayName = "4K (3441px+)")
};

/**
 * Breakpoint configuration
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOBreakpointConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breakpoint")
	int32 MaxWidth = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breakpoint")
	float ScaleFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breakpoint")
	float HUDScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breakpoint")
	float FontScale = 1.0f;
};

/**
 * Safe zone configuration (title-safe / action-safe areas)
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOSafeZone
{
	GENERATED_BODY()

	/** Horizontal inset as fraction of screen width (0.0 - 0.5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafeZone", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float Horizontal = 0.05f;

	/** Vertical inset as fraction of screen height (0.0 - 0.5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafeZone", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float Vertical = 0.05f;
};

/**
 * HUD element anchor positions
 */
UENUM(BlueprintType)
enum class ENIOAnchor : uint8
{
	TopLeft         UMETA(DisplayName = "Top Left"),
	TopCenter       UMETA(DisplayName = "Top Center"),
	TopRight        UMETA(DisplayName = "Top Right"),
	CenterLeft      UMETA(DisplayName = "Center Left"),
	Center          UMETA(DisplayName = "Center"),
	CenterRight     UMETA(DisplayName = "Center Right"),
	BottomLeft      UMETA(DisplayName = "Bottom Left"),
	BottomCenter    UMETA(DisplayName = "Bottom Center"),
	BottomRight     UMETA(DisplayName = "Bottom Right")
};

/**
 * HUD element layout definition
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOHUDElementLayout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	ENIOAnchor Anchor = ENIOAnchor::TopLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float OffsetX = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float OffsetY = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float Width = 0.0f; // 0 = auto

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float Height = 0.0f; // 0 = auto

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	int32 ZOrder = 10;
};

/**
 * Complete responsive layout configuration
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOResponsiveLayout
{
	GENERATED_BODY()

	// Breakpoints
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|Breakpoints")
	FNIOBreakpointConfig Mobile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|Breakpoints")
	FNIOBreakpointConfig Tablet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|Breakpoints")
	FNIOBreakpointConfig Desktop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|Breakpoints")
	FNIOBreakpointConfig Ultrawide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|Breakpoints")
	FNIOBreakpointConfig FourK;

	// Safe zones
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|SafeZone")
	FNIOSafeZone TitleSafe;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|SafeZone")
	FNIOSafeZone ActionSafe;

	/** Minimum margin from screen edges for HUD elements (in pixels) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|SafeZone")
	int32 HUDMarginTop = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|SafeZone")
	int32 HUDMarginRight = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|SafeZone")
	int32 HUDMarginBottom = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|SafeZone")
	int32 HUDMarginLeft = 40;

	// HUD element layouts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|HUD")
	FNIOHUDElementLayout SpeedDisplay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|HUD")
	FNIOHUDElementLayout PositionDisplay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|HUD")
	FNIOHUDElementLayout LapDisplay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|HUD")
	FNIOHUDElementLayout TimerDisplay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|HUD")
	FNIOHUDElementLayout NOMIComment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|HUD")
	FNIOHUDElementLayout Minimap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|HUD")
	FNIOHUDElementLayout ThrottleBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|HUD")
	FNIOHUDElementLayout BrakeBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|HUD")
	FNIOHUDElementLayout BatteryBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|HUD")
	FNIOHUDElementLayout Countdown;

	// Menu layout
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|Menu")
	int32 MenuMaxWidth = 1200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|Menu")
	int32 MenuButtonHeight = 56;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout|Menu")
	int32 MenuButtonSpacing = 12;
};

// ─── Shadow / Glow Config ─────────────────────────────────────────────────

/**
 * Shadow and glow effect configuration
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOEffects
{
	GENERATED_BODY()

	// Elevation shadows
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Shadow")
	float ShadowSmallBlur = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Shadow")
	float ShadowMediumBlur = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Shadow")
	float ShadowLargeBlur = 16.0f;

	// Glow effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Glow")
	float GlowBlueSmall = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Glow")
	float GlowBlueMedium = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Glow")
	float GlowBlueLarge = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Glow")
	float GlowCyanMedium = 10.0f;

	// Background blur
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Blur")
	float BackgroundBlur = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Blur")
	float OverlayBlur = 20.0f;
};

// ─── Complete Theme Assembly ──────────────────────────────────────────────

/**
 * Complete NIO UI Theme combining all design system elements
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNIOUITheme
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FString ThemeName = TEXT("NIO Racing Plus Theme");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FString Version = TEXT("2.0.0");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FNIOColorTheme Colors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FNIOTypography Typography;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FNIOSpacing Spacing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FNIOBorderRadius BorderRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FNIOResponsiveLayout Layout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FNIODurationPresets AnimDurations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FNIOAnimTransition ButtonHoverAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FNIOAnimTransition ButtonPressAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FNIOAnimTransition PanelSlideAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FNIOAnimTransition NOMICommentAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FNIOAnimTransition CountdownAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme")
	FNIOEffects Effects;
};

// ─── Theme Subsystem ──────────────────────────────────────────────────────

/**
 * NIO UI Theme Singleton
 * Provides global access to the complete UI theme configuration
 */
UCLASS(BlueprintType)
class NOMIRACINGPLUS_API UNIOThemeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UNIOThemeSubsystem();

	/** Get the complete UI theme */
	UFUNCTION(BlueprintCallable, Category = "NIO Theme")
	const FNIOUITheme& GetTheme() const { return Theme; }

	/** Set the complete UI theme */
	UFUNCTION(BlueprintCallable, Category = "NIO Theme")
	void SetTheme(const FNIOUITheme& InTheme) { Theme = InTheme; }

	/** Get just the color theme (convenience) */
	UFUNCTION(BlueprintCallable, Category = "NIO Theme")
	const FNIOColorTheme& GetColorTheme() const { return Theme.Colors; }

	/** Get the responsive layout config */
	UFUNCTION(BlueprintCallable, Category = "NIO Theme")
	const FNIOResponsiveLayout& GetLayout() const { return Theme.Layout; }

	/** Get the typography config */
	UFUNCTION(BlueprintCallable, Category = "NIO Theme")
	const FNIOTypography& GetTypography() const { return Theme.Typography; }

	/** Detect current breakpoint based on screen resolution */
	UFUNCTION(BlueprintCallable, Category = "NIO Theme")
	ENIOBreakpoint DetectBreakpoint() const;

	/** Get scale factor for current breakpoint */
	UFUNCTION(BlueprintCallable, Category = "NIO Theme")
	float GetCurrentScaleFactor() const;

	/** Get HUD element position based on anchor and screen size */
	UFUNCTION(BlueprintCallable, Category = "NIO Theme")
	FVector2D CalculateHUDPosition(const FNIOHUDElementLayout& Element, const FVector2D& ScreenSize) const;

	/** Get NIO Blue as hex string */
	UFUNCTION(BlueprintCallable, Category = "NIO Theme")
	FString GetNIOBlueHex() const { return TEXT("#00A1E0"); }

	/** Get NIO Cyan as hex string */
	UFUNCTION(BlueprintCallable, Category = "NIO Theme")
	FString GetNIOCyanHex() const { return TEXT("#00D4FF"); }

	/** Get animation transition config for a named transition */
	UFUNCTION(BlueprintCallable, Category = "NIO Theme")
	FNIOAnimTransition GetTransition(const FString& TransitionName) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Theme")
	FNIOUITheme Theme;
};
