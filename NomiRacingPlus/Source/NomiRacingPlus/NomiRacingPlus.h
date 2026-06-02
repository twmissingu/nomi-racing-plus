// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Log categories
DECLARE_LOG_CATEGORY_EXTERN(LogNomiRacing, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNomiRace, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNomiVehicle, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNomiNOMI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNomiAI, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNomiCamera, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNomiPerf, Log, All);

// Game-wide constants
namespace NomiRacingConstants
{
	// NIO Brand Colors
	constexpr const TCHAR* NIOBlueHex = TEXT("#00A1E0");
	constexpr const TCHAR* NIOCyanHex = TEXT("#00D4FF");
	constexpr const TCHAR* NIOBackgroundHex = TEXT("#0A0E1A");

	// Physics
	constexpr float DefaultGravity = 980.0f; // cm/s^2
	constexpr float AirDensity = 1.225f;     // kg/m^3

	// Race
	constexpr int32 MaxLaps = 20;
	constexpr int32 MaxAIOpponents = 7;
	constexpr float CountdownDuration = 5.0f;

	// NOMI
	constexpr float NOMICommentCooldown = 3.0f;
	constexpr int32 NOMIMaxQueueSize = 2;
	constexpr int32 NOMIRecentCommentsMax = 10;
}
