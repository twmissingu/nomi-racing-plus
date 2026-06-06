// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "UI/TutorialWidget.h"
#include "UI/TutorialManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

DEFINE_LOG_CATEGORY_STATIC(LogNomiTutorialWidget, Log, All);

// ────────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────────

void UTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button delegates
	if (NextButton)
	{
		NextButton->OnClicked.AddDynamic(this, &UTutorialWidget::OnNextClicked);
	}
	else
	{
		UE_LOG(LogNomiTutorialWidget, Warning, TEXT("NextButton BindWidget not found in widget blueprint."));
	}

	if (SkipButton)
	{
		SkipButton->OnClicked.AddDynamic(this, &UTutorialWidget::OnSkipClicked);
	}
	else
	{
		UE_LOG(LogNomiTutorialWidget, Warning, TEXT("SkipButton BindWidget not found in widget blueprint."));
	}

	// Start with progress at zero
	if (ProgressBar)
	{
		ProgressBar->SetPercent(0.f);
	}
}

// ────────────────────────────────────────────────────────────────
// Public API
// ────────────────────────────────────────────────────────────────

void UTutorialWidget::ShowStep(const FTutorialStepData& StepData)
{
	// Update instruction text
	if (InstructionText)
	{
		const FText DisplayText = FText::FromString(StepData.InstructionText);
		InstructionText->SetText(DisplayText);
	}

	// Update progress bar based on step index
	if (ProgressBar && TotalSteps > 0)
	{
		const int32 StepIndex = static_cast<int32>(StepData.Step);
		const float Progress = static_cast<float>(StepIndex) / static_cast<float>(TotalSteps);
		ProgressBar->SetPercent(FMath::Clamp(Progress, 0.f, 1.f));
	}

	// On the final step, disable Next and show completion state
	if (NextButton)
	{
		const bool bIsFinalStep = (StepData.Step == ETutorialStep::Complete);
		NextButton->SetIsEnabled(!bIsFinalStep);
	}

	UE_LOG(LogNomiTutorialWidget, Verbose, TEXT("ShowStep: %s"), *StepData.InstructionText);
}

void UTutorialWidget::SetTutorialManager(UTutorialManager* Manager)
{
	TutorialManager = Manager;

	// Cache total steps from the manager's step data
	if (TutorialManager)
	{
		// Enum count minus 1 (the Complete "step" is not a real teaching step)
		TotalSteps = static_cast<int32>(ETutorialStep::Complete);
	}
}

// ────────────────────────────────────────────────────────────────
// Button callbacks
// ────────────────────────────────────────────────────────────────

void UTutorialWidget::OnNextClicked()
{
	if (TutorialManager)
	{
		TutorialManager->AdvanceStep();
	}
	else
	{
		UE_LOG(LogNomiTutorialWidget, Warning, TEXT("OnNextClicked: TutorialManager is null."));
	}
}

void UTutorialWidget::OnSkipClicked()
{
	if (TutorialManager)
	{
		TutorialManager->SkipTutorial();
	}
	else
	{
		UE_LOG(LogNomiTutorialWidget, Warning, TEXT("OnSkipClicked: TutorialManager is null."));
	}
}
