// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Localization string keys for all UI strings in NIO Racing Plus.
 *
 * Each key is a static constexpr TCHAR* intended to be used with
 * UE5's localization pipeline (NSLOCTEXT / LOCTEXT macros) or as
 * string identifiers for a custom localization system.
 *
 * Usage:
 *   FText::FromString(FLocalizationKeys::MainMenu::Title)
 *   // or with LOCTEXT:
 *   LOCTEXT("MainMenuTitle", "NIO Racing Plus")
 */
struct FLocalizationKeys
{
	// ─── Main Menu ────────────────────────────────────────────────────────

	struct MainMenu
	{
		static constexpr TCHAR* Title       = TEXT("NIO Racing Plus");
		static constexpr TCHAR* QuickRace   = TEXT("Quick Race");
		static constexpr TCHAR* Career      = TEXT("Career");
		static constexpr TCHAR* Championship = TEXT("Championship");
		static constexpr TCHAR* TimeTrial   = TEXT("Time Trial");
		static constexpr TCHAR* Settings    = TEXT("Settings");
		static constexpr TCHAR* Garage      = TEXT("Garage");
		static constexpr TCHAR* Exit        = TEXT("Exit");
		static constexpr TCHAR* Credits     = TEXT("Credits");
		static constexpr TCHAR* Season      = TEXT("Season");
		static constexpr TCHAR* SelectTrack = TEXT("Select Track");
		static constexpr TCHAR* SelectVehicle = TEXT("Select Vehicle");
		static constexpr TCHAR* Back        = TEXT("Back");
		static constexpr TCHAR* Confirm     = TEXT("Confirm");
		static constexpr TCHAR* Cancel      = TEXT("Cancel");
		static constexpr TCHAR* Loading     = TEXT("Loading...");
		static constexpr TCHAR* NoData      = TEXT("No Data");
	};

	// ─── Race HUD ─────────────────────────────────────────────────────────

	struct RaceHUD
	{
		static constexpr TCHAR* Speed       = TEXT("SPEED");
		static constexpr TCHAR* SpeedUnit   = TEXT("km/h");
		static constexpr TCHAR* Lap         = TEXT("LAP");
		static constexpr TCHAR* LapOf       = TEXT("/");
		static constexpr TCHAR* Position    = TEXT("POSITION");
		static constexpr TCHAR* Time        = TEXT("TIME");
		static constexpr TCHAR* BestLap     = TEXT("BEST LAP");
		static constexpr TCHAR* LastLap     = TEXT("LAST LAP");
		static constexpr TCHAR* Gap         = TEXT("GAP");
		static constexpr TCHAR* Ahead       = TEXT("AHEAD");
		static constexpr TCHAR* Behind      = TEXT("BEHIND");
		static constexpr TCHAR* Throttle    = TEXT("THROTTLE");
		static constexpr TCHAR* Brake       = TEXT("BRAKE");
		static constexpr TCHAR* Battery     = TEXT("BATTERY");
		static constexpr TCHAR* DrsAvailable = TEXT("DRS AVAILABLE");
		static constexpr TCHAR* DrsActive  = TEXT("DRS ACTIVE");
		static constexpr TCHAR* Pause       = TEXT("PAUSED");
		static constexpr TCHAR* Resume      = TEXT("RESUME");
		static constexpr TCHAR* Restart     = TEXT("RESTART");
		static constexpr TCHAR* QuitRace    = TEXT("QUIT RACE");
		static constexpr TCHAR* RaceComplete = TEXT("RACE COMPLETE");
		static constexpr TCHAR* YouFinished = TEXT("You finished");
		static constexpr TCHAR* FirstPlace  = TEXT("1st");
		static constexpr TCHAR* SecondPlace = TEXT("2nd");
		static constexpr TCHAR* ThirdPlace  = TEXT("3rd");
		static constexpr TCHAR* Countdown3  = TEXT("3");
		static constexpr TCHAR* Countdown2  = TEXT("2");
		static constexpr TCHAR* Countdown1  = TEXT("1");
		static constexpr TCHAR* CountdownGo = TEXT("GO!");
		static constexpr TCHAR* LapsRemaining = TEXT("Laps Remaining");
		static constexpr TCHAR* Retired     = TEXT("RETIRED");
		static constexpr TCHAR* DNF         = TEXT("DNF");
	};

	// ─── Settings ─────────────────────────────────────────────────────────

	struct Settings
	{
		// General
		static constexpr TCHAR* Title       = TEXT("Settings");
		static constexpr TCHAR* Apply       = TEXT("Apply");
		static constexpr TCHAR* Reset       = TEXT("Reset to Defaults");
		static constexpr TCHAR* SaveAndBack = TEXT("Save & Back");

		// Graphics
		static constexpr TCHAR* Graphics    = TEXT("Graphics");
		static constexpr TCHAR* GraphicsQuality = TEXT("Graphics Quality");
		static constexpr TCHAR* Low         = TEXT("Low");
		static constexpr TCHAR* Medium      = TEXT("Medium");
		static constexpr TCHAR* High        = TEXT("High");
		static constexpr TCHAR* Nanite      = TEXT("Nanite");
		static constexpr TCHAR* NaniteDesc  = TEXT("High-detail geometry (Windows only)");
		static constexpr TCHAR* Lumen       = TEXT("Lumen");
		static constexpr TCHAR* LumenDesc   = TEXT("Global illumination and reflections");
		static constexpr TCHAR* MotionBlur  = TEXT("Motion Blur");
		static constexpr TCHAR* MotionBlurDesc = TEXT("Camera motion blur effect");
		static constexpr TCHAR* Resolution  = TEXT("Resolution");
		static constexpr TCHAR* Fullscreen  = TEXT("Fullscreen");
		static constexpr TCHAR* Windowed    = TEXT("Windowed");
		static constexpr TCHAR* Borderless  = TEXT("Borderless");

		// Audio
		static constexpr TCHAR* Audio       = TEXT("Audio");
		static constexpr TCHAR* MasterVolume = TEXT("Master Volume");
		static constexpr TCHAR* SFXVolume   = TEXT("SFX Volume");
		static constexpr TCHAR* MusicVolume = TEXT("Music Volume");
		static constexpr TCHAR* NOMIFrequency = TEXT("NOMI Commentary Frequency");
		static constexpr TCHAR* NOMIFreqOff = TEXT("Off");
		static constexpr TCHAR* NOMIFreqLow = TEXT("Low");
		static constexpr TCHAR* NOMIFreqMed = TEXT("Medium");
		static constexpr TCHAR* NOMIFreqHigh = TEXT("High");

		// Gameplay
		static constexpr TCHAR* Gameplay    = TEXT("Gameplay");
		static constexpr TCHAR* Difficulty  = TEXT("Difficulty");
		static constexpr TCHAR* Easy        = TEXT("Easy");
		static constexpr TCHAR* Normal      = TEXT("Normal");
		static constexpr TCHAR* Hard        = TEXT("Hard");
		static constexpr TCHAR* NumLaps     = TEXT("Number of Laps");
		static constexpr TCHAR* NumAI       = TEXT("AI Opponents");
		static constexpr TCHAR* GameMode    = TEXT("Game Mode");

		// Controls
		static constexpr TCHAR* Controls    = TEXT("Controls");
		static constexpr TCHAR* SteeringSensitivity = TEXT("Steering Sensitivity");
		static constexpr TCHAR* Vibration   = TEXT("Controller Vibration");
		static constexpr TCHAR* AutoBrake   = TEXT("Auto Brake Assist");
		static constexpr TCHAR* RacingLine  = TEXT("Racing Line Assist");

		// Accessibility
		static constexpr TCHAR* Accessibility = TEXT("Accessibility");
		static constexpr TCHAR* ColorblindMode = TEXT("Colorblind Mode");
		static constexpr TCHAR* HighContrast = TEXT("High Contrast Mode");
		static constexpr TCHAR* Subtitles   = TEXT("Subtitles");
		static constexpr TCHAR* TextSize    = TEXT("Text Size");
	};

	// ─── Tutorial Prompts ─────────────────────────────────────────────────

	struct Tutorial
	{
		static constexpr TCHAR* Welcome     = TEXT("Welcome to NIO Racing Plus!");
		static constexpr TCHAR* WelcomeDesc = TEXT("Learn the basics of racing with NIO electric vehicles.");
		static constexpr TCHAR* Steering    = TEXT("Steering");
		static constexpr TCHAR* SteeringDesc = TEXT("Use left/right to steer your vehicle.");
		static constexpr TCHAR* Accelerate  = TEXT("Accelerate");
		static constexpr TCHAR* AccelerateDesc = TEXT("Hold the throttle to accelerate.");
		static constexpr TCHAR* Brake       = TEXT("Brake");
		static constexpr TCHAR* BrakeDesc   = TEXT("Hold the brake to slow down.");
		static constexpr TCHAR* Drift       = TEXT("Drifting");
		static constexpr TCHAR* DriftDesc   = TEXT("Tap brake while turning to initiate a drift.");
		static constexpr TCHAR* Nitro       = TEXT("Nitro Boost");
		static constexpr TCHAR* NitroDesc   = TEXT("Press nitro for a temporary speed boost.");
		static constexpr TCHAR* DRS         = TEXT("DRS System");
		static constexpr TCHAR* DRSDesc     = TEXT("Activate DRS on straights for reduced drag.");
		static constexpr TCHAR* BatteryMgmt = TEXT("Battery Management");
		static constexpr TCHAR* BatteryMgmtDesc = TEXT("Manage battery usage for optimal performance.");
		static constexpr TCHAR* NOMI        = TEXT("Meet NOMI");
		static constexpr TCHAR* NOMIDesc    = TEXT("Your AI companion provides real-time race commentary.");
		static constexpr TCHAR* Skip        = TEXT("Skip Tutorial");
		static constexpr TCHAR* Next        = TEXT("Next");
		static constexpr TCHAR* Done        = TEXT("Done");
		static constexpr TCHAR* Progress    = TEXT("Press any key to continue...");
	};

	// ─── Error / System Messages ──────────────────────────────────────────

	struct System
	{
		static constexpr TCHAR* SaveSuccess = TEXT("Settings saved successfully.");
		static constexpr TCHAR* SaveFailed  = TEXT("Failed to save settings.");
		static constexpr TCHAR* LoadFailed  = TEXT("Failed to load settings.");
		static constexpr TCHAR* CorruptedSave = TEXT("Save file corrupted. Loading defaults.");
		static constexpr TCHAR* RecoverySuccess = TEXT("Recovered from backup save.");
		static constexpr TCHAR* RecoveryFailed = TEXT("Could not recover save data.");
		static constexpr TCHAR* QuitConfirm = TEXT("Are you sure you want to quit?");
		static constexpr TCHAR* Yes         = TEXT("Yes");
		static constexpr TCHAR* No          = TEXT("No");
		static constexpr TCHAR* OK          = TEXT("OK");
		static constexpr TCHAR* Error       = TEXT("Error");
		static constexpr TCHAR* Warning     = TEXT("Warning");
		static constexpr TCHAR* Info        = TEXT("Info");
	};
};
