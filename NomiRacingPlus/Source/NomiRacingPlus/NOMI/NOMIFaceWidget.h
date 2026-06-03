// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NOMIController.h"
#include "NOMIFaceWidget.generated.h"

/**
 * Eye shape data for rendering
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNOMIEyeState
{
	GENERATED_BODY()

	// Eye openness (0=closed, 1=fully open)
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Eye")
	float Openness = 1.0f;

	// Eye shape type (0=normal, 1=happy, 2=sad, 3=angry, 4=surprised)
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Eye")
	int32 ShapeType = 0;

	// Pupil size (0=small, 1=large)
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Eye")
	float PupilSize = 0.5f;

	// Eye color
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Eye")
	FLinearColor Color = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);
};

/**
 * Mouth shape data for rendering
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNOMIMouthState
{
	GENERATED_BODY()

	// Mouth open amount (0=closed, 1=fully open)
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Mouth")
	float OpenAmount = 0.0f;

	// Mouth shape type (0=neutral, 1=smile, 2=frown, 3=surprised, 4=speaking)
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Mouth")
	int32 ShapeType = 0;

	// Mouth width scale
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Mouth")
	float WidthScale = 1.0f;
};

/**
 * NOMI Face Widget - renders NOMI's facial expressions
 * Displays eyes, mouth, and expression animations on a 2D surface
 * Attached to the NOMI robot's body as a Widget Component
 */
UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API UNOMIFaceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UNOMIFaceWidget(const FObjectInitializer& ObjectInitializer);

	// Update the face state based on expression
	UFUNCTION(BlueprintCallable, Category = "NOMI|Face")
	void UpdateExpression(ENOMIExpression Expression);

	// Update eye openness (for blink animation)
	UFUNCTION(BlueprintCallable, Category = "NOMI|Face")
	void SetEyeOpenness(float LeftEye, float RightEye);

	// Update mouth open amount (for speaking)
	UFUNCTION(BlueprintCallable, Category = "NOMI|Face")
	void SetMouthOpen(float Amount);

	// Set mouth shape type
	UFUNCTION(BlueprintCallable, Category = "NOMI|Face")
	void SetMouthShape(int32 ShapeType);

	// Get current expression
	UFUNCTION(BlueprintCallable, Category = "NOMI|Face")
	ENOMIExpression GetCurrentExpression() const { return CurrentExpression; }

	// Get left eye state
	UFUNCTION(BlueprintCallable, Category = "NOMI|Face")
	const FNOMIEyeState& GetLeftEyeState() const { return LeftEyeState; }

	// Get right eye state
	UFUNCTION(BlueprintCallable, Category = "NOMI|Face")
	const FNOMIEyeState& GetRightEyeState() const { return RightEyeState; }

	// Get mouth state
	UFUNCTION(BlueprintCallable, Category = "NOMI|Face")
	const FNOMIMouthState& GetMouthState() const { return MouthState; }

protected:
	// Current expression
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Face")
	ENOMIExpression CurrentExpression = ENOMIExpression::Neutral;

	// Left eye state
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Face")
	FNOMIEyeState LeftEyeState;

	// Right eye state
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Face")
	FNOMIEyeState RightEyeState;

	// Mouth state
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Face")
	FNOMIMouthState MouthState;

	// Expression transition speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NOMI|Face")
	float ExpressionTransitionSpeed = 5.0f;

private:
	// Apply expression-specific eye/mouth settings
	void ApplyExpressionSettings(ENOMIExpression Expression);

	// Expression animation state
	float ExpressionBlendAlpha = 0.0f;
	ENOMIExpression TargetExpression = ENOMIExpression::Neutral;
};
