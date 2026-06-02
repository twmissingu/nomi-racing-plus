// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NOMI/NOMIController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "NomiRacingPlus.h"

ANOMIController::ANOMIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create head rotator (for head movement)
	HeadRotator = CreateDefaultSubobject<USceneComponent>(TEXT("HeadRotator"));
	RootComponent = HeadRotator;

	// Create body mesh (sphere)
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(HeadRotator);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create face widget component
	FaceWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FaceWidget"));
	FaceWidget->SetupAttachment(BodyMesh);
	FaceWidget->SetDrawAtDesiredSize(true);
	FaceWidget->SetWidgetSpace(EWidgetSpace::Screen);

	// Create commentary engine
	CommentaryEngine = CreateDefaultSubobject<UCommentaryEngine>(TEXT("CommentaryEngine"));
}

void ANOMIController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogNomiNOMI, Log, TEXT("NOMI Controller initialized"));
}

void ANOMIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update expression based on commentary engine
	UpdateExpressionFromEmotion();

	// Update idle animations
	UpdateIdleAnimations(DeltaTime);

	// Update head rotation
	CurrentLookRotation = FMath::RInterpTo(CurrentLookRotation, TargetLookRotation,
		DeltaTime, HeadRotationSpeed);

	if (HeadRotator)
	{
		HeadRotator->SetRelativeRotation(CurrentLookRotation);
	}
}

void ANOMIController::AttachToVehicle(AActor* Vehicle)
{
	if (!Vehicle)
	{
		return;
	}

	// Attach to vehicle
	AttachToActor(Vehicle, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	AttachedVehicle = Vehicle;
	bIsAttached = true;

	UE_LOG(LogNomiNOMI, Log, TEXT("NOMI attached to vehicle: %s"), *Vehicle->GetName());
}

void ANOMIController::DetachFromVehicle()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	AttachedVehicle = nullptr;
	bIsAttached = false;

	UE_LOG(LogNomiNOMI, Log, TEXT("NOMI detached from vehicle"));
}

void ANOMIController::SetExpression(ENOMIExpression Expression)
{
	CurrentExpression = Expression;

	// Update face widget (would be done via Blueprint in actual implementation)
	UE_LOG(LogNomiNOMI, Verbose, TEXT("NOMI expression changed: %d"), (int32)Expression);
}

void ANOMIController::PlayBlink()
{
	if (!bIsBlinking)
	{
		bIsBlinking = true;
		BlinkProgress = 0.0f;
	}
}

void ANOMIController::SetEyeShape(float Shape)
{
	// Update eye shape in face widget
	// This would typically drive a material parameter or widget animation
}

void ANOMIController::SetMouthOpen(float OpenAmount)
{
	// Update mouth open amount for speaking animation
	// This would typically drive a material parameter or widget animation
}

void ANOMIController::LookAtTarget(const FVector& TargetLocation)
{
	if (!HeadRotator)
	{
		return;
	}

	FVector Direction = TargetLocation - GetActorLocation();
	TargetLookRotation = Direction.Rotation();
}

void ANOMIController::SetLookDirection(const FRotator& Direction)
{
	TargetLookRotation = Direction;
}

void ANOMIController::UpdateExpressionFromEmotion()
{
	if (!CommentaryEngine)
	{
		return;
	}

	ENOMIEmotion Emotion = CommentaryEngine->GetCurrentEmotion();

	// Map emotion to expression
	ENOMIExpression NewExpression = CurrentExpression;

	switch (Emotion)
	{
	case ENOMIEmotion::Idle:
		NewExpression = ENOMIExpression::Neutral;
		break;

	case ENOMIEmotion::Happy:
		NewExpression = ENOMIExpression::Happy;
		break;

	case ENOMIEmotion::Excited:
		NewExpression = ENOMIExpression::Excited;
		break;

	case ENOMIEmotion::Nervous:
		NewExpression = ENOMIExpression::Curious;
		break;

	case ENOMIEmotion::Surprised:
		NewExpression = ENOMIExpression::Surprised;
		break;

	case ENOMIEmotion::Celebrating:
		NewExpression = ENOMIExpression::Excited;
		break;

	case ENOMIEmotion::Concerned:
		NewExpression = ENOMIExpression::Sad;
		break;

	case ENOMIEmotion::Confused:
		NewExpression = ENOMIExpression::Confused;
		break;

	case ENOMIEmotion::Tired:
		NewExpression = ENOMIExpression::Sleepy;
		break;
	}

	if (NewExpression != CurrentExpression)
	{
		SetExpression(NewExpression);
	}

	// Update mouth animation for speaking
	if (CommentaryEngine->IsCommentPlaying())
	{
		// Animate mouth
		UWorld* World = GetWorld();
		if (World)
		{
			float Time = World->GetTimeSeconds();
			float MouthOpen = FMath::Sin(Time * 10.0f) * 0.5f + 0.5f;
			SetMouthOpen(MouthOpen);
		}
	}
	else
	{
		SetMouthOpen(0.0f);
	}
}

void ANOMIController::UpdateIdleAnimations(float DeltaTime)
{
	// Blink timer
	BlinkTimer += DeltaTime;

	if (BlinkTimer >= BlinkInterval)
	{
		BlinkTimer = 0.0f;
		PlayBlink();
	}

	// Blink animation
	if (bIsBlinking)
	{
		BlinkProgress += DeltaTime / BlinkDuration;

		if (BlinkProgress >= 1.0f)
		{
			bIsBlinking = false;
			BlinkProgress = 0.0f;
			SetEyeShape(1.0f); // Fully open
		}
		else
		{
			// Blink curve: 1 -> 0 -> 1
			float BlinkCurve = 1.0f - FMath::Abs(FMath::Sin(BlinkProgress * PI));
			SetEyeShape(BlinkCurve);
		}
	}

	// Look at driver (if attached to vehicle)
	if (bIsAttached && AttachedVehicle)
	{
		// Look slightly upward and toward driver position
		FVector DriverPos = AttachedVehicle->GetActorLocation() +
			AttachedVehicle->GetActorRightVector() * 50.0f +
			AttachedVehicle->GetActorUpVector() * 100.0f;

		LookAtTarget(DriverPos);
	}
}
