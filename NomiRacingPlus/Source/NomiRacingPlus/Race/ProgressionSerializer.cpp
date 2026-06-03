// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Race/ProgressionSerializer.h"
#include "NomiRacingPlus.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace ProgressionSerializer
{

bool Save(const FString& SavePath,
	const FPlayerStatistics& Stats,
	const TMap<EAchievement, FAchievementData>& Achievements,
	const TMap<FString, FUnlockableItem>& Unlockables)
{
	TSharedPtr<FJsonObject> RootObj = MakeShared<FJsonObject>();

	// ── Statistics ──────────────────────────────────────────────────────
	TSharedPtr<FJsonObject> StatsObj = MakeShared<FJsonObject>();
	StatsObj->SetNumberField(TEXT("total_races"), Stats.TotalRaces);
	StatsObj->SetNumberField(TEXT("total_wins"), Stats.TotalWins);
	StatsObj->SetNumberField(TEXT("total_podiums"), Stats.TotalPodiums);
	StatsObj->SetNumberField(TEXT("total_laps"), Stats.TotalLaps);
	StatsObj->SetNumberField(TEXT("total_distance"), Stats.TotalDistance);
	StatsObj->SetNumberField(TEXT("total_drift_time"), Stats.TotalDriftTime);
	StatsObj->SetNumberField(TEXT("total_overtakes"), Stats.TotalOvertakes);
	StatsObj->SetNumberField(TEXT("best_lap_time"), Stats.BestLapTime);
	StatsObj->SetStringField(TEXT("best_lap_track"), Stats.BestLapTrack);
	StatsObj->SetStringField(TEXT("best_lap_vehicle"), Stats.BestLapVehicle);
	StatsObj->SetNumberField(TEXT("max_speed"), Stats.MaxSpeed);
	StatsObj->SetNumberField(TEXT("championship_wins"), Stats.ChampionshipWins);
	StatsObj->SetNumberField(TEXT("total_play_time"), Stats.TotalPlayTime);
	StatsObj->SetNumberField(TEXT("total_clean_races"), Stats.TotalCleanRaces);
	StatsObj->SetNumberField(TEXT("total_collisions"), Stats.TotalCollisions);
	StatsObj->SetNumberField(TEXT("current_win_streak"), Stats.CurrentWinStreak);
	StatsObj->SetNumberField(TEXT("best_win_streak"), Stats.BestWinStreak);
	StatsObj->SetNumberField(TEXT("current_podium_streak"), Stats.CurrentPodiumStreak);
	StatsObj->SetNumberField(TEXT("best_podium_streak"), Stats.BestPodiumStreak);

	// Completed tracks array
	TArray<TSharedPtr<FJsonValue>> TracksArray;
	for (const FString& Track : Stats.CompletedTracks)
	{
		TracksArray.Add(MakeShared<FJsonValueString>(Track));
	}
	StatsObj->SetArrayField(TEXT("completed_tracks"), TracksArray);

	// Used vehicles array
	TArray<TSharedPtr<FJsonValue>> VehiclesArray;
	for (const FString& Vehicle : Stats.UsedVehicles)
	{
		VehiclesArray.Add(MakeShared<FJsonValueString>(Vehicle));
	}
	StatsObj->SetArrayField(TEXT("used_vehicles"), VehiclesArray);

	// Per-track stats
	TSharedPtr<FJsonObject> TrackStatsObj = MakeShared<FJsonObject>();
	for (const auto& Pair : Stats.TrackStats)
	{
		TSharedPtr<FJsonObject> TS = MakeShared<FJsonObject>();
		TS->SetNumberField(TEXT("races_completed"), Pair.Value.RacesCompleted);
		TS->SetNumberField(TEXT("wins"), Pair.Value.Wins);
		TS->SetNumberField(TEXT("best_lap_time"), Pair.Value.BestLapTime);
		TS->SetStringField(TEXT("best_lap_vehicle"), Pair.Value.BestLapVehicle);
		TS->SetNumberField(TEXT("total_laps"), Pair.Value.TotalLaps);
		TS->SetNumberField(TEXT("best_position"), Pair.Value.BestPosition);
		TS->SetNumberField(TEXT("best_race_time"), Pair.Value.BestRaceTime);
		TS->SetNumberField(TEXT("total_distance"), Pair.Value.TotalDistance);
		TrackStatsObj->SetObjectField(Pair.Key, TS);
	}
	StatsObj->SetObjectField(TEXT("track_stats"), TrackStatsObj);

	// Per-vehicle stats
	TSharedPtr<FJsonObject> VehicleStatsObj = MakeShared<FJsonObject>();
	for (const auto& Pair : Stats.VehicleStats)
	{
		TSharedPtr<FJsonObject> VS = MakeShared<FJsonObject>();
		VS->SetNumberField(TEXT("races_completed"), Pair.Value.RacesCompleted);
		VS->SetNumberField(TEXT("wins"), Pair.Value.Wins);
		VS->SetNumberField(TEXT("best_lap_time"), Pair.Value.BestLapTime);
		VS->SetStringField(TEXT("best_lap_track"), Pair.Value.BestLapTrack);
		VS->SetNumberField(TEXT("total_distance"), Pair.Value.TotalDistance);
		VS->SetNumberField(TEXT("max_speed"), Pair.Value.MaxSpeed);
		VS->SetNumberField(TEXT("total_drift_time"), Pair.Value.TotalDriftTime);
		VehicleStatsObj->SetObjectField(Pair.Key, VS);
	}
	StatsObj->SetObjectField(TEXT("vehicle_stats"), VehicleStatsObj);

	RootObj->SetObjectField(TEXT("statistics"), StatsObj);

	// ── Achievements ────────────────────────────────────────────────────
	TSharedPtr<FJsonObject> AchievementsObj = MakeShared<FJsonObject>();
	for (const auto& Pair : Achievements)
	{
		TSharedPtr<FJsonObject> AchObj = MakeShared<FJsonObject>();
		AchObj->SetBoolField(TEXT("unlocked"), Pair.Value.bUnlocked);
		AchObj->SetNumberField(TEXT("progress"), Pair.Value.Progress);
		// Store as int-keyed for compactness; Load validates range to guard against corruption
		AchievementsObj->SetObjectField(FString::FromInt(static_cast<int32>(Pair.Key)), AchObj);
	}
	RootObj->SetObjectField(TEXT("achievements"), AchievementsObj);

	// ── Unlockables ─────────────────────────────────────────────────────
	TSharedPtr<FJsonObject> UnlockablesObj = MakeShared<FJsonObject>();
	for (const auto& Pair : Unlockables)
	{
		TSharedPtr<FJsonObject> UnlockObj = MakeShared<FJsonObject>();
		UnlockObj->SetBoolField(TEXT("unlocked"), Pair.Value.bUnlocked);
		UnlockablesObj->SetObjectField(Pair.Key, UnlockObj);
	}
	RootObj->SetObjectField(TEXT("unlockables"), UnlockablesObj);

	// ── Serialize ───────────────────────────────────────────────────────
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer);

	return FFileHelper::SaveStringToFile(JsonString, *SavePath,
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

bool Load(const FString& SavePath,
	FPlayerStatistics& Stats,
	TMap<EAchievement, FAchievementData>& Achievements,
	TMap<FString, FUnlockableItem>& Unlockables)
{
	if (!FPaths::FileExists(SavePath))
	{
		return false;
	}

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *SavePath))
	{
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}

	// ── Load statistics ─────────────────────────────────────────────────
	const TSharedPtr<FJsonObject>* StatsObj;
	if (JsonObject->TryGetObjectField(TEXT("statistics"), StatsObj))
	{
		(*StatsObj)->TryGetNumberField(TEXT("total_races"), Stats.TotalRaces);
		(*StatsObj)->TryGetNumberField(TEXT("total_wins"), Stats.TotalWins);
		(*StatsObj)->TryGetNumberField(TEXT("total_podiums"), Stats.TotalPodiums);
		(*StatsObj)->TryGetNumberField(TEXT("total_laps"), Stats.TotalLaps);
		(*StatsObj)->TryGetNumberField(TEXT("total_distance"), Stats.TotalDistance);
		(*StatsObj)->TryGetNumberField(TEXT("total_drift_time"), Stats.TotalDriftTime);
		(*StatsObj)->TryGetNumberField(TEXT("total_overtakes"), Stats.TotalOvertakes);
		(*StatsObj)->TryGetNumberField(TEXT("best_lap_time"), Stats.BestLapTime);
		(*StatsObj)->TryGetStringField(TEXT("best_lap_track"), Stats.BestLapTrack);
		(*StatsObj)->TryGetStringField(TEXT("best_lap_vehicle"), Stats.BestLapVehicle);
		(*StatsObj)->TryGetNumberField(TEXT("max_speed"), Stats.MaxSpeed);
		(*StatsObj)->TryGetNumberField(TEXT("championship_wins"), Stats.ChampionshipWins);
		(*StatsObj)->TryGetNumberField(TEXT("total_play_time"), Stats.TotalPlayTime);
		(*StatsObj)->TryGetNumberField(TEXT("total_clean_races"), Stats.TotalCleanRaces);
		(*StatsObj)->TryGetNumberField(TEXT("total_collisions"), Stats.TotalCollisions);
		(*StatsObj)->TryGetNumberField(TEXT("current_win_streak"), Stats.CurrentWinStreak);
		(*StatsObj)->TryGetNumberField(TEXT("best_win_streak"), Stats.BestWinStreak);
		(*StatsObj)->TryGetNumberField(TEXT("current_podium_streak"), Stats.CurrentPodiumStreak);
		(*StatsObj)->TryGetNumberField(TEXT("best_podium_streak"), Stats.BestPodiumStreak);

		// Load completed tracks
		const TArray<TSharedPtr<FJsonValue>>* TracksArray;
		if ((*StatsObj)->TryGetArrayField(TEXT("completed_tracks"), TracksArray))
		{
			for (const auto& Val : *TracksArray)
			{
				FString TrackName;
				if (Val->TryGetString(TrackName))
				{
					Stats.CompletedTracks.Add(TrackName);
				}
			}
		}

		// Load used vehicles
		const TArray<TSharedPtr<FJsonValue>>* VehiclesArray;
		if ((*StatsObj)->TryGetArrayField(TEXT("used_vehicles"), VehiclesArray))
		{
			for (const auto& Val : *VehiclesArray)
			{
				FString VehicleName;
				if (Val->TryGetString(VehicleName))
				{
					Stats.UsedVehicles.Add(VehicleName);
				}
			}
		}

		// Load per-track stats
		const TSharedPtr<FJsonObject>* TrackStatsObj;
		if ((*StatsObj)->TryGetObjectField(TEXT("track_stats"), TrackStatsObj))
		{
			for (const auto& Pair : (*TrackStatsObj)->Values)
			{
				const TSharedPtr<FJsonObject>* TSObj;
				if (Pair.Value->TryGetObject(TSObj))
				{
					FTrackStatistics TS;
					TS.TrackName = Pair.Key;
					(*TSObj)->TryGetNumberField(TEXT("races_completed"), TS.RacesCompleted);
					(*TSObj)->TryGetNumberField(TEXT("wins"), TS.Wins);
					(*TSObj)->TryGetNumberField(TEXT("best_lap_time"), TS.BestLapTime);
					(*TSObj)->TryGetStringField(TEXT("best_lap_vehicle"), TS.BestLapVehicle);
					(*TSObj)->TryGetNumberField(TEXT("total_laps"), TS.TotalLaps);
					(*TSObj)->TryGetNumberField(TEXT("best_position"), TS.BestPosition);
					(*TSObj)->TryGetNumberField(TEXT("best_race_time"), TS.BestRaceTime);
					(*TSObj)->TryGetNumberField(TEXT("total_distance"), TS.TotalDistance);
					Stats.TrackStats.Add(Pair.Key, TS);
				}
			}
		}

		// Load per-vehicle stats
		const TSharedPtr<FJsonObject>* VehicleStatsObj;
		if ((*StatsObj)->TryGetObjectField(TEXT("vehicle_stats"), VehicleStatsObj))
		{
			for (const auto& Pair : (*VehicleStatsObj)->Values)
			{
				const TSharedPtr<FJsonObject>* VSObj;
				if (Pair.Value->TryGetObject(VSObj))
				{
					FVehicleStatistics VS;
					VS.VehicleName = Pair.Key;
					(*VSObj)->TryGetNumberField(TEXT("races_completed"), VS.RacesCompleted);
					(*VSObj)->TryGetNumberField(TEXT("wins"), VS.Wins);
					(*VSObj)->TryGetNumberField(TEXT("best_lap_time"), VS.BestLapTime);
					(*VSObj)->TryGetStringField(TEXT("best_lap_track"), VS.BestLapTrack);
					(*VSObj)->TryGetNumberField(TEXT("total_distance"), VS.TotalDistance);
					(*VSObj)->TryGetNumberField(TEXT("max_speed"), VS.MaxSpeed);
					(*VSObj)->TryGetNumberField(TEXT("total_drift_time"), VS.TotalDriftTime);
					Stats.VehicleStats.Add(Pair.Key, VS);
				}
			}
		}
	}

	// ── Load achievements ───────────────────────────────────────────────
	const TSharedPtr<FJsonObject>* AchievementsObj;
	if (JsonObject->TryGetObjectField(TEXT("achievements"), AchievementsObj))
	{
		for (const auto& Pair : (*AchievementsObj)->Values)
		{
			const TSharedPtr<FJsonObject>* AchObj;
			if (Pair.Value->TryGetObject(AchObj))
			{
				int32 Key = FCString::Atoi(*Pair.Key);
				static constexpr int32 MaxAchievementKey = static_cast<int32>(EAchievement::HundredClub);
				if (Key < 0 || Key > MaxAchievementKey)
				{
					UE_LOG(LogNomiRace, Warning, TEXT("Skipping achievement with out-of-range key: %d"), Key);
					continue;
				}
				EAchievement AchievementType = static_cast<EAchievement>(Key);
				FAchievementData* Data = Achievements.Find(AchievementType);
				if (Data)
				{
					(*AchObj)->TryGetBoolField(TEXT("unlocked"), Data->bUnlocked);
					(*AchObj)->TryGetNumberField(TEXT("progress"), Data->Progress);
				}
			}
		}
	}

	// ── Load unlockables ────────────────────────────────────────────────
	const TSharedPtr<FJsonObject>* UnlockablesObj;
	if (JsonObject->TryGetObjectField(TEXT("unlockables"), UnlockablesObj))
	{
		for (const auto& Pair : (*UnlockablesObj)->Values)
		{
			const TSharedPtr<FJsonObject>* UnlockObj;
			if (Pair.Value->TryGetObject(UnlockObj))
			{
				FUnlockableItem* Item = Unlockables.Find(Pair.Key);
				if (Item)
				{
					(*UnlockObj)->TryGetBoolField(TEXT("unlocked"), Item->bUnlocked);
				}
			}
		}
	}

	UE_LOG(LogNomiRace, Verbose, TEXT("Progression loaded successfully"));
	return true;
}

} // namespace ProgressionSerializer
