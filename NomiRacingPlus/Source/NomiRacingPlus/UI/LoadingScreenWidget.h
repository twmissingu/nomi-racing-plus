// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UButton;
class UMenuManager;

/**
 * Loading screen widget displayed while a track is being loaded.
 * Shows loading progress, track name, and random gameplay tips.
 */
UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API ULoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Set the loading progress (0.0 - 1.0). Updates bar and percentage text. */
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void SetProgress(float Percent);

	/** Set the track name displayed on the loading screen. */
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void SetTrackName(const FString& Name);

	/** Pick and display a random gameplay tip. */
	UFUNCTION(BlueprintCallable, Category = "Loading")
	void ShowRandomTip();

	/** Bind the menu manager for cancel/navigation support. */
	void SetMenuManager(UMenuManager* InMenuManager);

protected:
	virtual void NativeConstruct() override;

	// --- Bound Widgets ---

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> LoadingBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ProgressText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TipText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TrackNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CancelButton;

private:
	UPROPERTY()
	TObjectPtr<UMenuManager> MenuManager;

	/** Array of gameplay tips shown during loading. */
	static const TArray<FString> LoadingTips;

	UFUNCTION()
	void OnCancel();
};
