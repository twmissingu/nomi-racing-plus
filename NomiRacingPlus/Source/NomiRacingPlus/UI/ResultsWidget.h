// NomiRacingPlus - Results Widget

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResultsWidget.generated.h"

class UButton;
class UTextBlock;
class UMenuManager;
struct FRaceSessionResult;

UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API UResultsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PositionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TotalTimeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BestLapText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GapToFirstText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PointsChangeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TrackNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RematchButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GarageButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MainMenuButton;

	UPROPERTY()
	TObjectPtr<UMenuManager> MenuManager;

	UFUNCTION()
	void SetResults(const FRaceSessionResult& Result, bool bIsBajaMode);

	UFUNCTION()
	void SetMenuManager(UMenuManager* Manager);

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnRematch();

	UFUNCTION()
	void OnGarage();

	UFUNCTION()
	void OnMainMenu();

	// Format a time in seconds as MM:SS.mmm
	static FString FormatTime(float TimeInSeconds);

	// Get ordinal suffix for a position (e.g., "st", "nd", "rd", "th")
	static FString GetOrdinalSuffix(int32 Position);
};
