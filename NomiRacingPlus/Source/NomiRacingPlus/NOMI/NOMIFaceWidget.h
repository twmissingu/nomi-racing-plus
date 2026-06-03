// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NOMIController.h"
#include "NOMIFaceWidget.generated.h"

/**
 * NOMI eye shape types
 */
UENUM(BlueprintType)
enum class ENOMIEyeShape : uint8
{
	Normal     UMETA(DisplayName = "Normal"),
	Happy      UMETA(DisplayName = "Happy (slightly closed)"),
	Sad        UMETA(DisplayName = "Sad"),
	Surprised  UMETA(DisplayName = "Surprised (wide)")
};

/**
 * NOMI mouth shape types
 */
UENUM(BlueprintType)
enum class ENOMIMouthShape : uint8
{
	Neutral    UMETA(DisplayName = "Neutral"),
	Smile      UMETA(DisplayName = "Smile"),
	Frown      UMETA(DisplayName = "Frown"),
	Surprised  UMETA(DisplayName = "Surprised (O shape)"),
	Speaking   UMETA(DisplayName = "Speaking")
};

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

	// Eye shape type
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Eye")
	ENOMIEyeShape ShapeType = ENOMIEyeShape::Normal;

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

	// Mouth shape type
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|Mouth")
	ENOMIMouthShape ShapeType = ENOMIMouthShape::Neutral;

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
	void SetMouthShape(ENOMIMouthShape ShapeType);

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
