// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "UI/MenuManager.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Race/RaceManager.h"

#include "UI/MainMenuWidget.h"
#include "UI/GarageWidget.h"
#include "UI/TrackSelectWidget.h"
#include "UI/RaceSettingsWidget.h"
#include "UI/LoadingScreenWidget.h"
#include "UI/PauseMenuWidget.h"
#include "UI/ResultsWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogNomiMenu, Log, All);

UMenuManager::UMenuManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMenuManager::Initialize(APlayerController* PC)
{
	if (!PC)
	{
		UE_LOG(LogNomiMenu, Error, TEXT("Initialize: PlayerController is null"));
		return;
	}

	OwningPlayer = PC;
	CurrentState = EMenuState::MainMenu;
	StateStack.Empty();

	UE_LOG(LogNomiMenu, Log, TEXT("MenuManager initialized"));
}

void UMenuManager::ShowMainMenu()
{
	SwitchToState(EMenuState::MainMenu);

	if (IsValid(OwningPlayer.Get()))
	{
		FInputModeUIOnly InputMode;
		OwningPlayer->SetInputMode(InputMode);
		OwningPlayer->SetShowMouseCursor(true);
	}

	UE_LOG(LogNomiMenu, Log, TEXT("Main menu displayed"));
}

void UMenuManager::ShowGarage()
{
	SwitchToState(EMenuState::Garage);
	UE_LOG(LogNomiMenu, Log, TEXT("Garage displayed, mode: %s"), *MenuContext.GameMode);
}

void UMenuManager::ShowTrackSelect()
{
	SwitchToState(EMenuState::TrackSelect);
	UE_LOG(LogNomiMenu, Log, TEXT("Track select displayed, mode: %s"), *MenuContext.GameMode);
}

void UMenuManager::ShowRaceSettings()
{
	SwitchToState(EMenuState::RaceSettings);
	UE_LOG(LogNomiMenu, Log, TEXT("Race settings displayed"));
}

void UMenuManager::ShowLoadingScreen()
{
	SwitchToState(EMenuState::Loading);

	if (IsValid(OwningPlayer.Get()))
	{
		FInputModeGameOnly InputMode;
		OwningPlayer->SetInputMode(InputMode);
		OwningPlayer->SetShowMouseCursor(false);
	}

	UE_LOG(LogNomiMenu, Log, TEXT("Loading screen displayed"));
}

void UMenuManager::ShowPauseMenu()
{
	// SwitchToState already pushes CurrentState to the stack when transitioning
	// to a non-MainMenu state. Avoid double-pushing Racing here.
	SwitchToState(EMenuState::Paused);

	if (IsValid(OwningPlayer.Get()))
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		OwningPlayer->SetInputMode(InputMode);
		OwningPlayer->SetShowMouseCursor(true);
	}

	UE_LOG(LogNomiMenu, Log, TEXT("Pause menu displayed"));
}

void UMenuManager::ShowResults(const FRaceSessionResult& Result)
{
	SwitchToState(EMenuState::Results);

	if (IsValid(OwningPlayer.Get()))
	{
		FInputModeUIOnly InputMode;
		OwningPlayer->SetInputMode(InputMode);
		OwningPlayer->SetShowMouseCursor(true);
	}

	UE_LOG(LogNomiMenu, Log, TEXT("Results displayed, position: %d"), Result.FinalPosition);
}

void UMenuManager::ReturnToPrevious()
{
	if (StateStack.Num() == 0)
	{
		UE_LOG(LogNomiMenu, Warning, TEXT("ReturnToPrevious: State stack is empty, returning to main menu"));
		ShowMainMenu();
		return;
	}

	EMenuState PreviousState = StateStack.Pop();
	SwitchToState(PreviousState);

	UE_LOG(LogNomiMenu, Log, TEXT("Returned to previous state: %d"), static_cast<int32>(PreviousState));
}

void UMenuManager::StartRace()
{
	UE_LOG(LogNomiMenu, Log, TEXT("Starting race: Mode=%s, Track=%s, Vehicle=%d, AI=%d, Diff=%d, Laps=%d, Weather=%d"),
		*MenuContext.GameMode,
		*MenuContext.TrackName,
		static_cast<int32>(MenuContext.VehicleType),
		MenuContext.AICount,
		MenuContext.Difficulty,
		MenuContext.Laps,
		MenuContext.Weather);

	ShowLoadingScreen();

	// Build race config from menu context
	FRaceConfig Config;
	Config.TrackName = MenuContext.TrackName;
	Config.RaceMode = MenuContext.GameMode;
	Config.MaxAIOpponents = MenuContext.AICount;
	Config.NumLaps = MenuContext.Laps;

	// Open the race track level. The level's World Settings should configure
	// NomiRaceGameMode, which auto-starts the race after a delay.
	// Store MenuContext in GameInstance so the GameMode can read vehicle/settings.
	if (IsValid(OwningPlayer.Get()) && OwningPlayer->GetWorld())
	{
		// Check if we already have a RaceManager (i.e., we're on a race track)
		TArray<AActor*> RaceManagers;
		UGameplayStatics::GetAllActorsOfClass(OwningPlayer->GetWorld(), ARaceManager::StaticClass(), RaceManagers);

		if (RaceManagers.Num() > 0)
		{
			ARaceManager* RaceManager = Cast<ARaceManager>(RaceManagers[0]);
			if (RaceManager)
			{
				RaceManager->StartRace(Config);
				CurrentState = EMenuState::Racing;
				UE_LOG(LogNomiMenu, Log, TEXT("Race started via RaceManager"));
			}
		}
		else
		{
			// No RaceManager — we're on the menu level. Open the race track.
			FString LevelName = MenuContext.TrackName.IsEmpty() ? TEXT("TestTrack") : MenuContext.TrackName;
			UGameplayStatics::OpenLevel(OwningPlayer->GetWorld(), FName(*LevelName));
			UE_LOG(LogNomiMenu, Log, TEXT("Opening race level: %s"), *LevelName);
		}
	}
}

EMenuState UMenuManager::GetCurrentState() const
{
	return CurrentState;
}

const FMenuContext& UMenuManager::GetMenuContext() const
{
	return MenuContext;
}

void UMenuManager::SetGameMode(const FString& Mode)
{
	MenuContext.GameMode = Mode;
	UE_LOG(LogNomiMenu, Log, TEXT("Game mode set to: %s"), *Mode);
}

void UMenuManager::SetVehicleType(ENIOVehicleType Type)
{
	MenuContext.VehicleType = Type;
	UE_LOG(LogNomiMenu, Log, TEXT("Vehicle type set to: %d"), static_cast<int32>(Type));
}

void UMenuManager::SetTrack(const FString& InTrackName)
{
	MenuContext.TrackName = InTrackName;
	UE_LOG(LogNomiMenu, Log, TEXT("Track set to: %s"), *InTrackName);
}

void UMenuManager::SetRaceSettings(int32 AI, int32 Diff, int32 InLaps, int32 InWeather)
{
	MenuContext.AICount = FMath::Clamp(AI, 0, 15);
	MenuContext.Difficulty = FMath::Clamp(Diff, 0, 100);
	MenuContext.Laps = FMath::Clamp(InLaps, 1, 50);
	MenuContext.Weather = FMath::Clamp(InWeather, 0, 10);

	UE_LOG(LogNomiMenu, Log, TEXT("Race settings updated: AI=%d, Diff=%d, Laps=%d, Weather=%d"),
		MenuContext.AICount, MenuContext.Difficulty, MenuContext.Laps, MenuContext.Weather);
}

UUserWidget* UMenuManager::CreateWidgetForState(EMenuState State)
{
	if (!IsValid(OwningPlayer.Get()))
	{
		UE_LOG(LogNomiMenu, Error, TEXT("CreateWidgetForState: OwningPlayer is null"));
		return nullptr;
	}

	// TODO: Replace with actual widget classes when implemented
	// Example pattern for each state:
	//   case EMenuState::MainMenu:
	//       return CreateWidget<UMainMenuWidget>(OwningPlayer);

	switch (State)
	{
	case EMenuState::MainMenu:
		UE_LOG(LogNomiMenu, Log, TEXT("Creating MainMenu widget"));
		return CreateWidget<UMainMenuWidget>(OwningPlayer);

	case EMenuState::Garage:
		UE_LOG(LogNomiMenu, Log, TEXT("Creating Garage widget (mode: %s)"), *MenuContext.GameMode);
		return CreateWidget<UGarageWidget>(OwningPlayer);

	case EMenuState::TrackSelect:
		UE_LOG(LogNomiMenu, Log, TEXT("Creating TrackSelect widget (mode: %s)"), *MenuContext.GameMode);
		return CreateWidget<UTrackSelectWidget>(OwningPlayer);

	case EMenuState::RaceSettings:
		UE_LOG(LogNomiMenu, Log, TEXT("Creating RaceSettings widget"));
		return CreateWidget<URaceSettingsWidget>(OwningPlayer);

	case EMenuState::Loading:
		UE_LOG(LogNomiMenu, Log, TEXT("Creating LoadingScreen widget"));
		return CreateWidget<ULoadingScreenWidget>(OwningPlayer);

	case EMenuState::Paused:
		UE_LOG(LogNomiMenu, Log, TEXT("Creating PauseMenu widget"));
		return CreateWidget<UPauseMenuWidget>(OwningPlayer);

	case EMenuState::Results:
		UE_LOG(LogNomiMenu, Log, TEXT("Creating Results widget"));
		return CreateWidget<UResultsWidget>(OwningPlayer);

	case EMenuState::Racing:
		// Racing state does not have a widget
		break;

	default:
		UE_LOG(LogNomiMenu, Warning, TEXT("CreateWidgetForState: Unknown state %d"), static_cast<int32>(State));
		break;
	}

	return nullptr;
}

void UMenuManager::SwitchToState(EMenuState NewState)
{
	// Remove current widget from viewport
	if (CurrentWidget)
	{
		CurrentWidget->RemoveFromParent();
		CurrentWidget = nullptr;
	}

	// Push current state to stack for back navigation (unless going to MainMenu)
	if (CurrentState != EMenuState::MainMenu && NewState != EMenuState::MainMenu)
	{
		StateStack.Push(CurrentState);
	}

	// Update state
	CurrentState = NewState;

	// Create and display new widget
	UUserWidget* NewWidget = CreateWidgetForState(NewState);
	if (NewWidget)
	{
		CurrentWidget = NewWidget;
		CurrentWidget->AddToViewport();
	}

	UE_LOG(LogNomiMenu, Verbose, TEXT("Switched to state: %d"), static_cast<int32>(NewState));
}
