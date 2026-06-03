// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Race/RaceProgressionTypes.h"

/**
 * Utility namespace for serializing progression data to/from JSON files.
 * Separates save/load logic from URaceProgression business logic.
 */
namespace ProgressionSerializer
{
	/**
	 * Save player progression data to a JSON file.
	 * @param SavePath Full file path for the save file
	 * @param Stats Player statistics to save
	 * @param Achievements Achievement map to save
	 * @param Unlockables Unlockable items map to save
	 * @return true if save succeeded
	 */
	NOMIRACINGPLUS_API bool Save(
		const FString& SavePath,
		const FPlayerStatistics& Stats,
		const TMap<EAchievement, FAchievementData>& Achievements,
		const TMap<FString, FUnlockableItem>& Unlockables);

	/**
	 * Load player progression data from a JSON file.
	 * @param SavePath Full file path for the save file
	 * @param Stats [out] Player statistics loaded
	 * @param Achievements [out] Achievement map loaded
	 * @param Unlockables [out] Unlockable items map loaded
	 * @return true if load succeeded and data was valid
	 */
	NOMIRACINGPLUS_API bool Load(
		const FString& SavePath,
		FPlayerStatistics& Stats,
		TMap<EAchievement, FAchievementData>& Achievements,
		TMap<FString, FUnlockableItem>& Unlockables);
};
