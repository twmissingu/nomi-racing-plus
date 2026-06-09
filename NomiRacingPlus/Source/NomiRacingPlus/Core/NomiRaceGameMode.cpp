// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NomiRaceGameMode.h"
#include "NomiGameInstance.h"
#include "NomiPlayerController.h"
#include "Vehicles/NIOVehicleBase.h"
#include "Vehicles/NIO_EP9.h"
#include "Vehicles/NIO_ET7.h"
#include "Vehicles/NIO_ES7.h"
#include "Vehicles/NIO_ET5.h"
#include "Vehicles/Xiaomi_SU7Ultra.h"
#include "AI/AICarController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "NomiRacingPlus.h"
#include "Core/NomiErrorHandler.h"
#include "UI/RaceHUD.h"
#include "UI/ResultsWidget.h"
#include "UI/MenuManager.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	/** Path to the track configuration JSON */
	FString GetTrackConfigPath()
	{
		return FPaths::ProjectContentDir() + TEXT("Maps/TrackConfig.json");
	}

	/** Parse TrackConfig.json and return the display name for a given track ID */
	FString GetTrackDisplayNameFromConfig(const FString& TrackID)
	{
		FString JsonContent;
		if (!FFileHelper::LoadFileToString(JsonContent, *GetTrackConfigPath()))
		{
			return FString();
		}

		TSharedPtr<FJsonObject> RootObj;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
		if (!FJsonSerializer::Deserialize(Reader, RootObj))
		{
			return FString();
		}

		const TSharedPtr<FJsonObject>* TracksObj = nullptr;
		if (!RootObj->TryGetObjectField(TEXT("tracks"), TracksObj))
		{
			return FString();
		}

		const TSharedPtr<FJsonObject>* TrackObj = nullptr;
		if (!(*TracksObj)->TryGetObjectField(TrackID, TrackObj))
		{
			return FString();
		}

		FString DisplayName;
		if ((*TrackObj)->TryGetStringField(TEXT("name"), DisplayName))
		{
			return DisplayName;
		}

		return FString();
	}

	/** Parse TrackConfig.json and return AI spawn points for a given track ID */
	TArray<FTransform> GetAISpawnPointsFromConfig(const FString& TrackID)
	{
		TArray<FTransform> Result;

		FString JsonContent;
		if (!FFileHelper::LoadFileToString(JsonContent, *GetTrackConfigPath()))
		{
			return Result;
		}

		TSharedPtr<FJsonObject> RootObj;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
		if (!FJsonSerializer::Deserialize(Reader, RootObj))
		{
			return Result;
		}

		const TSharedPtr<FJsonObject>* TracksObj = nullptr;
		if (!RootObj->TryGetObjectField(TEXT("tracks"), TracksObj))
		{
			return Result;
		}

		const TSharedPtr<FJsonObject>* TrackObj = nullptr;
		if (!(*TracksObj)->TryGetObjectField(TrackID, TrackObj))
		{
			return Result;
		}

		const TArray<TSharedPtr<FJsonValue>>* SpawnArray = nullptr;
		if (!(*TrackObj)->TryGetArrayField(TEXT("spawn_points"), SpawnArray))
		{
			return Result;
		}

		for (const TSharedPtr<FJsonValue>& Val : *SpawnArray)
		{
			const TSharedPtr<FJsonObject>* PointObj = nullptr;
			if (!Val->TryGetObject(PointObj)) continue;

			double X = 0.0, Y = 0.0, Z = 0.0, Yaw = 0.0;
			(*PointObj)->TryGetNumberField(TEXT("x"), X);
			(*PointObj)->TryGetNumberField(TEXT("y"), Y);
			(*PointObj)->TryGetNumberField(TEXT("z"), Z);
			(*PointObj)->TryGetNumberField(TEXT("yaw"), Yaw);

			FTransform SpawnTransform;
			SpawnTransform.SetLocation(FVector(X, Y, Z));
			SpawnTransform.SetRotation(FRotator(0.0, Yaw, 0.0).Quaternion());
			Result.Add(SpawnTransform);
		}

		return Result;
	}
}

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
	AIVehicleTypes.Add(ENIOVehicleType::ET5);
}

void ANomiRaceGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!GetWorld())
	{
		return;
	}

	// Read persisted settings from GameInstance (survives level transitions)
	UNomiGameInstance* GameInstance = Cast<UNomiGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		const FNomiGameSettings& SavedSettings = GameInstance->GetSettings();
		PlayerVehicleType = SavedSettings.SelectedVehicle;
		DefaultRaceConfig.NumLaps = SavedSettings.NumLaps;
		DefaultRaceConfig.MaxAIOpponents = SavedSettings.NumAIOpponents;
		DefaultRaceConfig.RaceMode = SavedSettings.GameMode;
		DefaultRaceConfig.bIsPointToPoint = (SavedSettings.GameMode == TEXT("Baja"));

		// Resolve track display name from TrackConfig.json using the selected track ID
		FString TrackDisplayName = GetTrackDisplayNameFromConfig(SavedSettings.SelectedTrack);
		if (!TrackDisplayName.IsEmpty())
		{
			DefaultRaceConfig.TrackName = TrackDisplayName;
		}
		else
		{
			// Fallback: use the track ID directly
			DefaultRaceConfig.TrackName = SavedSettings.SelectedTrack;
		}

		UE_LOG(LogNomiRacing, Log, TEXT("Loaded settings from GameInstance: Vehicle=%d, Mode=%s, Laps=%d, AI=%d, Track=%s"),
			static_cast<int32>(PlayerVehicleType), *SavedSettings.GameMode,
			SavedSettings.NumLaps, SavedSettings.NumAIOpponents, *DefaultRaceConfig.TrackName);
	}

	InitializeRaceManager();
	SpawnPlayerVehicle();

	// Cache player vehicle state manager for HUD data
	if (APawn* PlayerPawn = GetPlayerVehicle())
	{
		CachedPlayerVSM = PlayerPawn->FindComponentByClass<UVehicleStateManager>();
	}

	// Create championship manager
	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ChampionshipManager = World->SpawnActor<AChampionshipManager>(
			AChampionshipManager::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);
	}

	// Get progression component from game instance
	UNomiGameInstance* GI = Cast<UNomiGameInstance>(GetGameInstance());
	if (GI)
	{
		RaceProgression = GI->GetRaceProgression();

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
		CommentaryEngine->LoadCommentPool(FPaths::ProjectContentDir() + TEXT("NOMI/Comments/DefaultComments.json"));
		UE_LOG(LogNomiRacing, Log, TEXT("NOMI CommentaryEngine created"));
	}

	// Spawn NOMI controller
	if (UWorld* World = GetWorld())
	{
		FActorSpawnParameters NOMISpawnParams;
		NOMISpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		NOMIController = World->SpawnActor<ANOMIController>(
			ANOMIController::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			NOMISpawnParams
		);
	}

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
	// Skip auto-start on UI-only maps (MainMenu)
	if (!bRaceStarted && RaceManager)
	{
		UWorld* World = GetWorld();
		if (World && World->GetName().Contains(TEXT("MainMenu")))
		{
			return;
		}

		RaceStartDelay -= DeltaTime;
		if (RaceStartDelay <= 0.0f)
		{
			bRaceStarted = true;
			StartNewRace(DefaultRaceConfig);
			UE_LOG(LogNomiRacing, Log, TEXT("Race auto-started!"));
		}
	}

	// Update HUD with live race data
	if (RaceHUDWidget && RaceManager)
	{
		UWorld* World = GetWorld();
		APawn* PlayerPawn = GetPlayerVehicle();
		if (World && PlayerPawn)
		{
			// Re-cache VSM if needed (pawn may have been replaced on rematch)
			if (!CachedPlayerVSM)
			{
				CachedPlayerVSM = PlayerPawn->FindComponentByClass<UVehicleStateManager>();
			}

			FHUDData Data;

			// Vehicle telemetry
			if (CachedPlayerVSM)
			{
				const FNIOVehicleState& VState = CachedPlayerVSM->GetVehicleState();
				Data.Speed = VState.SpeedKmh;
				Data.ThrottleInput = VState.ThrottleInput;
				Data.BrakeInput = VState.BrakeInput;
				Data.SteeringInput = VState.SteeringInput;
				Data.bIsDrifting = VState.bIsDrifting;
				Data.DriftAngle = VState.SlipAngle;
				Data.BatteryLevel = VState.BatteryPercent;
				Data.bIsNIOVehicle = CachedPlayerVSM->IsNIOVehicle();

				// Populate tire temperatures from tire physics model (avoid per-frame TArray allocation)
				if (UNIOVehicleMovementComponent* VehicleMovement = CachedPlayerVSM->GetNIOMovement())
				{
					const FTireEffectsState& Effects = VehicleMovement->GetTireEffectsState();
					Data.ResizeTireTemps();
					for (int32 i = 0; i < FMath::Min(Effects.WheelTemperatures.Num(), FHUDData::TireTempCount); i++)
					{
						Data.TireTemperatures[i] = Effects.WheelTemperatures[i];
					}
					Data.AvgTireTemperature = Effects.AverageTireTemperature;
				}
			}

			// Race telemetry
			ERaceState CurrentState = RaceManager->GetRaceState();
			Data.RaceState = CurrentState;
			Data.Position = RaceManager->GetPlayerPosition();
			Data.TotalRacers = RaceManager->GetAllRacers().Num();
			Data.CurrentLap = RaceManager->GetPlayerCurrentLap();
			Data.TotalLaps = RaceManager->GetRaceConfig().NumLaps;
			Data.RaceTimer = RaceManager->GetRaceTimer();

			// Baja mode fields
			Data.bIsBajaMode = RaceManager->GetRaceConfig().bIsPointToPoint;
			if (Data.bIsBajaMode)
			{
				const TArray<FRacerData>& AllRacers = RaceManager->GetAllRacers();
				for (int32 i = 0; i < AllRacers.Num(); i++)
				{
					if (AllRacers[i].bIsPlayer)
					{
						Data.DistanceToFinish = RaceManager->GetDistanceToFinish(i);
						Data.ProgressPercent = RaceManager->GetProgressPercent(i);
						break;
					}
				}
			}

			// Best lap time from racer data
			FRacerData RacerData;
			if (RaceManager->GetRacerData(PlayerPawn, RacerData))
			{
				Data.BestLapTime = RacerData.BestLapTime;

				// Current lap time = race timer - sum of completed lap times
				float CompletedLapsTime = 0.0f;
				for (float T : RacerData.LapTimes) { CompletedLapsTime += T; }
				Data.CurrentLapTime = RaceManager->GetRaceTimer() - CompletedLapsTime;
			}

			// NOMI comment data
			if (CommentaryEngine)
			{
				Data.bNOMICommentVisible = CommentaryEngine->IsCommentPlaying();
				if (Data.bNOMICommentVisible)
				{
					Data.NOMICommentText = CommentaryEngine->GetCurrentCommentText();
					Data.NOMIEmotion = CommentaryEngine->GetCurrentEmotion();
				}
			}

			// Countdown display
			if (CurrentState == ERaceState::Countdown)
			{
				Data.CountdownValue = RaceManager->GetCountdownValue();
			}

			RaceHUDWidget->UpdateHUDData(Data);
		}
	}
}

void ANomiRaceGameMode::StartNewRace(const FRaceConfig& Config)
{
	if (!RaceManager)
	{
		NomiError::Log(ENomiErrorSeverity::Error, TEXT("Race"), TEXT("Cannot start race: RaceManager not found"));
		return;
	}

	// Disable player input during countdown
	if (ANomiPlayerController* PC = Cast<ANomiPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PC->SetInputEnabled(false);
	}

	// Destroy old AI vehicles before spawning new ones (rematch cleanup)
	// Copy the array since ResetRace() will empty it
	TArray<FRacerData> OldRacers = RaceManager->GetAllRacers();
	for (const FRacerData& Racer : OldRacers)
	{
		if (!Racer.bIsPlayer && Racer.VehiclePawn)
		{
			// Unpossess before destroying
			if (AController* Ctrl = Racer.VehiclePawn->GetController())
			{
				Ctrl->UnPossess();
				Ctrl->Destroy();
			}
			Racer.VehiclePawn->Destroy();
		}
	}

	// Reset RaceManager racers list
	RaceManager->ResetRace();

	// Reset auto-start flag for the new race
	bRaceStarted = true; // Prevent Tick from auto-starting again

	// Spawn AI opponents
	SpawnAIOpponents(Config.MaxAIOpponents);

	// Register player
	APawn* PlayerPawn = GetPlayerVehicle();
	if (PlayerPawn)
	{
		RaceManager->RegisterRacer(PlayerPawn, TEXT("Player"), true);
	}

	// Start the race (enters countdown state)
	RaceManager->StartRace(Config);

	// Create HUD early so countdown can be displayed
	if (ANomiPlayerController* PC = Cast<ANomiPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		if (!RaceHUDWidget)
		{
			RaceHUDWidget = CreateWidget<URaceHUD>(PC, URaceHUD::StaticClass());
			if (RaceHUDWidget)
			{
				RaceHUDWidget->AddToViewport();
				UE_LOG(LogNomiRacing, Log, TEXT("Race HUD created for countdown display"));
			}
		}
	}

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
	UWorld* World = GetWorld();
	if (!RaceManager || !World)
	{
		return;
	}

	// Resolve current track ID from GameInstance settings
	FString CurrentTrackID;
	{
		UNomiGameInstance* GI = Cast<UNomiGameInstance>(GetGameInstance());
		if (GI)
		{
			CurrentTrackID = GI->GetSettings().SelectedTrack;
		}
	}

	// Priority 1: Read spawn points from TrackConfig.json
	TArray<FTransform> ConfigSpawnPoints = GetAISpawnPointsFromConfig(CurrentTrackID);

	// Priority 2: Find level-placed AISpawn actors
	TArray<AActor*> LevelSpawnPoints;
	UGameplayStatics::GetAllActorsWithTag(this, TEXT("AISpawn"), LevelSpawnPoints);

	// Priority 3: Fallback — virtual spawn points offset from player
	TArray<FTransform> VirtualSpawnPoints;
	if (ConfigSpawnPoints.Num() == 0 && LevelSpawnPoints.Num() == 0)
	{
		UE_LOG(LogNomiRacing, Warning, TEXT("No AI spawn points in config or level — using fallback positions behind player"));

		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
		if (PlayerPawn)
		{
			FVector PlayerLoc = PlayerPawn->GetActorLocation();
			FVector PlayerForward = PlayerPawn->GetActorForwardVector();

			for (int32 i = 0; i < Count; i++)
			{
				float OffsetX = -300.0f - (i * 200.0f);
				float OffsetY = (i % 2 == 0 ? -150.0f : 150.0f);
				FVector SpawnLoc = PlayerLoc + PlayerForward * OffsetX + PlayerPawn->GetActorRightVector() * OffsetY;
				SpawnLoc.Z = PlayerLoc.Z;

				FTransform SpawnTransform;
				SpawnTransform.SetLocation(SpawnLoc);
				SpawnTransform.SetRotation(PlayerPawn->GetActorRotation().Quaternion());
				VirtualSpawnPoints.Add(SpawnTransform);
			}
		}
		else
		{
			for (int32 i = 0; i < Count; i++)
			{
				FVector SpawnLoc(-200.0f - (i * 200.0f), (i % 2 == 0 ? -150.0f : 150.0f), 100.0f);
				FTransform SpawnTransform;
				SpawnTransform.SetLocation(SpawnLoc);
				VirtualSpawnPoints.Add(SpawnTransform);
			}
		}
	}

	// Spawn AI vehicles
	if (AIVehicleTypes.Num() == 0)
	{
		UE_LOG(LogNomiRacing, Warning, TEXT("No AI vehicle types configured"));
		return;
	}

	// Determine max spawn count based on available spawn points
	int32 MaxConfig = ConfigSpawnPoints.Num();
	int32 MaxLevel = LevelSpawnPoints.Num();
	int32 MaxVirtual = VirtualSpawnPoints.Num();
	int32 MaxSpawns = Count;

	if (MaxConfig > 0)
	{
		MaxSpawns = FMath::Min(Count, MaxConfig);
	}
	else if (MaxLevel > 0)
	{
		MaxSpawns = FMath::Min(Count, MaxLevel);
	}
	else
	{
		MaxSpawns = FMath::Min(Count, MaxVirtual);
	}

	for (int32 i = 0; i < MaxSpawns; i++)
	{
		ENIOVehicleType VehicleType = AIVehicleTypes[i % AIVehicleTypes.Num()];

		// Determine spawn class based on vehicle type (shared helper)
		TSubclassOf<APawn> SpawnClass = GetVehicleSpawnClass(VehicleType);

		// Get spawn transform — Config > Level > Virtual
		FTransform SpawnTransform;
		if (ConfigSpawnPoints.IsValidIndex(i))
		{
			SpawnTransform = ConfigSpawnPoints[i];
		}
		else if (LevelSpawnPoints.IsValidIndex(i))
		{
			SpawnTransform = LevelSpawnPoints[i]->GetActorTransform();
		}
		else if (VirtualSpawnPoints.IsValidIndex(i))
		{
			SpawnTransform = VirtualSpawnPoints[i];
		}
		else
		{
			continue; // No spawn point available
		}

		// Spawn the vehicle
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* AIVehicle = World->SpawnActor<APawn>(
			SpawnClass,
			SpawnTransform.GetLocation(),
			SpawnTransform.GetRotation().Rotator(),
			SpawnParams
		);

		if (AIVehicle)
		{
			// Create AI controller
			AAICarController* AIController = World->SpawnActor<AAICarController>();
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
		if (UWorld* World = GetWorld())
		{
			// Create race manager
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			RaceManager = World->SpawnActor<ARaceManager>(
				ARaceManager::StaticClass(),
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				SpawnParams
			);
		}
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
	case ENIOVehicleType::ET5:
		return ANIO_ET5::StaticClass();
	case ENIOVehicleType::SU7Ultra:
		return AXiaomi_SU7Ultra::StaticClass();
	default:
		return ANIO_EP9::StaticClass();
	}
}

void ANomiRaceGameMode::SpawnPlayerVehicle()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Find player spawn point
	AActor* SpawnPoint = UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass());

	FVector SpawnLocation = FVector(0.0f, 0.0f, 200.0f);  // Default above ground
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (SpawnPoint)
	{
		SpawnLocation = SpawnPoint->GetActorLocation();
		SpawnRotation = SpawnPoint->GetActorRotation();
	}

	// Ensure vehicle spawns above ground level (trace down to find surface)
	// Use ECC_WorldStatic to avoid hitting dynamic objects (vehicles, characters, etc.)
	// Use MultiTrace + lowest Z to handle multi-level terrain (bridges, overhangs)
	TArray<FHitResult> Hits;
	FVector TraceStart = SpawnLocation + FVector(0.0f, 0.0f, 500.0f);
	FVector TraceEnd = SpawnLocation - FVector(0.0f, 0.0f, 1000.0f);
	if (World->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, ECC_WorldStatic))
	{
		float LowestZ = TraceStart.Z;
		FVector BestImpact = FVector(SpawnLocation.X, SpawnLocation.Y, TraceStart.Z);
		for (const FHitResult& Hit : Hits)
		{
			if (Hit.bBlockingHit && Hit.ImpactPoint.Z < LowestZ)
			{
				LowestZ = Hit.ImpactPoint.Z;
				BestImpact = Hit.ImpactPoint;
			}
		}
		SpawnLocation.Z = BestImpact.Z + 100.0f;  // Spawn above the surface
	}

	// Determine spawn class based on vehicle type
	TSubclassOf<APawn> SpawnClass = GetVehicleSpawnClass(PlayerVehicleType);

	// Spawn player vehicle
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* PlayerVehicle = World->SpawnActor<APawn>(
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

		// Show the HUD (already created during countdown in StartNewRace)
		if (RaceHUDWidget)
		{
			RaceHUDWidget->SetHUDVisible(true);
			RaceHUDWidget->HideCountdown();
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
		{
			// Build session result for progression and results display
			FRaceSessionResult SessionResult;
			SessionResult.FinalPosition = RacerData.Position;
			SessionResult.StartingPosition = PlayerStartingPosition;
			SessionResult.TotalRacers = RaceManager ? RaceManager->GetAllRacers().Num() : 0;
			SessionResult.NumLaps = RacerData.LapTimes.Num();
			SessionResult.LapTimes = RacerData.LapTimes;
			SessionResult.BestLapTime = RacerData.BestLapTime;
			SessionResult.TotalRaceTime = RacerData.TotalRaceTime;
			SessionResult.RaceMode = DefaultRaceConfig.RaceMode;
			SessionResult.Timestamp = FDateTime::Now();

			// Get track and vehicle info from settings
			SessionResult.TrackName = DefaultRaceConfig.TrackName;

			UNomiGameInstance* FinishGI = Cast<UNomiGameInstance>(GetGameInstance());
			if (FinishGI)
			{
				SessionResult.VehicleName = UEnum::GetValueAsString(FinishGI->GetSettings().SelectedVehicle);
				SessionResult.Difficulty = FinishGI->GetSettings().Difficulty;
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

			// Record session for progression
			if (RaceProgression)
			{
				RaceProgression->RecordRaceSession(SessionResult);
			}

			// If in championship, record championship result
			if (ChampionshipManager && ChampionshipManager->HasActiveChampionship())
			{
				SessionResult.bChampionshipRace = true;

				TMap<FString, int32> AIPositions;
				if (RaceManager)
				{
					for (const FRacerData& Racer : RaceManager->GetAllRacers())
					{
						if (!Racer.bIsPlayer)
						{
							AIPositions.Add(Racer.DisplayName, Racer.Position);
						}
					}
				}

				ChampionshipManager->RecordChampionshipRaceResult(RacerData.Position, AIPositions);
			}

			// Show ResultsWidget
			ANomiPlayerController* PC = Cast<ANomiPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
			if (PC)
			{
				UMenuManager* MenuMgr = PC->FindComponentByClass<UMenuManager>();
				if (MenuMgr)
				{
					MenuMgr->ShowResults(SessionResult);

					UResultsWidget* Results = CreateWidget<UResultsWidget>(PC, UResultsWidget::StaticClass());
					if (Results)
					{
						Results->SetMenuManager(MenuMgr);
						Results->SetResults(SessionResult, DefaultRaceConfig.bIsPointToPoint);
						Results->AddToViewport(10);
					}
				}
			}

			// Hide the race HUD
			if (RaceHUDWidget)
			{
				RaceHUDWidget->SetHUDVisible(false);
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
		NomiError::Log(ENomiErrorSeverity::Error, TEXT("Race"), TEXT("Cannot start championship: ChampionshipManager not available"));
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
