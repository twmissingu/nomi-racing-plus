// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Race/CheckpointSystem.h"
#include "Race/RaceManager.h"
#include "Kismet/GameplayStatics.h"
#include "NomiRacingPlus.h"

// ACheckpoint Implementation

ACheckpoint::ACheckpoint()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create trigger volume
	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	RootComponent = TriggerVolume;
	TriggerVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerVolume->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f));
	TriggerVolume->SetGenerateOverlapEvents(true);

	// Create optional visual mesh
	CheckpointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CheckpointMesh"));
	CheckpointMesh->SetupAttachment(RootComponent);
	CheckpointMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CheckpointMesh->SetVisibility(false); // Hidden by default, show via Blueprint
}

void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();

	// Cache RaceManager reference for performance
	CachedRaceManager = Cast<ARaceManager>(UGameplayStatics::GetActorOfClass(this, ARaceManager::StaticClass()));

	// Bind overlap event
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnOverlapBegin);
}

void ACheckpoint::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bIsActive)
	{
		return;
	}

	// Check if overlapping actor is a vehicle
	APawn* VehiclePawn = Cast<APawn>(OtherActor);
	if (!VehiclePawn)
	{
		return;
	}

	// Use cached RaceManager reference (cached in BeginPlay for performance)
	ARaceManager* RaceManager = CachedRaceManager.Get();
	if (!RaceManager)
	{
		return;
	}

	// Process checkpoint based on type
	switch (CheckpointType)
	{
	case ECheckpointType::FinishLine:
		RaceManager->RacerCrossFinishLine(VehiclePawn);
		break;

	case ECheckpointType::Regular:
	case ECheckpointType::Sector:
	case ECheckpointType::StartLine:
		RaceManager->RacerPassCheckpoint(VehiclePawn, CheckpointIndex);
		break;
	}

	UE_LOG(LogNomiRace, Verbose, TEXT("Checkpoint %d triggered by %s"),
		CheckpointIndex, *VehiclePawn->GetName());
}

// ACheckpointTrack Implementation

ACheckpointTrack::ACheckpointTrack()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACheckpointTrack::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoDiscoverCheckpoints)
	{
		DiscoverCheckpoints();
	}
}

ACheckpoint* ACheckpointTrack::GetCheckpoint(int32 Index) const
{
	if (Index >= 0 && Index < Checkpoints.Num())
	{
		return Checkpoints[Index];
	}
	return nullptr;
}

ACheckpoint* ACheckpointTrack::GetFinishLine() const
{
	for (ACheckpoint* Checkpoint : Checkpoints)
	{
		if (Checkpoint && Checkpoint->CheckpointType == ECheckpointType::FinishLine)
		{
			return Checkpoint;
		}
	}
	return nullptr;
}

ACheckpoint* ACheckpointTrack::GetStartLine() const
{
	for (ACheckpoint* Checkpoint : Checkpoints)
	{
		if (Checkpoint && Checkpoint->CheckpointType == ECheckpointType::StartLine)
		{
			return Checkpoint;
		}
	}
	return nullptr;
}

void ACheckpointTrack::DiscoverCheckpoints()
{
	// Find all checkpoints in the level
	TArray<AActor*> FoundCheckpoints;
	UGameplayStatics::GetAllActorsOfClass(this, ACheckpoint::StaticClass(), FoundCheckpoints);

	// Sort by checkpoint index
	FoundCheckpoints.Sort([](const AActor& A, const AActor& B) {
		const ACheckpoint* CheckpointA = Cast<ACheckpoint>(&A);
		const ACheckpoint* CheckpointB = Cast<ACheckpoint>(&B);

		if (CheckpointA && CheckpointB)
		{
			return CheckpointA->CheckpointIndex < CheckpointB->CheckpointIndex;
		}
		return false;
	});

	// Add to array
	Checkpoints.Empty();
	for (AActor* Actor : FoundCheckpoints)
	{
		if (ACheckpoint* Checkpoint = Cast<ACheckpoint>(Actor))
		{
			Checkpoints.Add(Checkpoint);
		}
	}

	UE_LOG(LogNomiRace, Log, TEXT("CheckpointTrack discovered %d checkpoints"), Checkpoints.Num());
}
