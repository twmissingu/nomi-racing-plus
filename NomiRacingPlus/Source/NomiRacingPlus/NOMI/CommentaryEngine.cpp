// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "CommentaryEngine.h"
#include "Vehicles/VehicleStateManager.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "NomiRacingPlus.h"
#include "Core/NomiErrorHandler.h"

UCommentaryEngine::UCommentaryEngine()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCommentaryEngine::BeginPlay()
{
	Super::BeginPlay();

	// Try to load default comment pool from configurable path
	FString DefaultPath = DefaultCommentPoolPath.FilePath;
	if (DefaultPath.IsEmpty())
	{
		DefaultPath = FPaths::ProjectContentDir() + TEXT("NOMI/Comments/DefaultComments.json");
	}
	if (FPaths::FileExists(DefaultPath))
	{
		LoadCommentPool(DefaultPath);
	}
	else
	{
		UE_LOG(LogNomiNOMI, Warning, TEXT("Default comment pool not found at: %s"), *DefaultPath);
	}

	bIsReady = true;
	UE_LOG(LogNomiNOMI, Log, TEXT("Commentary Engine initialized"));
}

void UCommentaryEngine::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateCooldown(DeltaTime);
	ProcessQueue(DeltaTime);
}

bool UCommentaryEngine::LoadCommentPool(const FString& JsonPath)
{
	// Load JSON file
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *JsonPath))
	{
		NomiError::Log(ENomiErrorSeverity::Error, TEXT("NOMI"), FString::Printf(TEXT("Failed to load comment pool: %s"), *JsonPath));
		return false;
	}

	// Parse JSON
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		NomiError::Log(ENomiErrorSeverity::Error, TEXT("NOMI"), TEXT("Failed to parse comment pool JSON"));
		return false;
	}

	// Parse each category
	for (const auto& Pair : JsonObject->Values)
	{
		FString CategoryName = Pair.Key;
		const TSharedPtr<FJsonObject>* CategoryObj;

		if (Pair.Value->TryGetObject(CategoryObj))
		{
			FCommentCategory Category;
			Category.CategoryName = CategoryName;

			// Parse subcategories
			for (const auto& SubPair : (*CategoryObj)->Values)
			{
				const TArray<TSharedPtr<FJsonValue>>* CommentArray;
				if (SubPair.Value->TryGetArray(CommentArray))
				{
					for (const auto& CommentValue : *CommentArray)
					{
						FString CommentText;
						if (CommentValue->TryGetString(CommentText))
						{
							FNOMIComment Comment;
							Comment.Text = CommentText;

							// Determine emotion based on category
							if (CategoryName == "overtake")
							{
								Comment.Emotion = ENOMIEmotion::Excited;
								Comment.Priority = ECommentPriority::High;
							}
							else if (CategoryName == "drift")
							{
								Comment.Emotion = ENOMIEmotion::Excited;
								Comment.Priority = ECommentPriority::High;
							}
							else if (CategoryName == "collision")
							{
								Comment.Emotion = ENOMIEmotion::Concerned;
								Comment.Priority = ECommentPriority::Medium;
							}
							else if (CategoryName == "nio_specific")
							{
								Comment.bIsNIOSpecific = true;
								Comment.Emotion = ENOMIEmotion::Happy;
							}

							Category.Comments.Add(Comment);
						}
					}
				}
			}

			// Map to event type
			if (CategoryName == "overtake")
			{
				CommentPool.Add(ERaceEvent::Overtake, Category);
			}
			else if (CategoryName == "overtaken")
			{
				CommentPool.Add(ERaceEvent::Overtaken, Category);
			}
			else if (CategoryName == "drift")
			{
				CommentPool.Add(ERaceEvent::DriftStart, Category);
			}
			else if (CategoryName == "lap_complete")
			{
				CommentPool.Add(ERaceEvent::LapComplete, Category);
			}
			else if (CategoryName == "fastest_lap")
			{
				CommentPool.Add(ERaceEvent::FastestLap, Category);
			}
			else if (CategoryName == "high_speed")
			{
				CommentPool.Add(ERaceEvent::HighSpeed, Category);
			}
			else if (CategoryName == "collision")
			{
				CommentPool.Add(ERaceEvent::Collision, Category);
			}
			else if (CategoryName == "first_place")
			{
				CommentPool.Add(ERaceEvent::FirstPlace, Category);
			}
			else if (CategoryName == "last_place")
			{
				CommentPool.Add(ERaceEvent::LastPlace, Category);
			}
			else if (CategoryName == "nio_specific")
			{
				NIOComments = Category;
			}
			else if (CategoryName == "comfort")
			{
				ComfortComments = Category;
			}
		}
	}

	UE_LOG(LogNomiNOMI, Log, TEXT("Loaded comment pool with %d categories"), CommentPool.Num());
	return true;
}

void UCommentaryEngine::AddCommentCategory(const FCommentCategory& Category)
{
	// Add to appropriate pool based on category name
	if (Category.CategoryName == "nio_specific")
	{
		NIOComments = Category;
	}
	else if (Category.CategoryName == "comfort")
	{
		ComfortComments = Category;
	}
}

bool UCommentaryEngine::RequestComment(const FCommentContext& Context)
{
	if (!bIsReady)
	{
		return false;
	}

	// Check cooldown
	if (CooldownTimer > 0.0f)
	{
		return false;
	}

	// Check queue capacity
	if (CommentQueue.Num() >= MaxQueueSize)
	{
		return false;
	}

	// Find matching comment (value copy for safety)
	TOptional<FNOMIComment> Comment = FindMatchingComment(Context);
	if (!Comment.IsSet())
	{
		return false;
	}

	// Create queued comment
	FQueuedComment QueuedComment;
	QueuedComment.Comment = Comment.GetValue();
	QueuedComment.DisplayDuration = FMath::RandRange(Comment.GetValue().MinDisplayDuration, Comment.GetValue().MaxDisplayDuration);

	// Replace variables
	QueuedComment.Comment.Text = ReplaceVariables(QueuedComment.Comment.Text, Context);

	// Add to queue
	CommentQueue.Add(QueuedComment);

	// Update emotion
	CurrentEmotion = Comment.GetValue().Emotion;

	// Add to recent comments
	AddToRecentComments(Comment.GetValue());

	// Reset cooldown
	CooldownTimer = CommentCooldown;

	UE_LOG(LogNomiNOMI, Verbose, TEXT("Comment requested: %s"), *QueuedComment.Comment.Text);
	return true;
}

bool UCommentaryEngine::GetNextComment(FNOMIComment& OutComment)
{
	if (CurrentComment.bIsPlaying)
	{
		OutComment = CurrentComment.Comment;
		return true;
	}

	if (CommentQueue.Num() > 0)
	{
		CurrentComment = CommentQueue[0];
		CommentQueue.RemoveAt(0);
		CurrentComment.bIsPlaying = true;

		UWorld* World = GetWorld();
		CurrentComment.DisplayStartTime = World ? World->GetTimeSeconds() : 0.0f;

		OutComment = CurrentComment.Comment;
		return true;
	}

	return false;
}

FString UCommentaryEngine::GetCurrentCommentText() const
{
	if (CurrentComment.bIsPlaying)
	{
		return CurrentComment.Comment.Text;
	}
	return FString();
}

void UCommentaryEngine::ClearQueue()
{
	CommentQueue.Empty();
	CurrentComment = FQueuedComment();
	CurrentEmotion = ENOMIEmotion::Idle;
}

TOptional<FNOMIComment> UCommentaryEngine::FindMatchingComment(const FCommentContext& Context)
{
	// First check event-specific comments
	FCommentCategory* Category = CommentPool.Find(Context.Event);
	if (Category && Category->Comments.Num() > 0)
	{
		// Filter by NIO vehicle if applicable
		TArray<int32> ValidIndices;
		for (int32 i = 0; i < Category->Comments.Num(); i++)
		{
			const FNOMIComment& Comment = Category->Comments[i];
			if (Comment.bIsNIOSpecific && !Context.bIsNIOVehicle)
			{
				continue;
			}
			if (WasRecentlyUsed(Comment))
			{
				continue;
			}
			ValidIndices.Add(i);
		}

		if (ValidIndices.Num() > 0)
		{
			// Random selection with frequency check
			if (FMath::FRand() <= CommentFrequency)
			{
				return Category->Comments[ValidIndices[FMath::RandRange(0, ValidIndices.Num() - 1)]];
			}
		}
	}

	// Check NIO-specific comments for NIO vehicles
	if (Context.bIsNIOVehicle && NIOComments.Comments.Num() > 0)
	{
		if (FMath::FRand() <= CommentFrequency * 0.5f)
		{
			int32 Index = FMath::RandRange(0, NIOComments.Comments.Num() - 1);
			return NIOComments.Comments[Index];
		}
	}

	// Check comfort comments for frustration scenarios
	if (Context.Event == ERaceEvent::LastPlace || Context.Event == ERaceEvent::Overtaken)
	{
		if (ComfortComments.Comments.Num() > 0)
		{
			if (FMath::FRand() <= 0.7f) // Higher chance for comfort
			{
				int32 Index = FMath::RandRange(0, ComfortComments.Comments.Num() - 1);
				return ComfortComments.Comments[Index];
			}
		}
	}

	return TOptional<FNOMIComment>();
}

FString UCommentaryEngine::ReplaceVariables(const FString& Text, const FCommentContext& Context) const
{
	FString Result = Text;

	Result.ReplaceInline(TEXT("{player}"), *Context.PlayerName);
	Result.ReplaceInline(TEXT("{rival}"), *Context.RivalName);
	Result.ReplaceInline(TEXT("{position}"), *FString::FromInt(Context.Position));
	Result.ReplaceInline(TEXT("{lap}"), *FString::FromInt(Context.CurrentLap));
	Result.ReplaceInline(TEXT("{time}"), *FString::Printf(TEXT("%.2f"), Context.LapTime));
	Result.ReplaceInline(TEXT("{duration}"), *FString::Printf(TEXT("%.1f"), Context.DriftDuration));
	Result.ReplaceInline(TEXT("{corner}"), *Context.CornerName);
	Result.ReplaceInline(TEXT("{speed}"), *FString::FromInt(FMath::RoundToInt(Context.Speed)));

	return Result;
}

bool UCommentaryEngine::WasRecentlyUsed(const FNOMIComment& Comment) const
{
	return RecentComments.Contains(Comment.Text);
}

void UCommentaryEngine::AddToRecentComments(const FNOMIComment& Comment)
{
	RecentComments.Add(Comment.Text);

	// Remove oldest if exceeding max (remove from front in bulk to avoid repeated O(N) shifts)
	if (RecentComments.Num() > RecentCommentsMax)
	{
		int32 Excess = RecentComments.Num() - RecentCommentsMax;
		RecentComments.RemoveAt(0, Excess);
	}
}

void UCommentaryEngine::UpdateCooldown(float DeltaTime)
{
	if (CooldownTimer > 0.0f)
	{
		CooldownTimer -= DeltaTime;
	}
}

void UCommentaryEngine::ProcessQueue(float DeltaTime)
{
	if (CurrentComment.bIsPlaying)
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			return;
		}

		float ElapsedTime = World->GetTimeSeconds() - CurrentComment.DisplayStartTime;
		if (ElapsedTime >= CurrentComment.DisplayDuration)
		{
			CurrentComment.bIsPlaying = false;
			CurrentEmotion = ENOMIEmotion::Idle;
		}
	}
}
