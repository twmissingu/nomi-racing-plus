// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Core/NomiErrorHandler.h"
#include "ErrorToastWidget.generated.h"

/**
 * A single toast notification entry
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FToastEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Toast")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "Toast")
	ENomiErrorSeverity Severity = ENomiErrorSeverity::Info;

	UPROPERTY(BlueprintReadOnly, Category = "Toast")
	float RemainingTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Toast")
	bool bDismissed = false;
};

/**
 * In-game toast notification widget for user-facing error feedback.
 * Displays brief, auto-dismissing messages with severity-based styling.
 *
 * Usage:
 *   UErrorToastWidget* Toast = CreateWidget<UErrorToastWidget>(PC, UErrorToastWidget::StaticClass());
 *   Toast->AddToViewport(100);
 *   Toast->ShowToast(TEXT("Failed to save progress"), ENomiErrorSeverity::Error);
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API UErrorToastWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UErrorToastWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Show a toast notification */
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void ShowToast(const FString& Message, ENomiErrorSeverity Severity = ENomiErrorSeverity::Info);

	/** Dismiss all active toasts */
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void DismissAll();

	/** Set auto-dismiss duration for new toasts */
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void SetDismissDuration(float Duration) { DismissDuration = FMath::Max(Duration, 1.0f); }

	/** Set maximum number of visible toasts */
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void SetMaxVisible(int32 Max) { MaxVisibleToasts = FMath::Max(Max, 1); }

	/** Bind to NomiError::OnError delegate — call in NativeConstruct */
	UFUNCTION(BlueprintCallable, Category = "Toast")
	void BindToErrorHandler();

protected:
	/** Container for toast entries — bound in UMG or created in C++ */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ToastContainer;

	/** Auto-dismiss duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toast")
	float DismissDuration = 4.0f;

	/** Maximum visible toasts before oldest is removed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Toast")
	int32 MaxVisibleToasts = 3;

private:
	/** Active toast entries */
	TArray<FToastEntry> ActiveToasts;

	/** Rebuild the visible toast list from ActiveToasts */
	void RebuildDisplay();

	/** Create a single toast row widget */
	UTextBlock* CreateToastRow(const FToastEntry& Entry) const;

	/** Get color for severity level */
	FLinearColor GetSeverityColor(ENomiErrorSeverity Severity) const;

	/** Get prefix icon character for severity */
	FString GetSeverityPrefix(ENomiErrorSeverity Severity) const;

	/** Callback for NomiError::OnError delegate */
	UFUNCTION()
	void OnErrorHandlerLog(ENomiErrorSeverity Severity, const FString& Message);
};
