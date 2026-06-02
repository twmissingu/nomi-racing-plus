// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NOMI/CommentaryEngine.h"
#include "NOMIController.generated.h"

/**
 * NOMI facial expression types
 */
UENUM(BlueprintType)
enum class ENOMIExpression : uint8
{
	Neutral      UMETA(DisplayName = "Neutral"),
	Happy        UMETA(DisplayName = "Happy"),
	Sad          UMETA(DisplayName = "Sad"),
	Sleepy       UMETA(DisplayName = "Sleepy"),
	Curious      UMETA(DisplayName = "Curious"),
	Surprised    UMETA(DisplayName = "Surprised"),
	Confused     UMETA(DisplayName = "Confused"),
	Excited      UMETA(DisplayName = "Excited")
};

/**
 * NOMI Controller - manages NOMI robot's visual representation
 * Controls facial expressions, animations, and comment display
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API ANOMIController : public AActor
{
	GENERATED_BODY()

public:
	ANOMIController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Attach NOMI to a vehicle
	UFUNCTION(BlueprintCallable, Category = "NOMI")
	void AttachToVehicle(AActor* Vehicle);

	// Detach NOMI from vehicle
	UFUNCTION(BlueprintCallable, Category = "NOMI")
	void DetachFromVehicle();

	// Set facial expression
	UFUNCTION(BlueprintCallable, Category = "NOMI|Expression")
	void SetExpression(ENOMIExpression Expression);

	// Get current expression
	UFUNCTION(BlueprintCallable, Category = "NOMI|Expression")
	ENOMIExpression GetExpression() const { return CurrentExpression; }

	// Play blink animation
	UFUNCTION(BlueprintCallable, Category = "NOMI|Animation")
	void PlayBlink();

	// Set eye shape (0-1, for animation)
	UFUNCTION(BlueprintCallable, Category = "NOMI|Animation")
	void SetEyeShape(float Shape);

	// Set mouth open (0-1, for speaking)
	UFUNCTION(BlueprintCallable, Category = "NOMI|Animation")
	void SetMouthOpen(float OpenAmount);

	// Get commentary engine
	UFUNCTION(BlueprintCallable, Category = "NOMI")
	UCommentaryEngine* GetCommentaryEngine() const { return CommentaryEngine; }

	// Look at target (rotate head)
	UFUNCTION(BlueprintCallable, Category = "NOMI|Animation")
	void LookAtTarget(const FVector& TargetLocation);

	// Set looking direction
	UFUNCTION(BlueprintCallable, Category = "NOMI|Animation")
	void SetLookDirection(const FRotator& Direction);

protected:
	// NOMI body mesh (sphere)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NOMI|Components")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	// NOMI face widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NOMI|Components")
	TObjectPtr<UWidgetComponent> FaceWidget;

	// Commentary engine
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NOMI|Components")
	TObjectPtr<UCommentaryEngine> CommentaryEngine;

	// Head rotation component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NOMI|Components")
	TObjectPtr<USceneComponent> HeadRotator;

	// Current expression
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|State")
	ENOMIExpression CurrentExpression = ENOMIExpression::Neutral;

	// Is attached to vehicle?
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|State")
	bool bIsAttached = false;

	// Attached vehicle
	UPROPERTY(BlueprintReadOnly, Category = "NOMI|State")
	TObjectPtr<AActor> AttachedVehicle;

	// Idle blink timer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NOMI|Animation")
	float BlinkInterval = 4.0f;

	// Head rotation speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NOMI|Animation")
	float HeadRotationSpeed = 2.0f;

private:
	// Update expression based on commentary engine emotion
	void UpdateExpressionFromEmotion();

	// Update idle animations
	void UpdateIdleAnimations(float DeltaTime);

	// Blink timer
	float BlinkTimer = 0.0f;

	// Is currently blinking?
	bool bIsBlinking = false;

	// Blink duration
	float BlinkDuration = 0.15f;

	// Current blink progress
	float BlinkProgress = 0.0f;

	// Target look rotation
	FRotator TargetLookRotation = FRotator::ZeroRotator;

	// Current look rotation
	FRotator CurrentLookRotation = FRotator::ZeroRotator;
};
