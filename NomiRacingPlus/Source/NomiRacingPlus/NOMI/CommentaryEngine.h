// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Race/RaceManager.h"
#include "Vehicles/VehicleStateManager.h"
#include "CommentaryEngine.generated.h"

/**
 * NOMI emotion states
 */
UENUM(BlueprintType)
enum class ENOMIEmotion : uint8
{
	Idle         UMETA(DisplayName = "Idle"),
	Happy        UMETA(DisplayName = "Happy"),
	Excited      UMETA(DisplayName = "Excited"),
	Nervous      UMETA(DisplayName = "Nervous"),
	Surprised    UMETA(DisplayName = "Surprised"),
	Celebrating  UMETA(DisplayName = "Celebrating"),
	Concerned    UMETA(DisplayName = "Concerned"),
	Confused     UMETA(DisplayName = "Confused"),
	Tired        UMETA(DisplayName = "Tired")
};

/**
 * Comment priority levels
 */
UENUM(BlueprintType)
enum class ECommentPriority : uint8
{
	Low          UMETA(DisplayName = "Low"),
	Medium       UMETA(DisplayName = "Medium"),
	High         UMETA(DisplayName = "High"),
	Critical     UMETA(DisplayName = "Critical")
};

/**
 * Single comment entry
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FNOMIComment
{
	GENERATED_BODY()

	// Comment text with placeholders: {player}, {rival}, {position}, {corner}, {duration}, {lap}, {time}
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comment")
	FString Text;

	// Priority level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comment")
	ECommentPriority Priority = ECommentPriority::Medium;

	// Associated emotion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comment")
	ENOMIEmotion Emotion = ENOMIEmotion::Happy;

	// Minimum display duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comment")
	float MinDisplayDuration = 2.0f;

	// Maximum display duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comment")
	float MaxDisplayDuration = 5.0f;

	// Is this a NIO-specific comment?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comment")
	bool bIsNIOSpecific = false;

	// Is this a frustration comfort comment?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comment")
	bool bIsComfortComment = false;
};

/**
 * Comment category for organizing the comment pool
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FCommentCategory
{
	GENERATED_BODY()

	// Category name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category")
	FString CategoryName;

	// Comments in this category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category")
	TArray<FNOMIComment> Comments;
};

/**
 * Comment context for matching
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FCommentContext
{
	GENERATED_BODY()

	// Event type
	UPROPERTY(BlueprintReadOnly, Category = "Context")
	ERaceEvent Event = ERaceEvent::Overtake;

	// Player name
	UPROPERTY(BlueprintReadOnly, Category = "Context")
	FString PlayerName;

	// Rival name (for overtakes)
	UPROPERTY(BlueprintReadOnly, Category = "Context")
	FString RivalName;

	// Current position
	UPROPERTY(BlueprintReadOnly, Category = "Context")
	int32 Position = 0;

	// Current lap
	UPROPERTY(BlueprintReadOnly, Category = "Context")
	int32 CurrentLap = 0;

	// Lap time
	UPROPERTY(BlueprintReadOnly, Category = "Context")
	float LapTime = 0.0f;

	// Drift duration
	UPROPERTY(BlueprintReadOnly, Category = "Context")
	float DriftDuration = 0.0f;

	// Speed (km/h)
	UPROPERTY(BlueprintReadOnly, Category = "Context")
	float Speed = 0.0f;

	// Corner name (if applicable)
	UPROPERTY(BlueprintReadOnly, Category = "Context")
	FString CornerName;

	// Is player in NIO vehicle?
	UPROPERTY(BlueprintReadOnly, Category = "Context")
	bool bIsNIOVehicle = false;

	// NIO vehicle type
	UPROPERTY(BlueprintReadOnly, Category = "Context")
	ENIOVehicleType NIOVehicleType = ENIOVehicleType::EP9;
};

/**
 * Comment queue entry with timing
 */
USTRUCT()
struct FQueuedComment
{
	GENERATED_BODY()

	FNOMIComment Comment;
	float DisplayStartTime = 0.0f;
	float DisplayDuration = 0.0f;
	bool bIsPlaying = false;
};

/**
 * Commentary Engine - manages NOMI's comment system
 * Handles comment pool, matching, cooldown, and queue
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UCommentaryEngine : public UActorComponent
{
	GENERATED_BODY()

public:
	UCommentaryEngine();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Load comment pool from JSON
	UFUNCTION(BlueprintCallable, Category = "NOMI|Comments")
	bool LoadCommentPool(const FString& JsonPath);

	// Add comments programmatically
	UFUNCTION(BlueprintCallable, Category = "NOMI|Comments")
	void AddCommentCategory(const FCommentCategory& Category);

	// Request a comment based on context
	UFUNCTION(BlueprintCallable, Category = "NOMI|Comments")
	bool RequestComment(const FCommentContext& Context);

	// Get next comment to display
	UFUNCTION(BlueprintCallable, Category = "NOMI|Comments")
	bool GetNextComment(FNOMIComment& OutComment);

	// Is a comment currently playing?
	UFUNCTION(BlueprintCallable, Category = "NOMI|Comments")
	bool IsCommentPlaying() const { return CurrentComment.bIsPlaying; }

	// Get current comment text (with variables replaced)
	UFUNCTION(BlueprintCallable, Category = "NOMI|Comments")
	FString GetCurrentCommentText() const;

	// Get current emotion
	UFUNCTION(BlueprintCallable, Category = "NOMI|Comments")
	ENOMIEmotion GetCurrentEmotion() const { return CurrentEmotion; }

	// Set comment frequency
	UFUNCTION(BlueprintCallable, Category = "NOMI|Comments")
	void SetCommentFrequency(float Frequency) { CommentFrequency = FMath::Clamp(Frequency, 0.0f, 1.0f); }

	// Clear comment queue
	UFUNCTION(BlueprintCallable, Category = "NOMI|Comments")
	void ClearQueue();

protected:
	virtual void BeginPlay() override;

	// Comment pool organized by event type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NOMI|Comments")
	TMap<ERaceEvent, FCommentCategory> CommentPool;

	// NIO-specific comments
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NOMI|Comments")
	FCommentCategory NIOComments;

	// Comfort comments for frustration scenarios
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NOMI|Comments")
	FCommentCategory ComfortComments;

	// Comment cooldown (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NOMI|Comments")
	float CommentCooldown = 3.0f;

	// Comment frequency multiplier (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NOMI|Comments")
	float CommentFrequency = 0.7f;

	// Path to default comment pool JSON (configurable via INI or Blueprint)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NOMI|Comments", meta = (FilePathFilter = "json"))
	FFilePath DefaultCommentPoolPath;

	// Maximum queue size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NOMI|Comments")
	int32 MaxQueueSize = 2;

	// Recent comments to avoid repetition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NOMI|Comments")
	int32 RecentCommentsMax = 10;

private:
	// Find matching comment for context (returns value copy for safety)
	TOptional<FNOMIComment> FindMatchingComment(const FCommentContext& Context);

	// Replace variables in comment text
	FString ReplaceVariables(const FString& Text, const FCommentContext& Context) const;

	// Check if comment was recently used
	bool WasRecentlyUsed(const FNOMIComment& Comment) const;

	// Add to recent comments
	void AddToRecentComments(const FNOMIComment& Comment);

	// Update cooldown
	void UpdateCooldown(float DeltaTime);

	// Queue management
	void ProcessQueue(float DeltaTime);

	// Current comment state
	FQueuedComment CurrentComment;
	ENOMIEmotion CurrentEmotion = ENOMIEmotion::Idle;

	// Queue
	TArray<FQueuedComment> CommentQueue;

	// Recent comments (for dedup)
	TArray<FString> RecentComments;

	// Cooldown timer
	float CooldownTimer = 0.0f;

	// Is engine ready?
	bool bIsReady = false;
};
