// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Race/RaceProgression.h"
#include "Vehicles/VehicleStateManager.h"
#include "MenuManager.generated.h"

class UUserWidget;
class APlayerController;

/**
 * Menu states for navigation
 */
UENUM(BlueprintType)
enum class EMenuState : uint8
{
	MainMenu     UMETA(DisplayName = "Main Menu"),
	Garage       UMETA(DisplayName = "Garage"),
	TrackSelect  UMETA(DisplayName = "Track Select"),
	RaceSettings UMETA(DisplayName = "Race Settings"),
	Loading      UMETA(DisplayName = "Loading"),
	Racing       UMETA(DisplayName = "Racing"),
	Paused       UMETA(DisplayName = "Paused"),
	Results      UMETA(DisplayName = "Results"),
	Settings     UMETA(DisplayName = "Settings")
};

/**
 * Menu context storing current selections and race configuration
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FMenuContext
{
	GENERATED_BODY()

	// Current game mode: "GT", "NIO", or "Baja"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	FString GameMode = TEXT("GT");

	// Selected vehicle type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	ENIOVehicleType VehicleType = ENIOVehicleType::EP9;

	// Selected track name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	FString TrackName;

	// Number of AI opponents
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|RaceSettings")
	int32 AICount = 7;

	// AI difficulty (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|RaceSettings")
	int32 Difficulty = 50;

	// Number of laps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|RaceSettings")
	int32 Laps = 3;

	// Weather preset index (0=Clear, 1=Rain, 2=Storm, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|RaceSettings")
	int32 Weather = 0;

	// Vehicle selection index in garage (for state preservation on back-navigation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	int32 VehicleIndex = 0;

	// Track selection index (for state preservation on back-navigation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu")
	int32 TrackIndex = 0;
};

/**
 * Menu Manager - handles UI menu navigation and state transitions
 * Manages the flow between main menu, garage, track selection, race settings,
 * loading screen, pause menu, and results display.
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UMenuManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UMenuManager();

	// Initialize the menu manager with a player controller
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void Initialize(APlayerController* PC);

	// Show the main menu
	UFUNCTION(BlueprintCallable, Category = "Menu|Navigation")
	void ShowMainMenu();

	// Show the garage for vehicle selection
	UFUNCTION(BlueprintCallable, Category = "Menu|Navigation")
	void ShowGarage();

	// Show track selection screen
	UFUNCTION(BlueprintCallable, Category = "Menu|Navigation")
	void ShowTrackSelect();

	// Show race settings screen
	UFUNCTION(BlueprintCallable, Category = "Menu|Navigation")
	void ShowRaceSettings();

	// Show loading screen
	UFUNCTION(BlueprintCallable, Category = "Menu|Navigation")
	void ShowLoadingScreen();

	// Show pause menu (overlay)
	UFUNCTION(BlueprintCallable, Category = "Menu|Navigation")
	void ShowPauseMenu();

	// Show results screen with race data
	UFUNCTION(BlueprintCallable, Category = "Menu|Navigation")
	void ShowResults(const FRaceSessionResult& Result);

	// Show settings screen
	UFUNCTION(BlueprintCallable, Category = "Menu|Navigation")
	void ShowSettings();

	// Return to the previous menu state
	UFUNCTION(BlueprintCallable, Category = "Menu|Navigation")
	void ReturnToPrevious();

	// Start the race with current settings
	UFUNCTION(BlueprintCallable, Category = "Menu|Race")
	void StartRace();

	// Get current menu state
	UFUNCTION(BlueprintCallable, Category = "Menu")
	EMenuState GetCurrentState() const;

	// Get current menu context (read-only)
	UFUNCTION(BlueprintCallable, Category = "Menu")
	const FMenuContext& GetMenuContext() const;

	// Update menu context (for state preservation)
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void SetMenuContext(const FMenuContext& NewContext);

	// Set game mode (GT, NIO, Baja)
	UFUNCTION(BlueprintCallable, Category = "Menu|Settings")
	void SetGameMode(const FString& Mode);

	// Set selected vehicle type
	UFUNCTION(BlueprintCallable, Category = "Menu|Settings")
	void SetVehicleType(ENIOVehicleType Type);

	// Set selected track
	UFUNCTION(BlueprintCallable, Category = "Menu|Settings")
	void SetTrack(const FString& TrackName);

	// Set race settings
	UFUNCTION(BlueprintCallable, Category = "Menu|Settings")
	void SetRaceSettings(int32 AI, int32 Diff, int32 Laps, int32 Weather);

private:
	// Current menu state
	UPROPERTY()
	EMenuState CurrentState = EMenuState::MainMenu;

	// Current menu context with selections
	UPROPERTY()
	FMenuContext MenuContext;

	// Active widget currently displayed
	UPROPERTY()
	TObjectPtr<UUserWidget> CurrentWidget;

	// State stack for back navigation
	UPROPERTY()
	TArray<EMenuState> StateStack;

	// Owning player controller
	UPROPERTY()
	TObjectPtr<APlayerController> OwningPlayer;

	// Create the appropriate widget for a given state
	UUserWidget* CreateWidgetForState(EMenuState State);

	// Switch to a new state, removing old widget and creating new one
	void SwitchToState(EMenuState NewState);
};
