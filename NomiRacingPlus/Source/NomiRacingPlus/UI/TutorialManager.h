// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TutorialManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNomiTutorial, Log, All);

/**
 * Tutorial steps for the driving tutorial
 */
UENUM(BlueprintType)
enum class ETutorialStep : uint8
{
	Welcome         UMETA(DisplayName = "Welcome"),
	Steering        UMETA(DisplayName = "Steering"),
	Throttle        UMETA(DisplayName = "Throttle"),
	Brake           UMETA(DisplayName = "Brake"),
	Drifting        UMETA(DisplayName = "Drifting"),
	Complete        UMETA(DisplayName = "Complete")
};

/**
 * Data for a single tutorial step
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FTutorialStepData
{
	GENERATED_BODY()

	// The step this data represents
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	ETutorialStep Step = ETutorialStep::Welcome;

	// Instruction text shown to the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FString InstructionText;

	// Name of widget to highlight during this step (optional)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FString HighlightWidgetName;

	// Condition that must be met to advance (e.g., "SteerLeft", "ThrottlePressed")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tutorial")
	FString CompletionCondition;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialStepChanged, ETutorialStep, NewStep);

/**
 * Tutorial Manager - handles driving tutorial progression
 * Manages step-by-step tutorial with highlighting and completion conditions.
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UTutorialManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UTutorialManager();

	// Start the tutorial from the beginning or a specific step
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void StartTutorial(ETutorialStep StartStep = ETutorialStep::Welcome);

	// Advance to the next step (call when completion condition is met)
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void AdvanceStep();

	// Skip the entire tutorial
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void SkipTutorial();

	// Get the current step
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	ETutorialStep GetCurrentStep() const;

	// Check if tutorial is complete
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	bool IsTutorialComplete() const;

	// Get data for a specific step
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	const FTutorialStepData& GetStepData(ETutorialStep Step) const;

	// Blueprint-assignable delegate for step changes
	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FOnTutorialStepChanged OnTutorialStepChanged;

protected:
	virtual void BeginPlay() override;

private:
	// Current step
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial", meta = (AllowPrivateAccess = "true"))
	ETutorialStep CurrentStep = ETutorialStep::Welcome;

	// Whether tutorial has been completed
	UPROPERTY(BlueprintReadOnly, Category = "Tutorial", meta = (AllowPrivateAccess = "true"))
	bool bIsCompleted = false;

	// Array of step data for each step
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial")
	TArray<FTutorialStepData> StepData;

	// Initialize default step data if empty
	void InitializeStepData();

	// Mark tutorial as complete and persist to settings
	void CompleteTutorial(bool bWasSkipped = false);
};
