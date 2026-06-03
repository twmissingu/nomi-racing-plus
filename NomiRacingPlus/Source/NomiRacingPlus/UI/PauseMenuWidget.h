/**
 * PauseMenuWidget.h
 * UI widget for the in-game pause menu.
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UMenuManager;

/**
 * Abstract pause menu widget that can be extended in Blueprint.
 * Expects BindWidget buttons: ContinueButton, RestartButton, SettingsButton, MainMenuButton.
 * Expects BindWidget text: PauseTitleText.
 */
UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Assigns the menu manager that owns this widget. */
	void SetMenuManager(UMenuManager* InMenuManager);

protected:
	virtual void NativeConstruct() override;

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ContinueButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RestartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PauseTitleText;

	// --- Button Handlers ---

	UFUNCTION()
	void OnContinue();

	UFUNCTION()
	void OnRestart();

	UFUNCTION()
	void OnSettings();

	UFUNCTION()
	void OnMainMenu();

private:
	UPROPERTY()
	TObjectPtr<UMenuManager> MenuManager;
};
