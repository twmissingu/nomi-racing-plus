// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "TutorialManager.h"
#include "Core/NomiGameInstance.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY(LogNomiTutorial);

UTutorialManager::UTutorialManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTutorialManager::BeginPlay()
{
	Super::BeginPlay();
	InitializeStepData();
}

void UTutorialManager::StartTutorial(ETutorialStep StartStep)
{
	CurrentStep = StartStep;
	bIsCompleted = false;

	UE_LOG(LogNomiTutorial, Log, TEXT("Tutorial started at step: %s"), *UEnum::GetValueAsString(CurrentStep));
	OnTutorialStepChanged.Broadcast(CurrentStep);
}

void UTutorialManager::AdvanceStep()
{
	if (bIsCompleted)
	{
		UE_LOG(LogNomiTutorial, Warning, TEXT("Tutorial already completed, cannot advance"));
		return;
	}

	// Move to next step
	const int32 CurrentIndex = static_cast<int32>(CurrentStep);
	const int32 NextIndex = CurrentIndex + 1;

	// Check if we have reached the end
	if (NextIndex >= static_cast<int32>(ETutorialStep::Complete))
	{
		CompleteTutorial(false);
		return;
	}

	CurrentStep = static_cast<ETutorialStep>(NextIndex);
	UE_LOG(LogNomiTutorial, Log, TEXT("Tutorial advanced to step: %s"), *UEnum::GetValueAsString(CurrentStep));
	OnTutorialStepChanged.Broadcast(CurrentStep);
}

void UTutorialManager::SkipTutorial()
{
	UE_LOG(LogNomiTutorial, Log, TEXT("Tutorial skipped"));
	CompleteTutorial(true);
}

ETutorialStep UTutorialManager::GetCurrentStep() const
{
	return CurrentStep;
}

bool UTutorialManager::IsTutorialComplete() const
{
	return bIsCompleted;
}

const FTutorialStepData& UTutorialManager::GetStepData(ETutorialStep Step) const
{
	// Find the step data for the requested step
	for (const FTutorialStepData& Data : StepData)
	{
		if (Data.Step == Step)
		{
			return Data;
		}
	}

	// Should never happen if initialized correctly, but return first as fallback
	UE_LOG(LogNomiTutorial, Error, TEXT("Step data not found for step: %s"), *UEnum::GetValueAsString(Step));
	return StepData[0];
}

void UTutorialManager::InitializeStepData()
{
	// Only initialize if empty (allow editor overrides)
	if (StepData.Num() > 0)
	{
		return;
	}

	StepData.Empty();

	FTutorialStepData WelcomeData;
	WelcomeData.Step = ETutorialStep::Welcome;
	WelcomeData.InstructionText = TEXT("Welcome to NIO Racing! Let's learn the basics of driving.");
	WelcomeData.HighlightWidgetName = TEXT("");
	WelcomeData.CompletionCondition = TEXT("Wait");
	StepData.Add(WelcomeData);

	FTutorialStepData SteeringData;
	SteeringData.Step = ETutorialStep::Steering;
	SteeringData.InstructionText = TEXT("Use the steering wheel or left stick to turn your vehicle.");
	SteeringData.HighlightWidgetName = TEXT("SteeringWidget");
	SteeringData.CompletionCondition = TEXT("SteerLeft");
	StepData.Add(SteeringData);

	FTutorialStepData ThrottleData;
	ThrottleData.Step = ETutorialStep::Throttle;
	ThrottleData.InstructionText = TEXT("Press the accelerator to speed up.");
	ThrottleData.HighlightWidgetName = TEXT("ThrottleWidget");
	ThrottleData.CompletionCondition = TEXT("ThrottlePressed");
	StepData.Add(ThrottleData);

	FTutorialStepData BrakeData;
	BrakeData.Step = ETutorialStep::Brake;
	BrakeData.InstructionText = TEXT("Press the brake to slow down and stop.");
	BrakeData.HighlightWidgetName = TEXT("BrakeWidget");
	BrakeData.CompletionCondition = TEXT("BrakePressed");
	StepData.Add(BrakeData);

	FTutorialStepData DriftingData;
	DriftingData.Step = ETutorialStep::Drifting;
	DriftingData.InstructionText = TEXT("Use handbrake while turning to drift around corners.");
	DriftingData.HighlightWidgetName = TEXT("DriftWidget");
	DriftingData.CompletionCondition = TEXT("DriftActivated");
	StepData.Add(DriftingData);

	FTutorialStepData CompleteData;
	CompleteData.Step = ETutorialStep::Complete;
	CompleteData.InstructionText = TEXT("Tutorial complete! You're ready to race.");
	CompleteData.HighlightWidgetName = TEXT("");
	CompleteData.CompletionCondition = TEXT("");
	StepData.Add(CompleteData);
}

void UTutorialManager::CompleteTutorial(bool bWasSkipped)
{
	bIsCompleted = true;
	CurrentStep = ETutorialStep::Complete;

	// Persist tutorial completion to game settings
	UWorld* World = GetWorld();
	if (World)
	{
		UGameInstance* GameInstance = World->GetGameInstance();
		if (GameInstance)
		{
			UNomiGameInstance* NomiGameInstance = Cast<UNomiGameInstance>(GameInstance);
			if (NomiGameInstance)
			{
				FNomiGameSettings Settings = NomiGameInstance->GetSettings();
				Settings.bTutorialCompleted = true;
				NomiGameInstance->UpdateSettings(Settings);
				NomiGameInstance->SaveSettings();
				UE_LOG(LogNomiTutorial, Log, TEXT("Tutorial completion persisted to game settings"));
			}
		}
	}

	OnTutorialStepChanged.Broadcast(CurrentStep);
	UE_LOG(LogNomiTutorial, Log, TEXT("Tutorial completed (skipped: %s)"), bWasSkipped ? TEXT("Yes") : TEXT("No"));
}