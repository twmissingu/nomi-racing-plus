// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/NomiErrorHandler.h"
#include "ErrorRecoveryWidget.generated.h"

/**
 * Recovery action options presented to the user
 */
UENUM(BlueprintType)
enum class ERecoveryAction : uint8
{
	RestoreBackup  UMETA(DisplayName = "Restore Backup"),
	ResetDefaults  UMETA(DisplayName = "Reset to Defaults"),
	Cancel         UMETA(DisplayName = "Cancel")
};

/**
 * Delegate broadcast when the user selects a recovery action
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecoveryActionSelected, ERecoveryAction, Action);

/**
 * In-game error recovery dialog for save corruption scenarios.
 * Displays when save integrity check fails, offering the user recovery options
 * instead of silently logging and falling back to defaults.
 *
 * Usage:
 *   UErrorRecoveryWidget* Widget = CreateWidget<UErrorRecoveryWidget>(PC, UErrorRecoveryWidget::StaticClass());
 *   Widget->ShowRecoveryDialog(TEXT("Save file is corrupted"), true);
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API UErrorRecoveryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UErrorRecoveryWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	/** Show the recovery dialog with a message and available options */
	UFUNCTION(BlueprintCallable, Category = "ErrorRecovery")
	void ShowRecoveryDialog(const FString& ErrorMessage, bool bCanRestoreBackup);

	/** Hide the recovery dialog */
	UFUNCTION(BlueprintCallable, Category = "ErrorRecovery")
	void HideRecoveryDialog();

	/** Check if dialog is currently visible */
	UFUNCTION(BlueprintCallable, Category = "ErrorRecovery")
	bool IsDialogVisible() const { return bIsVisible; }

	/** Broadcast when user selects a recovery action */
	UPROPERTY(BlueprintAssignable, Category = "ErrorRecovery")
	FOnRecoveryActionSelected OnRecoveryActionSelected;

protected:
	// --- Button callbacks ---
	UFUNCTION()
	void OnRestoreBackupClicked();

	UFUNCTION()
	void OnResetDefaultsClicked();

	UFUNCTION()
	void OnCancelClicked();

private:
	// --- UI state ---
	bool bIsVisible = false;
	bool bCanRestoreBackup = true;

	// --- Programmatic UI elements ---
	UPROPERTY()
	TObjectPtr<class UCanvasPanel> RootCanvas;

	UPROPERTY()
	TObjectPtr<class UBorder> OverlayBorder;

	UPROPERTY()
	TObjectPtr<class UVerticalBox> DialogContent;

	UPROPERTY()
	TObjectPtr<class UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<class UTextBlock> MessageText;

	UPROPERTY()
	TObjectPtr<class UVerticalBox> ButtonContainer;

	UPROPERTY()
	TObjectPtr<class UButton> RestoreBackupButton;

	UPROPERTY()
	TObjectPtr<class UTextBlock> RestoreBackupText;

	UPROPERTY()
	TObjectPtr<class UButton> ResetDefaultsButton;

	UPROPERTY()
	TObjectPtr<class UTextBlock> ResetDefaultsText;

	UPROPERTY()
	TObjectPtr<class UButton> CancelButton;

	UPROPERTY()
	TObjectPtr<class UTextBlock> CancelText;

	// --- Build UI programmatically ---
	void BuildLayout();

	// --- Create helper widgets ---
	UButton* CreateDialogButton(const FString& Label, const FLinearColor& ButtonColor);
};
