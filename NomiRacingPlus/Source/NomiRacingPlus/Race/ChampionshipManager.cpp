// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Race/ChampionshipManager.h"
#include "Race/RaceProgression.h"
#include "Kismet/GameplayStatics.h"
#include "NomiRacingPlus.h"

AChampionshipManager::AChampionshipManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AChampionshipManager::BeginPlay()
{
	Super::BeginPlay();

	InitializeAINames();
	InitializeChampionshipTemplates();

	UE_LOG(LogNomiRace, Log, TEXT("ChampionshipManager initialized with %d championship templates"),
		ChampionshipTemplates.Num());
}

void AChampionshipManager::InitializeAINames()
{
	AINamesPool = {
		TEXT("Max Verstappen"), TEXT("Lewis Hamilton"), TEXT("Charles Leclerc"),
		TEXT("Lando Norris"), TEXT("Carlos Sainz"), TEXT("Oscar Piastri"),
		TEXT("George Russell"), TEXT("Fernando Alonso"), TEXT("Pierre Gasly"),
		TEXT("Esteban Ocon"), TEXT("Alex Albon"), TEXT("Yuki Tsunoda"),
		TEXT("Daniel Ricciardo"), TEXT("Valtteri Bottas"), TEXT("Zhou Guanyu"),
		TEXT("Kevin Magnussen"), TEXT("Nico Hulkenberg"), TEXT("Lance Stroll"),
		TEXT("Sergio Perez"), TEXT("Logan Sargeant"), TEXT("Kimi Antonelli"),
		TEXT("Oliver Bearman"), TEXT("Jack Doohan"), TEXT("Liam Lawson"),
		TEXT("Sebastian Vettel"), TEXT("Kimi Raikkonen"), TEXT("Jenson Button"),
		TEXT("Nico Rosberg"), TEXT("Felipe Massa"), TEXT("Mark Webber"),
		TEXT("Robert Kubica"), TEXT("Romain Grosjean"), TEXT("Pastor Maldonado"),
		TEXT("Rubens Barrichello"), TEXT("David Coulthard"), TEXT("Eddie Irvine"),
		TEXT("Mika Hakkinen"), TEXT("Jean Alesi"), TEXT("Gerhard Berger"),
		TEXT("Nelson Piquet Jr"), TEXT("Timo Glock"), TEXT("Heikki Kovalainen"),
		TEXT("Adrian Sutil"), TEXT("Vitantonio Liuzzi"), TEXT("Sebastien Buemi"),
		TEXT("Jaime Alguersuari"), TEXT("Bruno Senna"), TEXT("Vitaly Petrov"),
		TEXT("Nick Heidfeld"), TEXT("Paul di Resta"), TEXT("Pedro de la Rosa")
	};
}

void AChampionshipManager::InitializeChampionshipTemplates()
{
	// ===== BEGINNER TIER =====

	// NIO Sprint Series
	{
		FChampionshipData Champ;
		Champ.ID = TEXT("Beginner_Sprint");
		Champ.Name = TEXT("NIO Sprint Series");
		Champ.Description = TEXT("A quick 3-race championship for newcomers. Perfect for learning the ropes.");
		Champ.Tier = EChampionshipTier::Beginner;
		Champ.Tracks = { TEXT("NIOCityCircuit"), TEXT("SpeedwayOval"), TEXT("NIOCityCircuit") };
		Champ.PointsPerPosition = {
			{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
			{6, 8}, {7, 6}, {8, 4}
		};
		Champ.NumAIOpponents = 5;
		Champ.Difficulty = 0; // Easy
		ChampionshipTemplates.Add(Champ);
	}

	// City Streets Cup
	{
		FChampionshipData Champ;
		Champ.ID = TEXT("Beginner_CityStreets");
		Champ.Name = TEXT("City Streets Cup");
		Champ.Description = TEXT("Race through the city streets in this 4-race beginner championship.");
		Champ.Tier = EChampionshipTier::Beginner;
		Champ.Tracks = { TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong"), TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong") };
		Champ.PointsPerPosition = {
			{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
			{6, 8}, {7, 6}, {8, 4}
		};
		Champ.NumAIOpponents = 5;
		Champ.Difficulty = 0;
		ChampionshipTemplates.Add(Champ);
	}

	// ===== INTERMEDIATE TIER =====

	// NIO Touring Championship
	{
		FChampionshipData Champ;
		Champ.ID = TEXT("Intermediate_Touring");
		Champ.Name = TEXT("NIO Touring Championship");
		Champ.Description = TEXT("A 5-race championship across varied tracks. Tests consistency and skill.");
		Champ.Tier = EChampionshipTier::Intermediate;
		Champ.Tracks = {
			TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong"), TEXT("SpeedwayOval"),
			TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong")
		};
		Champ.PointsPerPosition = {
			{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
			{6, 8}, {7, 6}, {8, 4}
		};
		Champ.NumAIOpponents = 7;
		Champ.Difficulty = 1; // Normal
		ChampionshipTemplates.Add(Champ);
	}

	// Mountain Challenge
	{
		FChampionshipData Champ;
		Champ.ID = TEXT("Intermediate_Mountain");
		Champ.Name = TEXT("Mountain Challenge");
		Champ.Description = TEXT("Take on the mountain pass and city circuits in this challenging series.");
		Champ.Tier = EChampionshipTier::Intermediate;
		Champ.Tracks = {
			TEXT("MountainPass"), TEXT("NIOCityCircuit"), TEXT("MountainPass"),
			TEXT("ShanghaiPudong"), TEXT("MountainPass")
		};
		Champ.PointsPerPosition = {
			{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
			{6, 8}, {7, 6}, {8, 4}
		};
		Champ.NumAIOpponents = 7;
		Champ.Difficulty = 1;
		ChampionshipTemplates.Add(Champ);
	}

	// ===== PRO TIER =====

	// NIO Grand Prix
	{
		FChampionshipData Champ;
		Champ.ID = TEXT("Pro_GrandPrix");
		Champ.Name = TEXT("NIO Grand Prix");
		Champ.Description = TEXT("The premier NIO racing championship. 7 races across all track types.");
		Champ.Tier = EChampionshipTier::Pro;
		Champ.Tracks = {
			TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong"), TEXT("SpeedwayOval"),
			TEXT("MountainPass"), TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong"),
			TEXT("SpeedwayOval")
		};
		Champ.PointsPerPosition = {
			{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
			{6, 8}, {7, 6}, {8, 4}, {9, 2}, {10, 1}
		};
		Champ.NumAIOpponents = 7;
		Champ.Difficulty = 2; // Hard
		ChampionshipTemplates.Add(Champ);
	}

	// Shanghai Masters
	{
		FChampionshipData Champ;
		Champ.ID = TEXT("Pro_ShanghaiMasters");
		Champ.Name = TEXT("Shanghai Masters");
		Champ.Description = TEXT("Dominate the Shanghai street circuit in this intense 6-race series.");
		Champ.Tier = EChampionshipTier::Pro;
		Champ.Tracks = {
			TEXT("ShanghaiPudong"), TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong"),
			TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong"), TEXT("NIOCityCircuit")
		};
		Champ.PointsPerPosition = {
			{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
			{6, 8}, {7, 6}, {8, 4}, {9, 2}, {10, 1}
		};
		Champ.NumAIOpponents = 7;
		Champ.Difficulty = 2;
		ChampionshipTemplates.Add(Champ);
	}

	// ===== ELITE TIER =====

	// World NIO Championship
	{
		FChampionshipData Champ;
		Champ.ID = TEXT("Elite_WorldNIO");
		Champ.Name = TEXT("World NIO Championship");
		Champ.Description = TEXT("The ultimate NIO championship. 10 races on the most demanding tracks.");
		Champ.Tier = EChampionshipTier::Elite;
		Champ.Tracks = {
			TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong"), TEXT("SpeedwayOval"),
			TEXT("MountainPass"), TEXT("DesertRally"), TEXT("NIOCityCircuit"),
			TEXT("ShanghaiPudong"), TEXT("MountainPass"), TEXT("DesertRally"),
			TEXT("ShanghaiPudong")
		};
		Champ.PointsPerPosition = {
			{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
			{6, 8}, {7, 6}, {8, 4}, {9, 2}, {10, 1}
		};
		Champ.NumAIOpponents = 7;
		Champ.Difficulty = 3; // Expert
		ChampionshipTemplates.Add(Champ);
	}

	// Endurance Masters
	{
		FChampionshipData Champ;
		Champ.ID = TEXT("Elite_Endurance");
		Champ.Name = TEXT("Endurance Masters");
		Champ.Description = TEXT("Long races testing stamina and consistency. 6 races with extra laps.");
		Champ.Tier = EChampionshipTier::Elite;
		Champ.Tracks = {
			TEXT("SpeedwayOval"), TEXT("MountainPass"), TEXT("DesertRally"),
			TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong"), TEXT("SpeedwayOval")
		};
		Champ.PointsPerPosition = {
			{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
			{6, 8}, {7, 6}, {8, 4}, {9, 2}, {10, 1}
		};
		Champ.NumAIOpponents = 7;
		Champ.Difficulty = 3;
		ChampionshipTemplates.Add(Champ);
	}

	// ===== LEGEND TIER =====

	// Legend Grand Slam
	{
		FChampionshipData Champ;
		Champ.ID = TEXT("Legend_GrandSlam");
		Champ.Name = TEXT("Legend Grand Slam");
		Champ.Description = TEXT("Only the best survive. 12 races across every track on expert difficulty.");
		Champ.Tier = EChampionshipTier::Legend;
		Champ.Tracks = {
			TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong"), TEXT("SpeedwayOval"),
			TEXT("MountainPass"), TEXT("DesertRally"), TEXT("NIOCityCircuit"),
			TEXT("ShanghaiPudong"), TEXT("SpeedwayOval"), TEXT("MountainPass"),
			TEXT("DesertRally"), TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong")
		};
		Champ.PointsPerPosition = {
			{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
			{6, 8}, {7, 6}, {8, 4}, {9, 2}, {10, 1}
		};
		Champ.NumAIOpponents = 7;
		Champ.Difficulty = 3;
		ChampionshipTemplates.Add(Champ);
	}

	// NIO Invitational
	{
		FChampionshipData Champ;
		Champ.ID = TEXT("Legend_Invitational");
		Champ.Name = TEXT("NIO Invitational");
		Champ.Description = TEXT("An exclusive invitation-only championship for proven champions.");
		Champ.Tier = EChampionshipTier::Legend;
		Champ.Tracks = {
			TEXT("ShanghaiPudong"), TEXT("MountainPass"), TEXT("DesertRally"),
			TEXT("NIOCityCircuit"), TEXT("ShanghaiPudong"), TEXT("MountainPass"),
			TEXT("DesertRally"), TEXT("NIOCityCircuit")
		};
		Champ.PointsPerPosition = {
			{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
			{6, 8}, {7, 6}, {8, 4}, {9, 2}, {10, 1}
		};
		Champ.NumAIOpponents = 7;
		Champ.Difficulty = 3;
		ChampionshipTemplates.Add(Champ);
	}
}

TArray<FChampionshipData> AChampionshipManager::GetChampionshipsForTier(EChampionshipTier Tier) const
{
	TArray<FChampionshipData> Result;
	for (const FChampionshipData& Champ : ChampionshipTemplates)
	{
		if (Champ.Tier == Tier)
		{
			Result.Add(Champ);
		}
	}
	return Result;
}

TArray<FChampionshipData> AChampionshipManager::GetAllAvailableChampionships() const
{
	// Return all templates (in a real game, filter by unlock status)
	return ChampionshipTemplates;
}

bool AChampionshipManager::StartChampionshipByID(const FString& ChampionshipID)
{
	for (const FChampionshipData& Template : ChampionshipTemplates)
	{
		if (Template.ID == ChampionshipID)
		{
			StartChampionship(Template);
			return true;
		}
	}

	UE_LOG(LogNomiRace, Warning, TEXT("Championship not found: %s"), *ChampionshipID);
	return false;
}

void AChampionshipManager::StartChampionship(const FChampionshipData& Championship)
{
	ActiveChampionship = Championship;
	ActiveChampionship.CurrentRace = 0;
	ActiveChampionship.PlayerPoints = 0;
	ActiveChampionship.AIOpponentPoints.Empty();
	ActiveChampionship.bComplete = false;
	ActiveChampionship.bPlayerWon = false;
	ActiveChampionship.FinalPlayerPosition = 0;
	ActiveChampionship.StartTime = FDateTime::Now();

	// Generate AI opponents
	AIOpponentNames = GenerateAIOpponents(Championship.NumAIOpponents, Championship.Tier);

	// Initialize standings
	InitializeStandings(ActiveChampionship);

	bHasActiveChampionship = true;

	// Notify progression component
	if (ProgressionComponent)
	{
		ProgressionComponent->StartChampionship(ActiveChampionship);
	}

	UE_LOG(LogNomiRace, Log, TEXT("Championship started: %s (%d races, %d AI opponents)"),
		*Championship.Name, Championship.Tracks.Num(), AIOpponentNames.Num());
}

bool AChampionshipManager::GetCurrentChampionship(FChampionshipData& OutChampionship) const
{
	if (bHasActiveChampionship)
	{
		OutChampionship = ActiveChampionship;
		return true;
	}
	return false;
}

bool AChampionshipManager::HasActiveChampionship() const
{
	return bHasActiveChampionship;
}

bool AChampionshipManager::GetCurrentRaceTrack(FString& OutTrackName) const
{
	if (!bHasActiveChampionship)
	{
		return false;
	}

	if (ActiveChampionship.CurrentRace >= 0 && ActiveChampionship.CurrentRace < ActiveChampionship.Tracks.Num())
	{
		OutTrackName = ActiveChampionship.Tracks[ActiveChampionship.CurrentRace];
		return true;
	}
	return false;
}

int32 AChampionshipManager::GetCurrentRaceNumber() const
{
	if (!bHasActiveChampionship) return 0;
	return ActiveChampionship.CurrentRace + 1;
}

int32 AChampionshipManager::GetTotalRacesInChampionship() const
{
	if (!bHasActiveChampionship) return 0;
	return ActiveChampionship.Tracks.Num();
}

void AChampionshipManager::RecordChampionshipRaceResult(int32 PlayerPosition, const TMap<FString, int32>& AIPositions)
{
	if (!bHasActiveChampionship)
	{
		UE_LOG(LogNomiRace, Warning, TEXT("No active championship to record results"));
		return;
	}

	// Award points based on position
	const int32* PlayerPoints = ActiveChampionship.PointsPerPosition.Find(PlayerPosition);
	if (PlayerPoints)
	{
		ActiveChampionship.PlayerPoints += *PlayerPoints;
	}

	// Award points to AI
	for (const auto& Pair : AIPositions)
	{
		const int32* AIPoints = ActiveChampionship.PointsPerPosition.Find(Pair.Value);
		if (AIPoints)
		{
			ActiveChampionship.AIOpponentPoints.FindOrAdd(Pair.Key) += *AIPoints;
		}
	}

	// Update standings
	for (FChampionshipStandingEntry& Entry : ActiveChampionship.Standings)
	{
		if (Entry.bIsPlayer)
		{
			Entry.Points = ActiveChampionship.PlayerPoints;
			Entry.Positions.Add(PlayerPosition);
			if (PlayerPosition == 1) Entry.Wins++;
			if (PlayerPosition <= 3) Entry.Podiums++;
		}
		else
		{
			const int32* AIPts = ActiveChampionship.AIOpponentPoints.Find(Entry.Name);
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

	// Sort standings
	ActiveChampionship.Standings.Sort([](const FChampionshipStandingEntry& A, const FChampionshipStandingEntry& B)
	{
		return A.Points > B.Points;
	});

	// Advance race counter
	ActiveChampionship.CurrentRace++;

	// Check completion
	if (ActiveChampionship.CurrentRace >= ActiveChampionship.Tracks.Num())
	{
		ActiveChampionship.bComplete = true;
		ActiveChampionship.CompletionTime = FDateTime::Now();

		// Find player position in standings
		for (int32 i = 0; i < ActiveChampionship.Standings.Num(); i++)
		{
			if (ActiveChampionship.Standings[i].bIsPlayer)
			{
				ActiveChampionship.FinalPlayerPosition = i + 1;
				break;
			}
		}

		ActiveChampionship.bPlayerWon = (ActiveChampionship.FinalPlayerPosition == 1);

		UE_LOG(LogNomiRace, Log, TEXT("Championship completed: %s - Player: %d place (Won: %s)"),
			*ActiveChampionship.Name, ActiveChampionship.FinalPlayerPosition,
			ActiveChampionship.bPlayerWon ? TEXT("Yes") : TEXT("No"));
	}
	else
	{
		UE_LOG(LogNomiRace, Log, TEXT("Championship race %d/%d completed. Player points: %d"),
			ActiveChampionship.CurrentRace, ActiveChampionship.Tracks.Num(),
			ActiveChampionship.PlayerPoints);
	}

	// Notify progression component with fully-calculated championship state
	// (single source of truth: this class does the calculation; RaceProgression only persists)
	if (ProgressionComponent)
	{
		ProgressionComponent->UpdateChampionshipResults(ActiveChampionship);
	}
}

bool AChampionshipManager::IsChampionshipComplete() const
{
	return bHasActiveChampionship && ActiveChampionship.bComplete;
}

bool AChampionshipManager::GetStandings(TArray<FChampionshipStandingEntry>& OutStandings) const
{
	if (!bHasActiveChampionship)
	{
		return false;
	}

	OutStandings = ActiveChampionship.Standings;
	return true;
}

int32 AChampionshipManager::GetPlayerStandingPosition() const
{
	if (!bHasActiveChampionship) return 0;

	for (int32 i = 0; i < ActiveChampionship.Standings.Num(); i++)
	{
		if (ActiveChampionship.Standings[i].bIsPlayer)
		{
			return i + 1;
		}
	}
	return 0;
}

TArray<FString> AChampionshipManager::GenerateAIOpponents(int32 Count, EChampionshipTier Tier) const
{
	TArray<FString> SelectedNames;
	TArray<FString> AvailableNames = AINamesPool;

	// Shuffle the pool
	for (int32 i = AvailableNames.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		AvailableNames.Swap(i, j);
	}

	// Select the requested number of names
	for (int32 i = 0; i < Count && i < AvailableNames.Num(); i++)
	{
		SelectedNames.Add(AvailableNames[i]);
	}

	return SelectedNames;
}

TMap<int32, int32> AChampionshipManager::GetDefaultPointsTable(EChampionshipTier Tier) const
{
	TMap<int32, int32> PointsTable;

	switch (Tier)
	{
	case EChampionshipTier::Beginner:
	case EChampionshipTier::Intermediate:
		PointsTable = {
			{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
			{6, 8}, {7, 6}, {8, 4}
		};
		break;

	case EChampionshipTier::Pro:
	case EChampionshipTier::Elite:
	case EChampionshipTier::Legend:
		PointsTable = {
			{1, 25}, {2, 18}, {3, 15}, {4, 12}, {5, 10},
			{6, 8}, {7, 6}, {8, 4}, {9, 2}, {10, 1}
		};
		break;
	}

	return PointsTable;
}

int32 AChampionshipManager::GetChampionshipReward(EChampionshipTier Tier, int32 Position) const
{
	// Base rewards by tier
	int32 BaseReward = 0;
	switch (Tier)
	{
	case EChampionshipTier::Beginner:     BaseReward = 100; break;
	case EChampionshipTier::Intermediate: BaseReward = 250; break;
	case EChampionshipTier::Pro:          BaseReward = 500; break;
	case EChampionshipTier::Elite:        BaseReward = 1000; break;
	case EChampionshipTier::Legend:        BaseReward = 2000; break;
	}

	// Position multiplier
	float Multiplier = 1.0f;
	switch (Position)
	{
	case 1:  Multiplier = 3.0f; break;
	case 2:  Multiplier = 2.0f; break;
	case 3:  Multiplier = 1.5f; break;
	default: Multiplier = 1.0f; break;
	}

	return FMath::RoundToInt(BaseReward * Multiplier);
}

const TArray<FChampionshipHistoryEntry>& AChampionshipManager::GetChampionshipHistory() const
{
	if (ProgressionComponent)
	{
		return ProgressionComponent->GetChampionshipHistory();
	}

	// Return empty array if no progression component
	static TArray<FChampionshipHistoryEntry> EmptyHistory;
	return EmptyHistory;
}

void AChampionshipManager::InitializeStandings(FChampionshipData& Championship)
{
	Championship.Standings.Empty();

	// Add player entry
	FChampionshipStandingEntry PlayerEntry;
	PlayerEntry.Name = TEXT("Player");
	PlayerEntry.bIsPlayer = true;
	PlayerEntry.Points = 0;
	Championship.Standings.Add(PlayerEntry);

	// Add AI entries
	for (const FString& AIName : AIOpponentNames)
	{
		FChampionshipStandingEntry AIEntry;
		AIEntry.Name = AIName;
		AIEntry.bIsPlayer = false;
		AIEntry.Points = 0;
		Championship.Standings.Add(AIEntry);
	}
}
