// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Race/RaceProgression.h"
#include "Kismet/GameplayStatics.h"
#include "NomiRacingPlus.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

URaceProgression::URaceProgression()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URaceProgression::BeginPlay()
{
	Super::BeginPlay();

	InitializeAchievements();
	InitializeUnlockables();
	LoadProgression();

	UE_LOG(LogNomiRace, Log, TEXT("Race Progression initialized"));
}

void URaceProgression::InitializeSystems()
{
	// Manual initialization for components created outside actor lifecycle
	InitializeAchievements();
	InitializeUnlockables();

	UE_LOG(LogNomiRace, Log, TEXT("Race Progression systems manually initialized: %d achievements, %d unlockables"),
		Achievements.Num(), Unlockables.Num());
}

void URaceProgression::InitializeAchievements()
{
	// Race milestones
	FAchievementData FirstRace;
	FirstRace.Achievement = EAchievement::FirstRace;
	FirstRace.DisplayName = TEXT("First Race");
	FirstRace.Description = TEXT("Complete your first race");
	FirstRace.TargetValue = 1.0f;
	Achievements.Add(EAchievement::FirstRace, FirstRace);

	FAchievementData FirstWin;
	FirstWin.Achievement = EAchievement::FirstWin;
	FirstWin.DisplayName = TEXT("First Win");
	FirstWin.Description = TEXT("Win your first race");
	FirstWin.TargetValue = 1.0f;
	Achievements.Add(EAchievement::FirstWin, FirstWin);

	FAchievementData Champion;
	Champion.Achievement = EAchievement::Champion;
	Champion.DisplayName = TEXT("Champion");
	Champion.Description = TEXT("Win a championship");
	Champion.TargetValue = 1.0f;
	Achievements.Add(EAchievement::Champion, Champion);

	FAchievementData ConsistentRacer;
	ConsistentRacer.Achievement = EAchievement::ConsistentRacer;
	ConsistentRacer.DisplayName = TEXT("Consistent Racer");
	ConsistentRacer.Description = TEXT("Complete 10 races");
	ConsistentRacer.TargetValue = 10.0f;
	Achievements.Add(EAchievement::ConsistentRacer, ConsistentRacer);

	FAchievementData VeteranRacer;
	VeteranRacer.Achievement = EAchievement::VeteranRacer;
	VeteranRacer.DisplayName = TEXT("Veteran Racer");
	VeteranRacer.Description = TEXT("Complete 50 races");
	VeteranRacer.TargetValue = 50.0f;
	Achievements.Add(EAchievement::VeteranRacer, VeteranRacer);

	FAchievementData LegendaryRacer;
	LegendaryRacer.Achievement = EAchievement::LegendaryRacer;
	LegendaryRacer.DisplayName = TEXT("Legendary Racer");
	LegendaryRacer.Description = TEXT("Complete 100 races");
	LegendaryRacer.TargetValue = 100.0f;
	Achievements.Add(EAchievement::LegendaryRacer, LegendaryRacer);

	// Speed achievements
	FAchievementData SpeedDemon;
	SpeedDemon.Achievement = EAchievement::SpeedDemon;
	SpeedDemon.DisplayName = TEXT("Speed Demon");
	SpeedDemon.Description = TEXT("Reach 300 km/h");
	SpeedDemon.TargetValue = 300.0f;
	Achievements.Add(EAchievement::SpeedDemon, SpeedDemon);

	FAchievementData SpeedKing;
	SpeedKing.Achievement = EAchievement::SpeedKing;
	SpeedKing.DisplayName = TEXT("Speed King");
	SpeedKing.Description = TEXT("Reach 350 km/h");
	SpeedKing.TargetValue = 350.0f;
	Achievements.Add(EAchievement::SpeedKing, SpeedKing);

	// Skill achievements
	FAchievementData DriftKing;
	DriftKing.Achievement = EAchievement::DriftKing;
	DriftKing.DisplayName = TEXT("Drift King");
	DriftKing.Description = TEXT("Drift for a total of 60 seconds");
	DriftKing.TargetValue = 60.0f;
	Achievements.Add(EAchievement::DriftKing, DriftKing);

	FAchievementData DriftLegend;
	DriftLegend.Achievement = EAchievement::DriftLegend;
	DriftLegend.DisplayName = TEXT("Drift Legend");
	DriftLegend.Description = TEXT("Drift for a total of 300 seconds");
	DriftLegend.TargetValue = 300.0f;
	Achievements.Add(EAchievement::DriftLegend, DriftLegend);

	FAchievementData PerfectLap;
	PerfectLap.Achievement = EAchievement::PerfectLap;
	PerfectLap.DisplayName = TEXT("Perfect Lap");
	PerfectLap.Description = TEXT("Complete a lap without collisions");
	PerfectLap.TargetValue = 1.0f;
	Achievements.Add(EAchievement::PerfectLap, PerfectLap);

	FAchievementData PerfectRace;
	PerfectRace.Achievement = EAchievement::PerfectRace;
	PerfectRace.DisplayName = TEXT("Perfect Race");
	PerfectRace.Description = TEXT("Win a race without any collisions");
	PerfectRace.TargetValue = 1.0f;
	Achievements.Add(EAchievement::PerfectRace, PerfectRace);

	FAchievementData NoCollision;
	NoCollision.Achievement = EAchievement::NoCollision;
	NoCollision.DisplayName = TEXT("No Collision");
	NoCollision.Description = TEXT("Complete a race without any collisions");
	NoCollision.TargetValue = 1.0f;
	Achievements.Add(EAchievement::NoCollision, NoCollision);

	FAchievementData OvertakeMaster;
	OvertakeMaster.Achievement = EAchievement::OvertakeMaster;
	OvertakeMaster.DisplayName = TEXT("Overtake Master");
	OvertakeMaster.Description = TEXT("Perform 100 overtakes");
	OvertakeMaster.TargetValue = 100.0f;
	Achievements.Add(EAchievement::OvertakeMaster, OvertakeMaster);

	FAchievementData OvertakeGod;
	OvertakeGod.Achievement = EAchievement::OvertakeGod;
	OvertakeGod.DisplayName = TEXT("Overtake God");
	OvertakeGod.Description = TEXT("Perform 500 overtakes");
	OvertakeGod.TargetValue = 500.0f;
	Achievements.Add(EAchievement::OvertakeGod, OvertakeGod);

	// Collection achievements
	FAchievementData NIOFan;
	NIOFan.Achievement = EAchievement::NIOFan;
	NIOFan.DisplayName = TEXT("NIO Fan");
	NIOFan.Description = TEXT("Race with all NIO vehicles");
	NIOFan.TargetValue = 3.0f;
	Achievements.Add(EAchievement::NIOFan, NIOFan);

	FAchievementData AllTracks;
	AllTracks.Achievement = EAchievement::AllTracks;
	AllTracks.DisplayName = TEXT("All Tracks");
	AllTracks.Description = TEXT("Race on all available tracks");
	AllTracks.TargetValue = 5.0f;
	Achievements.Add(EAchievement::AllTracks, AllTracks);

	FAchievementData AllVehicles;
	AllVehicles.Achievement = EAchievement::AllVehicles;
	AllVehicles.DisplayName = TEXT("All Vehicles");
	AllVehicles.Description = TEXT("Win a race with every vehicle");
	AllVehicles.TargetValue = 3.0f;
	Achievements.Add(EAchievement::AllVehicles, AllVehicles);

	// Difficulty achievements
	FAchievementData HardDifficulty;
	HardDifficulty.Achievement = EAchievement::HardDifficulty;
	HardDifficulty.DisplayName = TEXT("Hard Difficulty");
	HardDifficulty.Description = TEXT("Win a race on hard difficulty");
	HardDifficulty.TargetValue = 1.0f;
	Achievements.Add(EAchievement::HardDifficulty, HardDifficulty);

	FAchievementData ExpertDifficulty;
	ExpertDifficulty.Achievement = EAchievement::ExpertDifficulty;
	ExpertDifficulty.DisplayName = TEXT("Expert Difficulty");
	ExpertDifficulty.Description = TEXT("Win a race on expert difficulty");
	ExpertDifficulty.TargetValue = 1.0f;
	Achievements.Add(EAchievement::ExpertDifficulty, ExpertDifficulty);

	// Comeback achievements
	FAchievementData Underdog;
	Underdog.Achievement = EAchievement::Underdog;
	Underdog.DisplayName = TEXT("Underdog");
	Underdog.Description = TEXT("Win from last place");
	Underdog.TargetValue = 1.0f;
	Achievements.Add(EAchievement::Underdog, Underdog);

	FAchievementData Comeback;
	Comeback.Achievement = EAchievement::Comeback;
	Comeback.DisplayName = TEXT("Comeback");
	Comeback.Description = TEXT("Win after being in last place");
	Comeback.TargetValue = 1.0f;
	Achievements.Add(EAchievement::Comeback, Comeback);

	// Endurance achievements
	FAchievementData Endurance;
	Endurance.Achievement = EAchievement::Endurance;
	Endurance.DisplayName = TEXT("Endurance");
	Endurance.Description = TEXT("Drive a total of 100 km");
	Endurance.TargetValue = 100000.0f; // meters
	Achievements.Add(EAchievement::Endurance, Endurance);

	FAchievementData UltraEndurance;
	UltraEndurance.Achievement = EAchievement::UltraEndurance;
	UltraEndurance.DisplayName = TEXT("Ultra Endurance");
	UltraEndurance.Description = TEXT("Drive a total of 500 km");
	UltraEndurance.TargetValue = 500000.0f; // meters
	Achievements.Add(EAchievement::UltraEndurance, UltraEndurance);

	// Championship achievements
	FAchievementData DoubleChampion;
	DoubleChampion.Achievement = EAchievement::DoubleChampion;
	DoubleChampion.DisplayName = TEXT("Double Champion");
	DoubleChampion.Description = TEXT("Win 2 championships");
	DoubleChampion.TargetValue = 2.0f;
	Achievements.Add(EAchievement::DoubleChampion, DoubleChampion);

	FAchievementData TripleChampion;
	TripleChampion.Achievement = EAchievement::TripleChampion;
	TripleChampion.DisplayName = TEXT("Triple Champion");
	TripleChampion.Description = TEXT("Win 3 championships");
	TripleChampion.TargetValue = 3.0f;
	Achievements.Add(EAchievement::TripleChampion, TripleChampion);

	// Track mastery
	FAchievementData CircuitMaster;
	CircuitMaster.Achievement = EAchievement::CircuitMaster;
	CircuitMaster.DisplayName = TEXT("Circuit Master");
	CircuitMaster.Description = TEXT("Win on every circuit track");
	CircuitMaster.TargetValue = 3.0f;
	Achievements.Add(EAchievement::CircuitMaster, CircuitMaster);

	FAchievementData StreetMaster;
	StreetMaster.Achievement = EAchievement::StreetMaster;
	StreetMaster.DisplayName = TEXT("Street Master");
	StreetMaster.Description = TEXT("Win on every street track");
	StreetMaster.TargetValue = 2.0f;
	Achievements.Add(EAchievement::StreetMaster, StreetMaster);

	// Special achievements
	FAchievementData OnePercentClub;
	OnePercentClub.Achievement = EAchievement::OnePercentClub;
	OnePercentClub.DisplayName = TEXT("1% Club");
	OnePercentClub.Description = TEXT("Achieve a lap time under 1 minute");
	OnePercentClub.TargetValue = 60.0f;
	Achievements.Add(EAchievement::OnePercentClub, OnePercentClub);

	FAchievementData NightRider;
	NightRider.Achievement = EAchievement::NightRider;
	NightRider.DisplayName = TEXT("Night Rider");
	NightRider.Description = TEXT("Complete 10 races on night tracks");
	NightRider.TargetValue = 10.0f;
	Achievements.Add(EAchievement::NightRider, NightRider);

	FAchievementData SunsetChaser;
	SunsetChaser.Achievement = EAchievement::SunsetChaser;
	SunsetChaser.DisplayName = TEXT("Sunset Chaser");
	SunsetChaser.Description = TEXT("Complete 10 races on sunset tracks");
	SunsetChaser.TargetValue = 10.0f;
	Achievements.Add(EAchievement::SunsetChaser, SunsetChaser);

	FAchievementData HundredClub;
	HundredClub.Achievement = EAchievement::HundredClub;
	HundredClub.DisplayName = TEXT("100 Club");
	HundredClub.Description = TEXT("Reach 100 km/h in the first second");
	HundredClub.TargetValue = 1.0f;
	Achievements.Add(EAchievement::HundredClub, HundredClub);
}

void URaceProgression::InitializeUnlockables()
{
	// EP9 Paint Jobs
	FUnlockableItem EP9Paint1;
	EP9Paint1.ID = TEXT("EP9_Paint_Silver");
	EP9Paint1.Type = EUnlockableType::PaintJob;
	EP9Paint1.DisplayName = TEXT("Silver EP9");
	EP9Paint1.Description = TEXT("Silver paint job for EP9");
	EP9Paint1.UnlockCondition = TEXT("Win 5 races with EP9");
	EP9Paint1.RequiredWins = 5;
	EP9Paint1.Rarity = 1;
	Unlockables.Add(EP9Paint1.ID, EP9Paint1);

	FUnlockableItem EP9Paint2;
	EP9Paint2.ID = TEXT("EP9_Paint_Gold");
	EP9Paint2.Type = EUnlockableType::PaintJob;
	EP9Paint2.DisplayName = TEXT("Gold EP9");
	EP9Paint2.Description = TEXT("Gold paint job for EP9");
	EP9Paint2.UnlockCondition = TEXT("Win a championship");
	EP9Paint2.bRequiresAchievement = true;
	EP9Paint2.RequiredAchievement = EAchievement::Champion;
	EP9Paint2.Rarity = 3;
	Unlockables.Add(EP9Paint2.ID, EP9Paint2);

	FUnlockableItem EP9Paint3;
	EP9Paint3.ID = TEXT("EP9_Paint_Chrome");
	EP9Paint3.Type = EUnlockableType::PaintJob;
	EP9Paint3.DisplayName = TEXT("Chrome EP9");
	EP9Paint3.Description = TEXT("Chrome paint job for EP9");
	EP9Paint3.UnlockCondition = TEXT("Win 3 championships");
	EP9Paint3.bRequiresAchievement = true;
	EP9Paint3.RequiredAchievement = EAchievement::TripleChampion;
	EP9Paint3.Rarity = 4;
	Unlockables.Add(EP9Paint3.ID, EP9Paint3);

	// ET7 Paint Jobs
	FUnlockableItem ET7Paint1;
	ET7Paint1.ID = TEXT("ET7_Paint_Midnight");
	ET7Paint1.Type = EUnlockableType::PaintJob;
	ET7Paint1.DisplayName = TEXT("Midnight ET7");
	ET7Paint1.Description = TEXT("Midnight blue paint job for ET7");
	ET7Paint1.UnlockCondition = TEXT("Win 10 races");
	ET7Paint1.RequiredWins = 10;
	ET7Paint1.Rarity = 2;
	Unlockables.Add(ET7Paint1.ID, ET7Paint1);

	// ES7 Paint Jobs
	FUnlockableItem ES7Paint1;
	ES7Paint1.ID = TEXT("ES7_Paint_Desert");
	ES7Paint1.Type = EUnlockableType::PaintJob;
	ES7Paint1.DisplayName = TEXT("Desert ES7");
	ES7Paint1.Description = TEXT("Desert camo paint job for ES7");
	ES7Paint1.UnlockCondition = TEXT("Win on Desert Rally");
	ES7Paint1.Rarity = 2;
	Unlockables.Add(ES7Paint1.ID, ES7Paint1);

	// Wheel Designs
	FUnlockableItem Wheel1;
	Wheel1.ID = TEXT("Wheel_Sport");
	Wheel1.Type = EUnlockableType::WheelDesign;
	Wheel1.DisplayName = TEXT("Sport Wheels");
	Wheel1.Description = TEXT("Sport wheel design");
	Wheel1.UnlockCondition = TEXT("Complete 10 races");
	Wheel1.RequiredRaces = 10;
	Wheel1.Rarity = 0;
	Unlockables.Add(Wheel1.ID, Wheel1);

	FUnlockableItem Wheel2;
	Wheel2.ID = TEXT("Wheel_Luxury");
	Wheel2.Type = EUnlockableType::WheelDesign;
	Wheel2.DisplayName = TEXT("Luxury Wheels");
	Wheel2.Description = TEXT("Luxury wheel design");
	Wheel2.UnlockCondition = TEXT("Reach 300 km/h");
	Wheel2.bRequiresAchievement = true;
	Wheel2.RequiredAchievement = EAchievement::SpeedDemon;
	Wheel2.Rarity = 2;
	Unlockables.Add(Wheel2.ID, Wheel2);

	FUnlockableItem Wheel3;
	Wheel3.ID = TEXT("Wheel_Racing");
	Wheel3.Type = EUnlockableType::WheelDesign;
	Wheel3.DisplayName = TEXT("Racing Wheels");
	Wheel3.Description = TEXT("Professional racing wheel design");
	Wheel3.UnlockCondition = TEXT("Win 25 races");
	Wheel3.RequiredWins = 25;
	Wheel3.Rarity = 3;
	Unlockables.Add(Wheel3.ID, Wheel3);

	// NOMI Voices
	FUnlockableItem NOMIVoice1;
	NOMIVoice1.ID = TEXT("NOMI_Voice_Cheerful");
	NOMIVoice1.Type = EUnlockableType::NOMIVoice;
	NOMIVoice1.DisplayName = TEXT("Cheerful NOMI");
	NOMIVoice1.Description = TEXT("Cheerful NOMI voice pack");
	NOMIVoice1.UnlockCondition = TEXT("Drift for 30 seconds total");
	NOMIVoice1.Rarity = 1;
	Unlockables.Add(NOMIVoice1.ID, NOMIVoice1);

	FUnlockableItem NOMIVoice2;
	NOMIVoice2.ID = TEXT("NOMI_Voice_Professional");
	NOMIVoice2.Type = EUnlockableType::NOMIVoice;
	NOMIVoice2.DisplayName = TEXT("Professional NOMI");
	NOMIVoice2.Description = TEXT("Professional NOMI voice pack");
	NOMIVoice2.UnlockCondition = TEXT("Complete 10 races");
	NOMIVoice2.bRequiresAchievement = true;
	NOMIVoice2.RequiredAchievement = EAchievement::ConsistentRacer;
	NOMIVoice2.Rarity = 2;
	Unlockables.Add(NOMIVoice2.ID, NOMIVoice2);

	FUnlockableItem NOMIVoice3;
	NOMIVoice3.ID = TEXT("NOMI_Voice_RacingExpert");
	NOMIVoice3.Type = EUnlockableType::NOMIVoice;
	NOMIVoice3.DisplayName = TEXT("Racing Expert NOMI");
	NOMIVoice3.Description = TEXT("Racing expert NOMI voice pack");
	NOMIVoice3.UnlockCondition = TEXT("Win a championship on hard difficulty");
	NOMIVoice3.Rarity = 4;
	Unlockables.Add(NOMIVoice3.ID, NOMIVoice3);

	// Decals
	FUnlockableItem Decal1;
	Decal1.ID = TEXT("Decal_NIO_Flame");
	Decal1.Type = EUnlockableType::Decal;
	Decal1.DisplayName = TEXT("NIO Flame Decal");
	Decal1.Description = TEXT("Flame decal with NIO branding");
	Decal1.UnlockCondition = TEXT("Win 5 races");
	Decal1.RequiredWins = 5;
	Decal1.Rarity = 1;
	Unlockables.Add(Decal1.ID, Decal1);

	FUnlockableItem Decal2;
	Decal2.ID = TEXT("Decal_Lightning");
	Decal2.Type = EUnlockableType::Decal;
	Decal2.DisplayName = TEXT("Lightning Decal");
	Decal2.Description = TEXT("Electric lightning bolt decal");
	Decal2.UnlockCondition = TEXT("Reach 350 km/h");
	Decal2.bRequiresAchievement = true;
	Decal2.RequiredAchievement = EAchievement::SpeedKing;
	Decal2.Rarity = 3;
	Unlockables.Add(Decal2.ID, Decal2);

	// Championships (unlockable tiers)
	FUnlockableItem ChampPro;
	ChampPro.ID = TEXT("Championship_Pro");
	ChampPro.Type = EUnlockableType::Championship;
	ChampPro.DisplayName = TEXT("Pro Championship");
	ChampPro.Description = TEXT("Unlock the Pro tier championship");
	ChampPro.UnlockCondition = TEXT("Win Beginner and Intermediate championships");
	ChampPro.RequiredChampionshipWins = 2;
	ChampPro.Rarity = 2;
	Unlockables.Add(ChampPro.ID, ChampPro);

	FUnlockableItem ChampElite;
	ChampElite.ID = TEXT("Championship_Elite");
	ChampElite.Type = EUnlockableType::Championship;
	ChampElite.DisplayName = TEXT("Elite Championship");
	ChampElite.Description = TEXT("Unlock the Elite tier championship");
	ChampElite.UnlockCondition = TEXT("Win 4 championships");
	ChampElite.RequiredChampionshipWins = 4;
	ChampElite.Rarity = 3;
	Unlockables.Add(ChampElite.ID, ChampElite);

	FUnlockableItem ChampLegend;
	ChampLegend.ID = TEXT("Championship_Legend");
	ChampLegend.Type = EUnlockableType::Championship;
	ChampLegend.DisplayName = TEXT("Legend Championship");
	ChampLegend.Description = TEXT("Unlock the Legend tier championship");
	ChampLegend.UnlockCondition = TEXT("Win 6 championships");
	ChampLegend.RequiredChampionshipWins = 6;
	ChampLegend.Rarity = 4;
	Unlockables.Add(ChampLegend.ID, ChampLegend);

	// Titles
	FUnlockableItem Title1;
	Title1.ID = TEXT("Title_Rookie");
	Title1.Type = EUnlockableType::Title;
	Title1.DisplayName = TEXT("Rookie");
	Title1.Description = TEXT("Just getting started");
	Title1.UnlockCondition = TEXT("Complete your first race");
	Title1.bRequiresAchievement = true;
	Title1.RequiredAchievement = EAchievement::FirstRace;
	Title1.Rarity = 0;
	Unlockables.Add(Title1.ID, Title1);

	FUnlockableItem Title2;
	Title2.ID = TEXT("Title_Champion");
	Title2.Type = EUnlockableType::Title;
	Title2.DisplayName = TEXT("Champion");
	Title2.Description = TEXT("Championship winner");
	Title2.UnlockCondition = TEXT("Win a championship");
	Title2.bRequiresAchievement = true;
	Title2.RequiredAchievement = EAchievement::Champion;
	Title2.Rarity = 3;
	Unlockables.Add(Title2.ID, Title2);

	FUnlockableItem Title3;
	Title3.ID = TEXT("Title_Legend");
	Title3.Type = EUnlockableType::Title;
	Title3.DisplayName = TEXT("Legend");
	Title3.Description = TEXT("The ultimate racer");
	Title3.UnlockCondition = TEXT("Win 3 championships");
	Title3.bRequiresAchievement = true;
	Title3.RequiredAchievement = EAchievement::TripleChampion;
	Title3.Rarity = 4;
	Unlockables.Add(Title3.ID, Title3);
}

bool URaceProgression::GetAchievement(EAchievement Achievement, FAchievementData& OutData) const
{
	const FAchievementData* Data = Achievements.Find(Achievement);
	if (Data)
	{
		OutData = *Data;
		return true;
	}
	return false;
}

bool URaceProgression::UnlockAchievement(EAchievement Achievement)
{
	FAchievementData* Data = Achievements.Find(Achievement);
	if (!Data)
	{
		return false;
	}

	if (Data->bUnlocked)
	{
		return false; // Already unlocked
	}

	Data->bUnlocked = true;
	Data->Progress = 1.0f;
	Data->UnlockTime = FDateTime::Now();

	OnAchievementUnlocked.Broadcast(*Data);

	UE_LOG(LogNomiRace, Log, TEXT("Achievement unlocked: %s"), *Data->DisplayName);

	SaveProgression();
	return true;
}

void URaceProgression::UpdateAchievementProgress(EAchievement Achievement, float Progress)
{
	FAchievementData* Data = Achievements.Find(Achievement);
	if (!Data || Data->bUnlocked)
	{
		return;
	}

	Data->Progress = FMath::Clamp(Progress / Data->TargetValue, 0.0f, 1.0f);

	if (Data->Progress >= 1.0f)
	{
		UnlockAchievement(Achievement);
	}
}

int32 URaceProgression::GetUnlockedAchievementCount() const
{
	int32 Count = 0;
	for (const auto& Pair : Achievements)
	{
		if (Pair.Value.bUnlocked)
		{
			Count++;
		}
	}
	return Count;
}

void URaceProgression::UpdateStatistics(const FPlayerStatistics& RaceStats)
{
	Statistics.TotalRaces += RaceStats.TotalRaces;
	Statistics.TotalWins += RaceStats.TotalWins;
	Statistics.TotalPodiums += RaceStats.TotalPodiums;
	Statistics.TotalLaps += RaceStats.TotalLaps;
	Statistics.TotalDistance += RaceStats.TotalDistance;
	Statistics.TotalDriftTime += RaceStats.TotalDriftTime;
	Statistics.TotalOvertakes += RaceStats.TotalOvertakes;
	Statistics.TotalPlayTime += RaceStats.TotalPlayTime;

	if (RaceStats.MaxSpeed > Statistics.MaxSpeed)
	{
		Statistics.MaxSpeed = RaceStats.MaxSpeed;
	}

	if (RaceStats.BestLapTime > 0 && (Statistics.BestLapTime < 0 || RaceStats.BestLapTime < Statistics.BestLapTime))
	{
		Statistics.BestLapTime = RaceStats.BestLapTime;
		Statistics.BestLapTrack = RaceStats.BestLapTrack;
		Statistics.BestLapVehicle = RaceStats.BestLapVehicle;
	}

	for (const FString& Track : RaceStats.CompletedTracks)
	{
		Statistics.CompletedTracks.Add(Track);
	}

	for (const FString& Vehicle : RaceStats.UsedVehicles)
	{
		Statistics.UsedVehicles.Add(Vehicle);
	}

	CheckAchievements();
	SaveProgression();
}

void URaceProgression::RecordRaceSession(const FRaceSessionResult& SessionResult)
{
	// Process the session into global statistics
	ProcessSessionToStatistics(SessionResult);

	// Update per-track statistics
	UpdateTrackStatistics(SessionResult);

	// Update per-vehicle statistics
	UpdateVehicleStatistics(SessionResult);

	// Update streaks
	UpdateStreaks(SessionResult);

	// Add to recent sessions
	Statistics.RecentSessions.Add(SessionResult);
	if (Statistics.RecentSessions.Num() > MaxRecentSessions)
	{
		Statistics.RecentSessions.RemoveAt(0);
	}

	// Mark track and vehicle as used
	Statistics.CompletedTracks.Add(SessionResult.TrackName);
	Statistics.UsedVehicles.Add(SessionResult.VehicleName);

	// Check achievements
	CheckAchievements();

	// Check stat-based unlocks
	CheckStatBasedUnlocks();

	// Save
	SaveProgression();

	// Broadcast event
	OnRaceSessionCompleted.Broadcast(SessionResult);

	UE_LOG(LogNomiRace, Log, TEXT("Race session recorded: %s on %s, Position: %d/%d"),
		*SessionResult.VehicleName, *SessionResult.TrackName,
		SessionResult.FinalPosition, SessionResult.TotalRacers);
}

void URaceProgression::ProcessSessionToStatistics(const FRaceSessionResult& SessionResult)
{
	Statistics.TotalRaces++;
	Statistics.TotalLaps += SessionResult.NumLaps;
	Statistics.TotalDistance += SessionResult.DistanceDriven;
	Statistics.TotalDriftTime += SessionResult.DriftTime;
	Statistics.TotalOvertakes += SessionResult.Overtakes;
	Statistics.TotalCollisions += SessionResult.Collisions;

	// Win check (position 1)
	if (SessionResult.FinalPosition == 1)
	{
		Statistics.TotalWins++;
	}

	// Podium check (top 3)
	if (SessionResult.FinalPosition <= 3)
	{
		Statistics.TotalPodiums++;
	}

	// Clean race
	if (SessionResult.bCleanRace)
	{
		Statistics.TotalCleanRaces++;
	}

	// Max speed
	if (SessionResult.MaxSpeed > Statistics.MaxSpeed)
	{
		Statistics.MaxSpeed = SessionResult.MaxSpeed;
	}

	// Best lap time (global)
	if (SessionResult.BestLapTime > 0 &&
		(Statistics.BestLapTime < 0 || SessionResult.BestLapTime < Statistics.BestLapTime))
	{
		Statistics.BestLapTime = SessionResult.BestLapTime;
		Statistics.BestLapTrack = SessionResult.TrackName;
		Statistics.BestLapVehicle = SessionResult.VehicleName;
	}
}

void URaceProgression::UpdateTrackStatistics(const FRaceSessionResult& SessionResult)
{
	FTrackStatistics& TrackStat = Statistics.TrackStats.FindOrAdd(SessionResult.TrackName);
	TrackStat.TrackName = SessionResult.TrackName;
	TrackStat.RacesCompleted++;
	TrackStat.TotalLaps += SessionResult.NumLaps;
	TrackStat.TotalDistance += SessionResult.DistanceDriven;

	if (SessionResult.FinalPosition == 1)
	{
		TrackStat.Wins++;
	}

	// Best position
	if (TrackStat.BestPosition == 0 || SessionResult.FinalPosition < TrackStat.BestPosition)
	{
		TrackStat.BestPosition = SessionResult.FinalPosition;
	}

	// Best lap time on this track
	if (SessionResult.BestLapTime > 0 &&
		(TrackStat.BestLapTime < 0 || SessionResult.BestLapTime < TrackStat.BestLapTime))
	{
		TrackStat.BestLapTime = SessionResult.BestLapTime;
		TrackStat.BestLapVehicle = SessionResult.VehicleName;
	}

	// Best race time on this track
	if (SessionResult.TotalRaceTime > 0 &&
		(TrackStat.BestRaceTime < 0 || SessionResult.TotalRaceTime < TrackStat.BestRaceTime))
	{
		TrackStat.BestRaceTime = SessionResult.TotalRaceTime;
	}
}

void URaceProgression::UpdateVehicleStatistics(const FRaceSessionResult& SessionResult)
{
	FVehicleStatistics& VehicleStat = Statistics.VehicleStats.FindOrAdd(SessionResult.VehicleName);
	VehicleStat.VehicleName = SessionResult.VehicleName;
	VehicleStat.RacesCompleted++;
	VehicleStat.TotalDistance += SessionResult.DistanceDriven;
	VehicleStat.TotalDriftTime += SessionResult.DriftTime;

	if (SessionResult.FinalPosition == 1)
	{
		VehicleStat.Wins++;
	}

	// Max speed with this vehicle
	if (SessionResult.MaxSpeed > VehicleStat.MaxSpeed)
	{
		VehicleStat.MaxSpeed = SessionResult.MaxSpeed;
	}

	// Best lap time with this vehicle
	if (SessionResult.BestLapTime > 0 &&
		(VehicleStat.BestLapTime < 0 || SessionResult.BestLapTime < VehicleStat.BestLapTime))
	{
		VehicleStat.BestLapTime = SessionResult.BestLapTime;
		VehicleStat.BestLapTrack = SessionResult.TrackName;
	}
}

void URaceProgression::UpdateStreaks(const FRaceSessionResult& SessionResult)
{
	if (SessionResult.FinalPosition == 1)
	{
		Statistics.CurrentWinStreak++;
		if (Statistics.CurrentWinStreak > Statistics.BestWinStreak)
		{
			Statistics.BestWinStreak = Statistics.CurrentWinStreak;
		}
	}
	else
	{
		Statistics.CurrentWinStreak = 0;
	}

	if (SessionResult.FinalPosition <= 3)
	{
		Statistics.CurrentPodiumStreak++;
	}
	else
	{
		Statistics.CurrentPodiumStreak = 0;
	}
}

bool URaceProgression::GetTrackStatistics(const FString& TrackName, FTrackStatistics& OutStats) const
{
	const FTrackStatistics* Stats = Statistics.TrackStats.Find(TrackName);
	if (Stats)
	{
		OutStats = *Stats;
		return true;
	}
	return false;
}

bool URaceProgression::GetVehicleStatistics(const FString& VehicleName, FVehicleStatistics& OutStats) const
{
	const FVehicleStatistics* Stats = Statistics.VehicleStats.Find(VehicleName);
	if (Stats)
	{
		OutStats = *Stats;
		return true;
	}
	return false;
}

void URaceProgression::StartChampionship(const FChampionshipData& Championship)
{
	CurrentChampionship = Championship;
	CurrentChampionship.StartTime = FDateTime::Now();
	bHasActiveChampionship = true;

	UE_LOG(LogNomiRace, Log, TEXT("Championship started: %s (Tier: %d)"),
		*Championship.Name, (int32)Championship.Tier);
}

bool URaceProgression::GetCurrentChampionship(FChampionshipData& OutChampionship) const
{
	if (bHasActiveChampionship)
	{
		OutChampionship = CurrentChampionship;
		return true;
	}
	return false;
}

void URaceProgression::UpdateChampionshipResults(int32 PlayerPosition, const TMap<FString, int32>& AIPositions)
{
	if (!bHasActiveChampionship)
	{
		return;
	}

	// Award points
	const int32* Points = CurrentChampionship.PointsPerPosition.Find(PlayerPosition);
	if (Points)
	{
		CurrentChampionship.PlayerPoints += *Points;
	}

	for (const auto& Pair : AIPositions)
	{
		const int32* AIPoints = CurrentChampionship.PointsPerPosition.Find(Pair.Value);
		if (AIPoints)
		{
			CurrentChampionship.AIOpponentPoints.FindOrAdd(Pair.Key) += *AIPoints;
		}
	}

	// Update standings
	bool bPlayerFoundInStandings = false;
	for (FChampionshipStandingEntry& Entry : CurrentChampionship.Standings)
	{
		if (Entry.bIsPlayer)
		{
			Entry.Points = CurrentChampionship.PlayerPoints;
			Entry.Positions.Add(PlayerPosition);
			if (PlayerPosition == 1) Entry.Wins++;
			if (PlayerPosition <= 3) Entry.Podiums++;
			bPlayerFoundInStandings = true;
		}
		else
		{
			const int32* AIPts = CurrentChampionship.AIOpponentPoints.Find(Entry.Name);
			if (AIPts)
			{
				Entry.Points = *AIPts;
			}
			const int32* AIPos = AIPositions.Find(Entry.Name);
			if (AIPos)
			{
				Entry.Positions.Add(*AIPos);
				if (*AIPos == 1) Entry.Wins++;
				if (*AIPos <= 3) Entry.Podiums++;
			}
		}
	}

	// Sort standings by points
	CurrentChampionship.Standings.Sort([](const FChampionshipStandingEntry& A, const FChampionshipStandingEntry& B)
	{
		return A.Points > B.Points;
	});

	// Advance to next race
	CurrentChampionship.CurrentRace++;

	// Check if championship is complete
	if (CurrentChampionship.CurrentRace >= CurrentChampionship.Tracks.Num())
	{
		CurrentChampionship.bComplete = true;
		CurrentChampionship.CompletionTime = FDateTime::Now();

		// Determine player final position
		for (int32 i = 0; i < CurrentChampionship.Standings.Num(); i++)
		{
			if (CurrentChampionship.Standings[i].bIsPlayer)
			{
				CurrentChampionship.FinalPlayerPosition = i + 1;
				break;
			}
		}

		// Check if player won
		CurrentChampionship.bPlayerWon = (CurrentChampionship.FinalPlayerPosition == 1);

		if (CurrentChampionship.bPlayerWon)
		{
			Statistics.ChampionshipWins++;
			UnlockAchievement(EAchievement::Champion);

			// Check for multi-championship achievements
			if (Statistics.ChampionshipWins >= 2)
			{
				UnlockAchievement(EAchievement::DoubleChampion);
			}
			if (Statistics.ChampionshipWins >= 3)
			{
				UnlockAchievement(EAchievement::TripleChampion);
			}
		}

		// Record to history
		FChampionshipHistoryEntry HistoryEntry;
		HistoryEntry.Championship = CurrentChampionship;
		HistoryEntry.bPlayerWon = CurrentChampionship.bPlayerWon;
		HistoryEntry.PlayerPosition = CurrentChampionship.FinalPlayerPosition;
		HistoryEntry.CompletionTime = CurrentChampionship.CompletionTime;
		ChampionshipHistory.Add(HistoryEntry);

		// Broadcast completion event
		OnChampionshipCompleted.Broadcast(CurrentChampionship, CurrentChampionship.bPlayerWon);

		UE_LOG(LogNomiRace, Log, TEXT("Championship completed: %s - Player position: %d, Won: %s"),
			*CurrentChampionship.Name, CurrentChampionship.FinalPlayerPosition,
			CurrentChampionship.bPlayerWon ? TEXT("Yes") : TEXT("No"));

		bHasActiveChampionship = false;
	}

	// Check stat-based unlocks
	CheckStatBasedUnlocks();

	SaveProgression();
}

int32 URaceProgression::GetChampionshipWinCount() const
{
	return Statistics.ChampionshipWins;
}

TArray<FUnlockableItem> URaceProgression::GetUnlockablesByType(EUnlockableType Type) const
{
	TArray<FUnlockableItem> Result;
	for (const auto& Pair : Unlockables)
	{
		if (Pair.Value.Type == Type)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

bool URaceProgression::UnlockItem(const FString& ItemID)
{
	FUnlockableItem* Item = Unlockables.Find(ItemID);
	if (!Item || Item->bUnlocked)
	{
		return false;
	}

	Item->bUnlocked = true;
	OnItemUnlocked.Broadcast(*Item);

	UE_LOG(LogNomiRace, Log, TEXT("Item unlocked: %s"), *Item->DisplayName);

	SaveProgression();
	return true;
}

bool URaceProgression::IsItemUnlocked(const FString& ItemID) const
{
	const FUnlockableItem* Item = Unlockables.Find(ItemID);
	return Item ? Item->bUnlocked : false;
}

float URaceProgression::GetUnlockProgress() const
{
	if (Unlockables.Num() == 0) return 0.0f;

	int32 UnlockedCount = 0;
	for (const auto& Pair : Unlockables)
	{
		if (Pair.Value.bUnlocked)
		{
			UnlockedCount++;
		}
	}
	return static_cast<float>(UnlockedCount) / static_cast<float>(Unlockables.Num());
}

bool URaceProgression::SaveProgression()
{
	FString SavePath = GetSavePath();

	// Create save data JSON using proper JSON serialization
	TSharedPtr<FJsonObject> RootObj = MakeShared<FJsonObject>();

	// Statistics
	TSharedPtr<FJsonObject> StatsObj = MakeShared<FJsonObject>();
	StatsObj->SetNumberField(TEXT("total_races"), Statistics.TotalRaces);
	StatsObj->SetNumberField(TEXT("total_wins"), Statistics.TotalWins);
	StatsObj->SetNumberField(TEXT("total_podiums"), Statistics.TotalPodiums);
	StatsObj->SetNumberField(TEXT("total_laps"), Statistics.TotalLaps);
	StatsObj->SetNumberField(TEXT("total_distance"), Statistics.TotalDistance);
	StatsObj->SetNumberField(TEXT("total_drift_time"), Statistics.TotalDriftTime);
	StatsObj->SetNumberField(TEXT("total_overtakes"), Statistics.TotalOvertakes);
	StatsObj->SetNumberField(TEXT("best_lap_time"), Statistics.BestLapTime);
	StatsObj->SetStringField(TEXT("best_lap_track"), Statistics.BestLapTrack);
	StatsObj->SetStringField(TEXT("best_lap_vehicle"), Statistics.BestLapVehicle);
	StatsObj->SetNumberField(TEXT("max_speed"), Statistics.MaxSpeed);
	StatsObj->SetNumberField(TEXT("championship_wins"), Statistics.ChampionshipWins);
	StatsObj->SetNumberField(TEXT("total_play_time"), Statistics.TotalPlayTime);
	StatsObj->SetNumberField(TEXT("total_clean_races"), Statistics.TotalCleanRaces);
	StatsObj->SetNumberField(TEXT("total_collisions"), Statistics.TotalCollisions);
	StatsObj->SetNumberField(TEXT("current_win_streak"), Statistics.CurrentWinStreak);
	StatsObj->SetNumberField(TEXT("best_win_streak"), Statistics.BestWinStreak);
	StatsObj->SetNumberField(TEXT("current_podium_streak"), Statistics.CurrentPodiumStreak);

	// Completed tracks array
	TArray<TSharedPtr<FJsonValue>> TracksArray;
	for (const FString& Track : Statistics.CompletedTracks)
	{
		TracksArray.Add(MakeShared<FJsonValueString>(Track));
	}
	StatsObj->SetArrayField(TEXT("completed_tracks"), TracksArray);

	// Used vehicles array
	TArray<TSharedPtr<FJsonValue>> VehiclesArray;
	for (const FString& Vehicle : Statistics.UsedVehicles)
	{
		VehiclesArray.Add(MakeShared<FJsonValueString>(Vehicle));
	}
	StatsObj->SetArrayField(TEXT("used_vehicles"), VehiclesArray);

	// Per-track stats
	TSharedPtr<FJsonObject> TrackStatsObj = MakeShared<FJsonObject>();
	for (const auto& Pair : Statistics.TrackStats)
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
	for (const auto& Pair : Statistics.VehicleStats)
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

	// Achievements
	TSharedPtr<FJsonObject> AchievementsObj = MakeShared<FJsonObject>();
	for (const auto& Pair : Achievements)
	{
		TSharedPtr<FJsonObject> AchObj = MakeShared<FJsonObject>();
		AchObj->SetBoolField(TEXT("unlocked"), Pair.Value.bUnlocked);
		AchObj->SetNumberField(TEXT("progress"), Pair.Value.Progress);
		AchievementsObj->SetObjectField(FString::FromInt((int32)Pair.Key), AchObj);
	}
	RootObj->SetObjectField(TEXT("achievements"), AchievementsObj);

	// Unlockables
	TSharedPtr<FJsonObject> UnlockablesObj = MakeShared<FJsonObject>();
	for (const auto& Pair : Unlockables)
	{
		TSharedPtr<FJsonObject> UnlockObj = MakeShared<FJsonObject>();
		UnlockObj->SetBoolField(TEXT("unlocked"), Pair.Value.bUnlocked);
		UnlockablesObj->SetObjectField(Pair.Key, UnlockObj);
	}
	RootObj->SetObjectField(TEXT("unlockables"), UnlockablesObj);

	// Serialize to string
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer);

	return FFileHelper::SaveStringToFile(JsonString, *SavePath);
}

bool URaceProgression::LoadProgression()
{
	FString SavePath = GetSavePath();

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

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		// Load statistics
		const TSharedPtr<FJsonObject>* StatsObj;
		if (JsonObject->TryGetObjectField(TEXT("statistics"), StatsObj))
		{
			(*StatsObj)->TryGetNumberField(TEXT("total_races"), Statistics.TotalRaces);
			(*StatsObj)->TryGetNumberField(TEXT("total_wins"), Statistics.TotalWins);
			(*StatsObj)->TryGetNumberField(TEXT("total_podiums"), Statistics.TotalPodiums);
			(*StatsObj)->TryGetNumberField(TEXT("total_laps"), Statistics.TotalLaps);
			(*StatsObj)->TryGetNumberField(TEXT("total_distance"), Statistics.TotalDistance);
			(*StatsObj)->TryGetNumberField(TEXT("total_drift_time"), Statistics.TotalDriftTime);
			(*StatsObj)->TryGetNumberField(TEXT("total_overtakes"), Statistics.TotalOvertakes);
			(*StatsObj)->TryGetNumberField(TEXT("best_lap_time"), Statistics.BestLapTime);
			(*StatsObj)->TryGetStringField(TEXT("best_lap_track"), Statistics.BestLapTrack);
			(*StatsObj)->TryGetStringField(TEXT("best_lap_vehicle"), Statistics.BestLapVehicle);
			(*StatsObj)->TryGetNumberField(TEXT("max_speed"), Statistics.MaxSpeed);
			(*StatsObj)->TryGetNumberField(TEXT("championship_wins"), Statistics.ChampionshipWins);
			(*StatsObj)->TryGetNumberField(TEXT("total_play_time"), Statistics.TotalPlayTime);
			(*StatsObj)->TryGetNumberField(TEXT("total_clean_races"), Statistics.TotalCleanRaces);
			(*StatsObj)->TryGetNumberField(TEXT("total_collisions"), Statistics.TotalCollisions);
			(*StatsObj)->TryGetNumberField(TEXT("current_win_streak"), Statistics.CurrentWinStreak);
			(*StatsObj)->TryGetNumberField(TEXT("best_win_streak"), Statistics.BestWinStreak);
			(*StatsObj)->TryGetNumberField(TEXT("current_podium_streak"), Statistics.CurrentPodiumStreak);

			// Load completed tracks
			const TArray<TSharedPtr<FJsonValue>>* TracksArray;
			if ((*StatsObj)->TryGetArrayField(TEXT("completed_tracks"), TracksArray))
			{
				for (const auto& Val : *TracksArray)
				{
					FString TrackName;
					if (Val->TryGetString(TrackName))
					{
						Statistics.CompletedTracks.Add(TrackName);
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
						Statistics.UsedVehicles.Add(VehicleName);
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
						Statistics.TrackStats.Add(Pair.Key, TS);
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
						Statistics.VehicleStats.Add(Pair.Key, VS);
					}
				}
			}
		}

		// Load achievements
		const TSharedPtr<FJsonObject>* AchievementsObj;
		if (JsonObject->TryGetObjectField(TEXT("achievements"), AchievementsObj))
		{
			for (const auto& Pair : (*AchievementsObj)->Values)
			{
				const TSharedPtr<FJsonObject>* AchObj;
				if (Pair.Value->TryGetObject(AchObj))
				{
					int32 Key = FCString::Atoi(*Pair.Key);
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

		// Load unlockables
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

		UE_LOG(LogNomiRace, Log, TEXT("Progression loaded successfully"));
		return true;
	}

	return false;
}

void URaceProgression::CheckAchievements()
{
	// First Race
	if (Statistics.TotalRaces >= 1)
	{
		UpdateAchievementProgress(EAchievement::FirstRace, 1.0f);
	}

	// Race count milestones
	UpdateAchievementProgress(EAchievement::ConsistentRacer, Statistics.TotalRaces);
	UpdateAchievementProgress(EAchievement::VeteranRacer, Statistics.TotalRaces);
	UpdateAchievementProgress(EAchievement::LegendaryRacer, Statistics.TotalRaces);

	// Overtake milestones
	UpdateAchievementProgress(EAchievement::OvertakeMaster, Statistics.TotalOvertakes);
	UpdateAchievementProgress(EAchievement::OvertakeGod, Statistics.TotalOvertakes);

	// Speed milestones
	UpdateAchievementProgress(EAchievement::SpeedDemon, Statistics.MaxSpeed);
	UpdateAchievementProgress(EAchievement::SpeedKing, Statistics.MaxSpeed);

	// Drift milestones
	UpdateAchievementProgress(EAchievement::DriftKing, Statistics.TotalDriftTime);
	UpdateAchievementProgress(EAchievement::DriftLegend, Statistics.TotalDriftTime);

	// Endurance milestones
	UpdateAchievementProgress(EAchievement::Endurance, Statistics.TotalDistance);
	UpdateAchievementProgress(EAchievement::UltraEndurance, Statistics.TotalDistance);

	// Championship milestones
	UpdateAchievementProgress(EAchievement::Champion, Statistics.ChampionshipWins);
	UpdateAchievementProgress(EAchievement::DoubleChampion, Statistics.ChampionshipWins);
	UpdateAchievementProgress(EAchievement::TripleChampion, Statistics.ChampionshipWins);

	// NIO Fan - use all vehicles
	int32 NIOVehiclesUsed = 0;
	if (Statistics.UsedVehicles.Contains(TEXT("EP9"))) NIOVehiclesUsed++;
	if (Statistics.UsedVehicles.Contains(TEXT("ET7"))) NIOVehiclesUsed++;
	if (Statistics.UsedVehicles.Contains(TEXT("ES7"))) NIOVehiclesUsed++;
	UpdateAchievementProgress(EAchievement::NIOFan, NIOVehiclesUsed);

	// All Tracks
	UpdateAchievementProgress(EAchievement::AllTracks, Statistics.CompletedTracks.Num());

	// All Vehicles - win with every vehicle
	int32 VehiclesWithWins = 0;
	for (const auto& Pair : Statistics.VehicleStats)
	{
		if (Pair.Value.Wins > 0) VehiclesWithWins++;
	}
	UpdateAchievementProgress(EAchievement::AllVehicles, VehiclesWithWins);

	// Check session-based achievements
	if (Statistics.RecentSessions.Num() > 0)
	{
		const FRaceSessionResult& LastSession = Statistics.RecentSessions.Last();

		// First Win
		if (LastSession.FinalPosition == 1)
		{
			UnlockAchievement(EAchievement::FirstWin);
		}

		// Hard Difficulty win
		if (LastSession.FinalPosition == 1 && LastSession.Difficulty >= 2)
		{
			UnlockAchievement(EAchievement::HardDifficulty);
		}

		// Expert Difficulty win
		if (LastSession.FinalPosition == 1 && LastSession.Difficulty >= 3)
		{
			UnlockAchievement(EAchievement::ExpertDifficulty);
		}

		// No Collision
		if (LastSession.bCleanRace)
		{
			UnlockAchievement(EAchievement::NoCollision);
		}

		// Perfect Race (win + no collisions)
		if (LastSession.FinalPosition == 1 && LastSession.bCleanRace)
		{
			UnlockAchievement(EAchievement::PerfectRace);
		}

		// Underdog (win from last place)
		if (LastSession.FinalPosition == 1 && LastSession.TotalRacers > 1)
		{
			// This would need position tracking during the race, simplified here
			UnlockAchievement(EAchievement::Underdog);
		}

		// 1% Club (lap under 60 seconds)
		if (LastSession.BestLapTime > 0 && LastSession.BestLapTime < 60.0f)
		{
			UnlockAchievement(EAchievement::OnePercentClub);
		}
	}
}

void URaceProgression::CheckStatBasedUnlocks()
{
	for (auto& Pair : Unlockables)
	{
		FUnlockableItem& Item = Pair.Value;
		if (Item.bUnlocked) continue;

		bool bShouldUnlock = false;

		// Achievement-based unlocks
		if (Item.bRequiresAchievement)
		{
			const FAchievementData* AchData = Achievements.Find(Item.RequiredAchievement);
			if (AchData && AchData->bUnlocked)
			{
				bShouldUnlock = true;
			}
		}

		// Win-based unlocks
		if (Item.RequiredWins > 0 && Statistics.TotalWins >= Item.RequiredWins)
		{
			bShouldUnlock = true;
		}

		// Race-count-based unlocks
		if (Item.RequiredRaces > 0 && Statistics.TotalRaces >= Item.RequiredRaces)
		{
			bShouldUnlock = true;
		}

		// Championship-win-based unlocks
		if (Item.RequiredChampionshipWins > 0 && Statistics.ChampionshipWins >= Item.RequiredChampionshipWins)
		{
			bShouldUnlock = true;
		}

		if (bShouldUnlock)
		{
			UnlockItem(Pair.Key);
		}
	}
}

FString URaceProgression::GetSavePath() const
{
	return FPaths::ProjectSavedDir() / TEXT("SaveGames") / TEXT("NomiRacingProgression.json");
}

FString URaceProgression::GetChampionshipHistoryPath() const
{
	return FPaths::ProjectSavedDir() / TEXT("SaveGames") / TEXT("NomiRacingChampionshipHistory.json");
}
