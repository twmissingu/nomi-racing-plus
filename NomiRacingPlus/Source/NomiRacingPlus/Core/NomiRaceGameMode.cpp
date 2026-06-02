// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Core/NomiRaceGameMode.h"
#include "Core/NomiGameInstance.h"
#include "Vehicles/NIOVehicleBase.h"
#include "Vehicles/NIO_EP9.h"
#include "Vehicles/NIO_ET7.h"
#include "Vehicles/NIO_ES7.h"
#include "AI/AICarController.h"
#include "Kismet/GameplayStatics.h"
#include "NomiRacingPlus.h"

ANomiRaceGameMode::ANomiRaceGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set default race config
	DefaultRaceConfig.NumLaps = 3;
	DefaultRaceConfig.MaxAIOpponents = 7;
	DefaultRaceConfig.TrackName = TEXT("NIO City Circuit");
	DefaultRaceConfig.RaceMode = TEXT("StreetGT");
	DefaultRaceConfig.CountdownDuration = 5.0f;
	DefaultRaceConfig.bAllowCollisions = true;

	// Set default AI vehicle types
	AIVehicleTypes.Add(ENIOVehicleType::ET7);
	AIVehicleTypes.Add(ENIOVehicleType::ES7);
	AIVehicleTypes.Add(ENIOVehicleType::EP9);
}

void ANomiRaceGameMode::BeginPlay()
{
	Super::BeginPlay();

	InitializeRaceManager();
	SpawnPlayerVehicle();

	// Create championship manager
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ChampionshipManager = GetWorld()->SpawnActor<AChampionshipManager>(
		AChampionshipManager::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);

	// Get progression component from game instance
	UNomiGameInstance* GameInstance = Cast<UNomiGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		RaceProgression = GameInstance->GetRaceProgression();

		// Wire championship manager to progression component
		if (ChampionshipManager && RaceProgression)
		{
			ChampionshipManager->ProgressionComponent = RaceProgression;
		}
	}

	UE_LOG(LogNomiRacing, Log, TEXT("NomiRaceGameMode initialized"));
}

void ANomiRaceGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANomiRaceGameMode::StartNewRace(const FRaceConfig& Config)
{
	if (!RaceManager)
	{
		UE_LOG(LogNomiRacing, Error, TEXT("Cannot start race: RaceManager not found"));
		return;
	}

	// Spawn AI opponents
	SpawnAIOpponents(Config.MaxAIOpponents);

	// Register player
	APawn* PlayerPawn = GetPlayerVehicle();
	if (PlayerPawn)
	{
		RaceManager->RegisterRacer(PlayerPawn, TEXT("Player"), true);
	}

	// Start the race
	RaceManager->StartRace(Config);

	UE_LOG(LogNomiRacing, Log, TEXT("Race started: %s"), *Config.TrackName);
}

APawn* ANomiRaceGameMode::GetPlayerVehicle() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		return PC->GetPawn();
	}
	return nullptr;
}

void ANomiRaceGameMode::SetPlayerVehicleType(ENIOVehicleType VehicleType)
{
	PlayerVehicleType = VehicleType;
	UE_LOG(LogNomiRacing, Log, TEXT("Player vehicle type set to: %d"), (int32)VehicleType);
}

void ANomiRaceGameMode::SpawnAIOpponents(int32 Count)
{
	if (!RaceManager)
	{
		return;
	}

	// Find spawn points
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsWithTag(this, TEXT("AISpawn"), SpawnPoints);

	if (SpawnPoints.Num() == 0)
	{
		UE_LOG(LogNomiRacing, Warning, TEXT("No AI spawn points found"));
		return;
	}

	// Spawn AI vehicles
	if (AIVehicleTypes.Num() == 0)
	{
		UE_LOG(LogNomiRacing, Warning, TEXT("No AI vehicle types configured"));
		return;
	}

	for (int32 i = 0; i < Count && i < SpawnPoints.Num(); i++)
	{
		ENIOVehicleType VehicleType = AIVehicleTypes[i % AIVehicleTypes.Num()];

		// Determine spawn class based on vehicle type
		TSubclassOf<APawn> SpawnClass;
		switch (VehicleType)
		{
		case ENIOVehicleType::EP9:
			SpawnClass = ANIO_EP9::StaticClass();
			break;
		case ENIOVehicleType::ET7:
			SpawnClass = ANIO_ET7::StaticClass();
			break;
		case ENIOVehicleType::ES7:
			SpawnClass = ANIO_ES7::StaticClass();
			break;
		default:
			SpawnClass = ANIO_ET7::StaticClass();
			break;
		}

		// Spawn the vehicle
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* AIVehicle = GetWorld()->SpawnActor<APawn>(
			SpawnClass,
			SpawnPoints[i]->GetActorLocation(),
			SpawnPoints[i]->GetActorRotation(),
			SpawnParams
		);

		if (AIVehicle)
		{
			// Create AI controller
			AAICarController* AIController = GetWorld()->SpawnActor<AAICarController>();
			if (AIController)
			{
				AIController->Possess(AIVehicle);
				AIController->SetDifficulty(AIDifficulty);
			}

			// Register with race manager
			FString DisplayName = FString::Printf(TEXT("AI %d"), i + 1);
			RaceManager->RegisterRacer(AIVehicle, DisplayName, false);

			UE_LOG(LogNomiRacing, Log, TEXT("Spawned AI vehicle: %s"), *DisplayName);
		}
	}
}

void ANomiRaceGameMode::SetAIDifficulty(EAIDifficulty Difficulty)
{
	AIDifficulty = Difficulty;

	// Update all existing AI controllers
	TArray<AActor*> AIVehicles;
	UGameplayStatics::GetAllActorsOfClass(this, APawn::StaticClass(), AIVehicles);

	for (AActor* Actor : AIVehicles)
	{
		if (APawn* Pawn = Cast<APawn>(Actor))
		{
			if (AAICarController* AIController = Cast<AAICarController>(Pawn->GetController()))
			{
				AIController->SetDifficulty(Difficulty);
			}
		}
	}

	UE_LOG(LogNomiRacing, Log, TEXT("AI difficulty set to: %d"), (int32)Difficulty);
}

void ANomiRaceGameMode::InitializeRaceManager()
{
	// Find or create race manager
	RaceManager = Cast<ARaceManager>(UGameplayStatics::GetActorOfClass(this, ARaceManager::StaticClass()));

	if (!RaceManager)
	{
		// Create race manager
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		RaceManager = GetWorld()->SpawnActor<ARaceManager>(
			ARaceManager::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
	}

	if (RaceManager)
	{
		// Bind to race events
		RaceManager->OnRaceEvent.AddDynamic(this, &ANomiRaceGameMode::OnRaceEvent);
	}
}

void ANomiRaceGameMode::SpawnPlayerVehicle()
{
	// Find player spawn point
	AActor* SpawnPoint = UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass());

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (SpawnPoint)
	{
		SpawnLocation = SpawnPoint->GetActorLocation();
		SpawnRotation = SpawnPoint->GetActorRotation();
	}

	// Determine spawn class based on vehicle type
	TSubclassOf<APawn> SpawnClass;
	switch (PlayerVehicleType)
	{
	case ENIOVehicleType::EP9:
		SpawnClass = ANIO_EP9::StaticClass();
		break;
	case ENIOVehicleType::ET7:
		SpawnClass = ANIO_ET7::StaticClass();
		break;
	case ENIOVehicleType::ES7:
		SpawnClass = ANIO_ES7::StaticClass();
		break;
	default:
		SpawnClass = ANIO_EP9::StaticClass();
		break;
	}

	// Spawn player vehicle
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* PlayerVehicle = GetWorld()->SpawnActor<APawn>(
		SpawnClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (PlayerVehicle)
	{
		// Possess with player controller
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			PC->Possess(PlayerVehicle);
		}

		UE_LOG(LogNomiRacing, Log, TEXT("Player vehicle spawned: %s"), *PlayerVehicle->GetName());
	}
}

void ANomiRaceGameMode::OnRaceEvent(ERaceEvent Event, const FRacerData& RacerData)
{
	// Handle race events
	switch (Event)
	{
	case ERaceEvent::RaceStart:
		UE_LOG(LogNomiRacing, Log, TEXT("Race started!"));
		break;

	case ERaceEvent::RaceFinish:
		UE_LOG(LogNomiRacing, Log, TEXT("Race finished! Player position: %d"), RacerData.Position);

		// Record session for progression if we have the data
		if (RaceProgression && RaceManager)
		{
			FRaceSessionResult SessionResult;
			SessionResult.FinalPosition = RacerData.Position;
			SessionResult.TotalRacers = RaceManager->GetAllRacers().Num();
			SessionResult.NumLaps = RacerData.LapTimes.Num();
			SessionResult.LapTimes = RacerData.LapTimes;
			SessionResult.BestLapTime = RacerData.BestLapTime;
			SessionResult.TotalRaceTime = RacerData.TotalRaceTime;
			SessionResult.Timestamp = FDateTime::Now();

			// Get track and vehicle info from settings
			UNomiGameInstance* GameInstance = Cast<UNomiGameInstance>(GetGameInstance());
			if (GameInstance)
			{
				SessionResult.TrackName = GameInstance->GetSettings().SelectedTrack;
				SessionResult.VehicleName = UEnum::GetValueAsString(GameInstance->GetSettings().SelectedVehicle);
				SessionResult.Difficulty = GameInstance->GetSettings().Difficulty;
			}

			// Determine if clean race (no collisions recorded during race)
			SessionResult.bCleanRace = (SessionResult.Collisions == 0);

			RaceProgression->RecordRaceSession(SessionResult);

			// If in championship, record championship result
			if (ChampionshipManager && ChampionshipManager->HasActiveChampionship())
			{
				SessionResult.bChampionshipRace = true;

				// Build AI positions map
				TMap<FString, int32> AIPositions;
				for (const FRacerData& Racer : RaceManager->GetAllRacers())
				{
					if (!Racer.bIsPlayer)
					{
						AIPositions.Add(Racer.DisplayName, Racer.Position);
					}
				}

				ChampionshipManager->RecordChampionshipRaceResult(RacerData.Position, AIPositions);
			}
		}
		break;

	case ERaceEvent::LapComplete:
		if (RacerData.bIsPlayer)
		{
			UE_LOG(LogNomiRacing, Log, TEXT("Lap %d completed, time: %.2f"), RacerData.CurrentLap, RacerData.LapTimes.Last());
		}
		break;

	default:
		break;
	}
}

bool ANomiRaceGameMode::StartChampionship(const FString& ChampionshipID)
{
	if (!ChampionshipManager)
	{
		UE_LOG(LogNomiRace, Error, TEXT("Cannot start championship: ChampionshipManager not available"));
		return false;
	}

	return ChampionshipManager->StartChampionshipByID(ChampionshipID);
}

bool ANomiRaceGameMode::IsInChampionship() const
{
	return ChampionshipManager && ChampionshipManager->HasActiveChampionship();
}

bool ANomiRaceGameMode::GetCurrentChampionshipTrack(FString& OutTrackName) const
{
	if (ChampionshipManager && ChampionshipManager->HasActiveChampionship())
	{
		return ChampionshipManager->GetCurrentRaceTrack(OutTrackName);
	}
	return false;
}
