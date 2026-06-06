// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UMenuManager;
class UTutorialManager;

UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GTModeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NIOModeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BajaModeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	/** Tutorial button — optional in case the blueprint omits it */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> TutorialButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	/** Mode description text — shown on hover */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ModeDescriptionText;

	UPROPERTY()
	TObjectPtr<UMenuManager> MenuManager;

	UFUNCTION()
	void SetMenuManager(UMenuManager* Manager);

	/** Assign the tutorial manager for starting the tutorial */
	UFUNCTION(BlueprintCallable, Category = "Menu|Tutorial")
	void SetTutorialManager(UTutorialManager* Manager);

	/** Called on first launch to auto-start the tutorial */
	void AutoStartTutorialIfNeeded();

protected:
	virtual void NativeConstruct() override;

private:
	/** Reference to the tutorial manager */
	UPROPERTY()
	TObjectPtr<UTutorialManager> TutorialManager;

	UFUNCTION()
	void OnGTModeClicked();

	UFUNCTION()
	void OnNIOModeClicked();

	UFUNCTION()
	void OnBajaModeClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnExitClicked();

	UFUNCTION()
	void OnTutorialClicked();

	/** Update mode description on hover */
	UFUNCTION()
	void OnGTModeHovered();

	UFUNCTION()
	void OnNIOModeHovered();

	UFUNCTION()
	void OnBajaModeHovered();

	UFUNCTION()
	void OnModeUnhovered();

	/** Set the description text (or clear if empty) */
	void SetModeDescription(const FString& Text);
};
