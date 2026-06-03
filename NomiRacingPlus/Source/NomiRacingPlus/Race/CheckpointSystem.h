// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "CheckpointSystem.generated.h"

/**
 * Checkpoint types
 */
UENUM(BlueprintType)
enum class ECheckpointType : uint8
{
	Regular      UMETA(DisplayName = "Regular"),
	FinishLine   UMETA(DisplayName = "Finish Line"),
	StartLine    UMETA(DisplayName = "Start Line"),
	Sector       UMETA(DisplayName = "Sector")
};

/**
 * Single checkpoint actor
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API ACheckpoint : public AActor
{
	GENERATED_BODY()

public:
	ACheckpoint();

	// Checkpoint index (order in track)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 CheckpointIndex = 0;

	// Checkpoint type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	ECheckpointType CheckpointType = ECheckpointType::Regular;

	// Is this checkpoint active?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool bIsActive = true;

protected:
	virtual void BeginPlay() override;

	// Trigger volume
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
	TObjectPtr<UBoxComponent> TriggerVolume;

	// Visual mesh (optional)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;

	// Collision response
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

private:
	// Cached RaceManager for performance (avoid GetActorOfClass on every overlap)
	TWeakObjectPtr<class ARaceManager> CachedRaceManager;
};

/**
 * Checkpoint track - manages all checkpoints for a track
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API ACheckpointTrack : public AActor
{
	GENERATED_BODY()

public:
	ACheckpointTrack();

	// Get all checkpoints in order
	UFUNCTION(BlueprintCallable, Category = "Checkpoint Track")
	const TArray<ACheckpoint*>& GetCheckpoints() const { return Checkpoints; }

	// Get checkpoint count
	UFUNCTION(BlueprintCallable, Category = "Checkpoint Track")
	int32 GetCheckpointCount() const { return Checkpoints.Num(); }

	// Get checkpoint by index
	UFUNCTION(BlueprintCallable, Category = "Checkpoint Track")
	ACheckpoint* GetCheckpoint(int32 Index) const;

	// Get finish line checkpoint
	UFUNCTION(BlueprintCallable, Category = "Checkpoint Track")
	ACheckpoint* GetFinishLine() const;

	// Get start line checkpoint
	UFUNCTION(BlueprintCallable, Category = "Checkpoint Track")
	ACheckpoint* GetStartLine() const;

protected:
	virtual void BeginPlay() override;

	// All checkpoints in order
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint Track")
	TArray<TObjectPtr<ACheckpoint>> Checkpoints;

	// Auto-discover checkpoints in level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint Track")
	bool bAutoDiscoverCheckpoints = true;

private:
	// Find and register checkpoints in the level
	void DiscoverCheckpoints();
};
