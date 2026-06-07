// NIO Racing Plus - Garage Widget

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GarageWidget.generated.h"

class UButton;
class UTextBlock;
class UMenuManager;
enum class ENIOVehicleType : uint8;

UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API UGarageWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PrevVehicleButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NextVehicleButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SelectButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> VehicleNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ModeNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ModeDescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PowerText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TorqueText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AccelText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TopSpeedText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DriveTypeText;

	UPROPERTY()
	TObjectPtr<UMenuManager> MenuManager;

	UPROPERTY()
	int32 CurrentVehicleIndex = 0;

	UPROPERTY()
	TArray<ENIOVehicleType> AvailableVehicles;

	UFUNCTION()
	void SetMenuManager(UMenuManager* Manager);

	UFUNCTION()
	void SetModeFilter(const FString& Mode);

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnPrevClicked();

	UFUNCTION()
	void OnNextClicked();

	UFUNCTION()
	void OnSelectClicked();

	UFUNCTION()
	void OnBackClicked();

	void UpdateVehicleDisplay();

	/** Apply or clear empty state UI (shared by NativeConstruct and SetModeFilter) */
	void ApplyEmptyState(bool bEmpty);
};
