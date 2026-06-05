// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NomiRaceGameMode.h"
#include "NomiGameInstance.h"
#include "NomiPlayerController.h"
#include "Vehicles/NIOVehicleBase.h"
#include "Vehicles/NIO_EP9.h"
#include "Vehicles/NIO_ET7.h"
#include "Vehicles/NIO_ES7.h"
#include "Vehicles/Xiaomi_SU7Ultra.h"
#include "AI/AICarController.h"
#include "GameFramework/PlayerStart.h"
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

	// Create NOMI commentary engine as a component on the game mode
	CommentaryEngine = NewObject<UCommentaryEngine>(this, TEXT("CommentaryEngine"));
	if (CommentaryEngine)
	{
		CommentaryEngine->RegisterComponent();
		CommentaryEngine->LoadCommentPool(TEXT("/Game/NOMI/Comments/DefaultComments"));
		UE_LOG(LogNomiRacing, Log, TEXT("NOMI CommentaryEngine created"));
	}

	// Spawn NOMI controller
	FActorSpawnParameters NOMISpawnParams;
	NOMISpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	NOMIController = GetWorld()->SpawnActor<ANOMIController>(
		ANOMIController::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		NOMISpawnParams
	);

	if (NOMIController)
	{
		UE_LOG(LogNomiRacing, Log, TEXT("NOMI Controller spawned"));
	}

	UE_LOG(LogNomiRacing, Log, TEXT("NomiRaceGameMode initialized"));
}

void ANomiRaceGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Auto-start race after a short delay (simulates countdown)
	if (!bRaceStarted && RaceManager)
	{
		RaceStartDelay -= DeltaTime;
		if (RaceStartDelay <= 0.0f)
		{
			bRaceStarted = true;
			StartNewRace(DefaultRaceConfig);
			UE_LOG(LogNomiRacing, Log, TEXT("Race auto-started!"));
		}
	}
}

void ANomiRaceGameMode::StartNewRace(const FRaceConfig& Config)
{
	if (!RaceManager)
	{
		UE_LOG(LogNomiRacing, Error, TEXT("Cannot start race: RaceManager not found"));
		return;
	}

	// Disable player input during countdown
	if (ANomiPlayerController* PC = Cast<ANomiPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputEnabled(false);
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

		// Determine spawn class based on vehicle type (shared helper)
		TSubclassOf<APawn> SpawnClass = GetVehicleSpawnClass(VehicleType);

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
		RaceManager->OnRaceEvent.AddUObject(this, &ANomiRaceGameMode::OnRaceEvent);
	}
}

TSubclassOf<APawn> ANomiRaceGameMode::GetVehicleSpawnClass(ENIOVehicleType VehicleType) const
{
	switch (VehicleType)
	{
	case ENIOVehicleType::EP9:
		return ANIO_EP9::StaticClass();
	case ENIOVehicleType::ET7:
		return ANIO_ET7::StaticClass();
	case ENIOVehicleType::ES7:
		return ANIO_ES7::StaticClass();
	case ENIOVehicleType::SU7Ultra:
		return AXiaomi_SU7Ultra::StaticClass();
	default:
		return ANIO_EP9::StaticClass();
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
	TSubclassOf<APawn> SpawnClass = GetVehicleSpawnClass(PlayerVehicleType);

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
		// Enable player input when race begins
		if (ANomiPlayerController* PC = Cast<ANomiPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			PC->SetInputEnabled(true);
		}
		// Attach NOMI to the player vehicle
		if (NOMIController)
		{
			APawn* PlayerPawn = GetPlayerVehicle();
			if (PlayerPawn)
			{
				NOMIController->AttachToVehicle(PlayerPawn);
			}
		}
		// Track starting position for achievements
		if (RacerData.bIsPlayer)
		{
			PlayerStartingPosition = RacerData.Position;
			UE_LOG(LogNomiRacing, Verbose, TEXT("Player starting position: %d"), PlayerStartingPosition);
		}
		break;

	case ERaceEvent::RaceFinish:
		UE_LOG(LogNomiRacing, Log, TEXT("Race finished! Player position: %d"), RacerData.Position);

		// Record session for progression if we have the data
		if (RaceProgression && RaceManager)
		{
			FRaceSessionResult SessionResult;
			SessionResult.FinalPosition = RacerData.Position;
			SessionResult.StartingPosition = PlayerStartingPosition;
			SessionResult.TotalRacers = RaceManager->GetAllRacers().Num();
			SessionResult.NumLaps = RacerData.LapTimes.Num();
			SessionResult.LapTimes = RacerData.LapTimes;
			SessionResult.BestLapTime = RacerData.BestLapTime;
			SessionResult.TotalRaceTime = RacerData.TotalRaceTime;
			SessionResult.RaceMode = DefaultRaceConfig.RaceMode;
			SessionResult.Timestamp = FDateTime::Now();

			// Get track and vehicle info from settings
			UNomiGameInstance* GameInstance = Cast<UNomiGameInstance>(GetGameInstance());
			if (GameInstance)
			{
				SessionResult.TrackName = GameInstance->GetSettings().SelectedTrack;
				SessionResult.VehicleName = UEnum::GetValueAsString(GameInstance->GetSettings().SelectedVehicle);
				SessionResult.Difficulty = GameInstance->GetSettings().Difficulty;
			}

			// Get race stats from player vehicle
			if (ANIOVehicleBase* PlayerVehicle = Cast<ANIOVehicleBase>(RacerData.VehiclePawn))
			{
				SessionResult.Collisions = PlayerVehicle->CollisionCount;
				SessionResult.MaxSpeed = PlayerVehicle->MaxSpeedKmh;
				SessionResult.DistanceDriven = PlayerVehicle->DistanceDriven;
				SessionResult.Overtakes = PlayerVehicle->OvertakeCount;
			}

			// Determine if clean race
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

	// Forward event to NOMI commentary engine
	ForwardEventToNOMI(Event, RacerData);
}

void ANomiRaceGameMode::ForwardEventToNOMI(ERaceEvent Event, const FRacerData& RacerData)
{
	if (!CommentaryEngine)
	{
		return;
	}

	// Only forward events that are relevant to NOMI commentary
	switch (Event)
	{
	case ERaceEvent::Overtake:
	case ERaceEvent::Overtaken:
	case ERaceEvent::DriftStart:
	case ERaceEvent::DriftEnd:
	case ERaceEvent::LapComplete:
	case ERaceEvent::FastestLap:
	case ERaceEvent::HighSpeed:
	case ERaceEvent::FirstPlace:
	case ERaceEvent::LastPlace:
	case ERaceEvent::RaceStart:
	case ERaceEvent::RaceFinish:
	case ERaceEvent::Collision:
		break;
	default:
		return;
	}

	// Build comment context from race event data
	FCommentContext Context;
	Context.Event = Event;
	Context.Position = RacerData.Position;
	Context.CurrentLap = RacerData.CurrentLap;
	Context.LapTime = (RacerData.LapTimes.Num() > 0) ? RacerData.LapTimes.Last() : 0.0f;

	if (RacerData.bIsPlayer)
	{
		Context.PlayerName = RacerData.DisplayName;
	}

	// Extract vehicle-specific data if available
	if (ANIOVehicleBase* Vehicle = Cast<ANIOVehicleBase>(RacerData.VehiclePawn))
	{
		Context.Speed = Vehicle->GetVehicleState().SpeedKmh;
		Context.bIsNIOVehicle = Vehicle->IsNIOVehicle();
		Context.NIOVehicleType = Vehicle->GetNIOVehicleType();
	}

	CommentaryEngine->RequestComment(Context);
}

bool ANomiRaceGameMode::StartChampionship(const FString& ChampionshipID)
{
	if (!ChampionshipManager)
	{
		UE_LOG(LogNomiRacing, Error, TEXT("Cannot start championship: ChampionshipManager not available"));
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
