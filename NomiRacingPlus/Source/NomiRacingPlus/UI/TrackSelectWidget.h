// NomiRacingPlus - Track Select Widget

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TrackSelectWidget.generated.h"

class UButton;
class UTextBlock;
class UMenuManager;

/**
 * Track information data
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FTrackInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FString Description;

	// Supported modes: "GT", "NIO", "Baja", "All"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FString Mode;
};

/**
 * Track selection widget - allows player to browse and select a track
 */
UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API UTrackSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PrevTrackButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NextTrackButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SelectButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TrackNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TrackDescText;

	UPROPERTY()
	int32 CurrentTrackIndex = 0;

	UPROPERTY()
	TArray<FTrackInfo> AvailableTracks;

	UPROPERTY()
	TObjectPtr<UMenuManager> MenuManager;

	UFUNCTION()
	void SetMenuManager(UMenuManager* Manager);

	// Filter tracks by mode ("GT", "NIO", "Baja"); pass empty to show all
	UFUNCTION(BlueprintCallable, Category = "Track")
	void SetModeFilter(const FString& Mode);

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	FString ActiveModeFilter;

	UPROPERTY()
	TArray<FTrackInfo> AllTracks;

	UFUNCTION()
	void OnPrevClicked();

	UFUNCTION()
	void OnNextClicked();

	UFUNCTION()
	void OnSelectClicked();

	UFUNCTION()
	void OnBackClicked();

	void UpdateTrackDisplay();
	void PopulateAllTracks();
};
