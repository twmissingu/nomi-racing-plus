// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialWidget.generated.h"

class UButton;
class UTextBlock;
class UProgressBar;
class UTutorialManager;
struct FTutorialStepData;

/**
 * Tutorial Widget — overlay displayed during the driving tutorial.
 * Shows step instructions, progress, and Next / Skip controls.
 * The widget expects a matching UMG blueprint where the BindWidget
 * targets (InstructionText, NextButton, SkipButton, ProgressBar) exist.
 */
UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API UTutorialWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ── BindWidgets ──────────────────────────────────────────────

	/** Multi-line instruction text for the current tutorial step */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InstructionText;

	/** Button to advance to the next tutorial step */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NextButton;

	/** Button to skip the entire tutorial */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SkipButton;

	/** Progress bar showing completion through the tutorial steps */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;

	// ── Public API ───────────────────────────────────────────────

	/** Update the overlay to display a new tutorial step */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void ShowStep(const FTutorialStepData& StepData);

	/** Assign the TutorialManager that owns this tutorial flow */
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void SetTutorialManager(UTutorialManager* Manager);

	// ── Overrides ────────────────────────────────────────────────

protected:
	virtual void NativeConstruct() override;

private:
	/** Reference to the tutorial manager driving this widget */
	UPROPERTY()
	TObjectPtr<UTutorialManager> TutorialManager;

	/** Cached total number of tutorial steps (for progress calc) */
	int32 TotalSteps = 0;

	// ── Button callbacks ─────────────────────────────────────────

	UFUNCTION()
	void OnNextClicked();

	UFUNCTION()
	void OnSkipClicked();
};
