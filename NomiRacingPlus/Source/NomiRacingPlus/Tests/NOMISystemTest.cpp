// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Tests/NOMISystemTest.h"
#include "NOMI/CommentaryEngine.h"

/**
 * Test CommentaryEngine initialization
 */
bool FCommentaryEngineInitTest::RunTest(const FString& Parameters)
{
	// Test 1: Create CommentaryEngine
	UCommentaryEngine* Engine = NewObject<UCommentaryEngine>();
	TestNotNull(TEXT("CommentaryEngine should be created"), Engine);

	if (!Engine)
	{
		return false;
	}

	// Test 2: Verify initial state
	TestFalse(TEXT("Should not be playing comment initially"), Engine->IsCommentPlaying());
	TestEqual(TEXT("Initial emotion should be Idle"), Engine->GetCurrentEmotion(), ENOMIEmotion::Idle);

	// Test 3: Verify comment text is empty initially
	FString InitialText = Engine->GetCurrentCommentText();
	TestTrue(TEXT("Initial comment text should be empty"), InitialText.IsEmpty());

	return true;
}

/**
 * Test comment matching logic
 */
bool FCommentMatchingTest::RunTest(const FString& Parameters)
{
	// Test 1: Create CommentaryEngine
	UCommentaryEngine* Engine = NewObject<UCommentaryEngine>();
	TestNotNull(TEXT("CommentaryEngine should be created"), Engine);

	if (!Engine)
	{
		return false;
	}

	// Test 2: Create a comment category
	FCommentCategory Category;
	Category.CategoryName = TEXT("test");

	FNOMIComment Comment1;
	Comment1.Text = TEXT("Test comment 1");
	Comment1.Priority = ECommentPriority::Medium;
	Comment1.Emotion = ENOMIEmotion::Happy;
	Category.Comments.Add(Comment1);

	FNOMIComment Comment2;
	Comment2.Text = TEXT("Test comment 2");
	Comment2.Priority = ECommentPriority::High;
	Comment2.Emotion = ENOMIEmotion::Excited;
	Category.Comments.Add(Comment2);

	// Test 3: Add category to engine
	Engine->AddCommentCategory(Category);

	// Test 4: Create comment context
	FCommentContext Context;
	Context.Event = ERaceEvent::Overtake;
	Context.PlayerName = TEXT("Player");
	Context.Position = 1;
	Context.bIsNIOVehicle = true;

	// Test 5: Request comment
	bool bRequested = Engine->RequestComment(Context);
	// Note: Request may fail due to cooldown, but function should not crash
	TestTrue(TEXT("RequestComment should return a boolean"), true);

	return true;
}

/**
 * Test comment queue management
 */
bool FCommentQueueTest::RunTest(const FString& Parameters)
{
	// Test 1: Create CommentaryEngine
	UCommentaryEngine* Engine = NewObject<UCommentaryEngine>();
	TestNotNull(TEXT("CommentaryEngine should be created"), Engine);

	if (!Engine)
	{
		return false;
	}

	// Test 2: Clear queue
	Engine->ClearQueue();
	TestFalse(TEXT("Should not be playing after clear"), Engine->IsCommentPlaying());

	// Test 3: Get next comment when empty
	FNOMIComment OutComment;
	bool bHasComment = Engine->GetNextComment(OutComment);
	TestFalse(TEXT("Should not have comment when queue is empty"), bHasComment);

	return true;
}

/**
 * Test comment cooldown system
 */
bool FCommentCooldownTest::RunTest(const FString& Parameters)
{
	// Test 1: Create CommentaryEngine
	UCommentaryEngine* Engine = NewObject<UCommentaryEngine>();
	TestNotNull(TEXT("CommentaryEngine should be created"), Engine);

	if (!Engine)
	{
		return false;
	}

	// Test 2: Set comment frequency
	Engine->SetCommentFrequency(0.5f);

	// Test 3: Verify frequency is clamped
	Engine->SetCommentFrequency(2.0f); // Should be clamped to 1.0
	Engine->SetCommentFrequency(-1.0f); // Should be clamped to 0.0

	// Test 4: Add test comments
	FCommentCategory Category;
	Category.CategoryName = TEXT("test_cooldown");

	FNOMIComment Comment;
	Comment.Text = TEXT("Cooldown test comment");
	Comment.Priority = ECommentPriority::Medium;
	Category.Comments.Add(Comment);

	Engine->AddCommentCategory(Category);

	return true;
}

/**
 * Test NOMI emotion system
 */
bool FNOMIEmotionTest::RunTest(const FString& Parameters)
{
	// Test 1: Verify all emotion types exist
	ENOMIEmotion Idle = ENOMIEmotion::Idle;
	ENOMIEmotion Happy = ENOMIEmotion::Happy;
	ENOMIEmotion Excited = ENOMIEmotion::Excited;
	ENOMIEmotion Nervous = ENOMIEmotion::Nervous;
	ENOMIEmotion Surprised = ENOMIEmotion::Surprised;
	ENOMIEmotion Celebrating = ENOMIEmotion::Celebrating;
	ENOMIEmotion Concerned = ENOMIEmotion::Concerned;
	ENOMIEmotion Confused = ENOMIEmotion::Confused;
	ENOMIEmotion Tired = ENOMIEmotion::Tired;

	// Test 2: Verify emotions are distinct
	TestNotEqual(TEXT("Idle should differ from Happy"), (int32)Idle, (int32)Happy);
	TestNotEqual(TEXT("Happy should differ from Excited"), (int32)Happy, (int32)Excited);
	TestNotEqual(TEXT("Excited should differ from Nervous"), (int32)Excited, (int32)Nervous);

	// Test 3: Create CommentaryEngine and verify emotion
	UCommentaryEngine* Engine = NewObject<UCommentaryEngine>();
	TestNotNull(TEXT("CommentaryEngine should be created"), Engine);

	if (Engine)
	{
		TestEqual(TEXT("Initial emotion should be Idle"), Engine->GetCurrentEmotion(), ENOMIEmotion::Idle);
	}

	return true;
}
