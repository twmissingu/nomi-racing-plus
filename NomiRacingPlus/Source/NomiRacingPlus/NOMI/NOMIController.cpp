// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NOMIController.h"
#include "NOMIFaceWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "NomiRacingPlus.h"

ANOMIController::ANOMIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create head rotator (for head movement)
	HeadRotator = CreateDefaultSubobject<USceneComponent>(TEXT("HeadRotator"));
	RootComponent = HeadRotator;

	// Create body mesh (sphere) using built-in engine sphere
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(HeadRotator);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeScale3D(FVector(0.3f)); // 30cm diameter sphere
	BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 15.0f)); // Raised above base

	// Load built-in engine sphere mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshFinder.Succeeded())
	{
		BodyMesh->SetStaticMesh(SphereMeshFinder.Object);
	}

	// Create a white material for the body
	static ConstructorHelpers::FObjectFinder<UMaterial> WhiteMaterialFinder(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (WhiteMaterialFinder.Succeeded())
	{
		BodyMesh->SetMaterial(0, WhiteMaterialFinder.Object);
	}

	// Create base mesh (small cylinder)
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(HeadRotator);
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BaseMesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.05f)); // Small flat cylinder

	// Load built-in engine cylinder mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshFinder(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMeshFinder.Succeeded())
	{
		BaseMesh->SetStaticMesh(CylinderMeshFinder.Object);
	}

	// Use same white material for base
	if (WhiteMaterialFinder.Succeeded())
	{
		BaseMesh->SetMaterial(0, WhiteMaterialFinder.Object);
	}

	// Create face widget component (attached to front of sphere)
	FaceWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("FaceWidget"));
	FaceWidget->SetupAttachment(BodyMesh);
	FaceWidget->SetDrawAtDesiredSize(true);
	FaceWidget->SetWidgetSpace(EWidgetSpace::Screen);
	FaceWidget->SetRelativeLocation(FVector(30.0f, 0.0f, 0.0f)); // Front of sphere
	FaceWidget->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f)); // Face forward
	FaceWidget->SetPivot(FVector2D(0.5f, 0.5f)); // Center pivot

	// Create audio component for NOMI voice
	NOMIAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("NOMIAudio"));
	NOMIAudioComponent->SetupAttachment(HeadRotator);
	NOMIAudioComponent->bAutoActivate = false;

	// Create commentary engine
	CommentaryEngine = CreateDefaultSubobject<UCommentaryEngine>(TEXT("CommentaryEngine"));
}

void ANOMIController::BeginPlay()
{
	Super::BeginPlay();

	// Set up face widget with fallback to default if Blueprint-assigned type mismatches
	if (FaceWidget)
	{
		UNOMIFaceWidget* FaceWidgetInstance = Cast<UNOMIFaceWidget>(FaceWidget->GetWidget());
		if (!FaceWidgetInstance)
		{
			// Fallback: create a default UNOMIFaceWidget programmatically
			UE_LOG(LogNomiNOMI, Log, TEXT("Creating default UNOMIFaceWidget as fallback"));
			FaceWidgetInstance = CreateWidget<UNOMIFaceWidget>(GetWorld());
			if (FaceWidgetInstance)
			{
				FaceWidget->SetWidget(FaceWidgetInstance);
			}
			else
			{
				UE_LOG(LogNomiNOMI, Warning, TEXT("NOMI face widget is not UNOMIFaceWidget type and fallback creation failed"));
			}
		}

		if (FaceWidgetInstance)
		{
			// Initialize with neutral expression
			FaceWidgetInstance->UpdateExpression(ENOMIExpression::Neutral);
			UE_LOG(LogNomiNOMI, Log, TEXT("NOMI face widget initialized"));
		}
	}

	// Set up dynamic material for body (black matte)
	if (BodyMesh && BodyMesh->GetMaterial(0))
	{
		UMaterialInstanceDynamic* DynamicMat = BodyMesh->CreateDynamicMaterialInstance(0);
		if (DynamicMat)
		{
			// Set black color with slight metallic sheen
			DynamicMat->SetVectorParameterValue(FName(TEXT("BaseColor")), FLinearColor(0.05f, 0.05f, 0.05f, 1.0f));
			DynamicMat->SetScalarParameterValue(FName(TEXT("Roughness")), 0.6f);
			DynamicMat->SetScalarParameterValue(FName(TEXT("Metallic")), 0.3f);
		}
	}

	// Set up dynamic material for base (also black)
	if (BaseMesh && BaseMesh->GetMaterial(0))
	{
		UMaterialInstanceDynamic* DynamicBaseMat = BaseMesh->CreateDynamicMaterialInstance(0);
		if (DynamicBaseMat)
		{
			DynamicBaseMat->SetVectorParameterValue(FName(TEXT("BaseColor")), FLinearColor(0.05f, 0.05f, 0.05f, 1.0f));
			DynamicBaseMat->SetScalarParameterValue(FName(TEXT("Roughness")), 0.6f);
			DynamicBaseMat->SetScalarParameterValue(FName(TEXT("Metallic")), 0.3f);
		}
	}

	FString MeshName = TEXT("None");
	if (BodyMesh && BodyMesh->GetStaticMesh())
	{
		MeshName = BodyMesh->GetStaticMesh()->GetName();
	}
	UE_LOG(LogNomiNOMI, Log, TEXT("NOMI Controller initialized - Body mesh: %s, Face widget: %s"),
		*MeshName,
		FaceWidget ? TEXT("Set") : TEXT("None"));
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

	// Update face widget
	if (FaceWidget)
	{
		UNOMIFaceWidget* FaceWidgetInstance = Cast<UNOMIFaceWidget>(FaceWidget->GetWidget());
		if (FaceWidgetInstance)
		{
			FaceWidgetInstance->UpdateExpression(Expression);
		}
	}

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
	if (FaceWidget)
	{
		UNOMIFaceWidget* FaceWidgetInstance = Cast<UNOMIFaceWidget>(FaceWidget->GetWidget());
		if (FaceWidgetInstance)
		{
			FaceWidgetInstance->SetEyeOpenness(Shape, Shape);
		}
	}
}

void ANOMIController::SetMouthOpen(float OpenAmount)
{
	// Update mouth open amount for speaking animation
	if (FaceWidget)
	{
		UNOMIFaceWidget* FaceWidgetInstance = Cast<UNOMIFaceWidget>(FaceWidget->GetWidget());
		if (FaceWidgetInstance)
		{
			FaceWidgetInstance->SetMouthOpen(OpenAmount);
		}
	}
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
