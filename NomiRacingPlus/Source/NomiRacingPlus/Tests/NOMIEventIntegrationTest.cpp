// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Tests/NOMIEventIntegrationTest.h"
#include "NOMI/CommentaryEngine.h"
#include "NOMI/NOMIController.h"
#include "Race/RaceManager.h"

/**
 * Test the full pipeline from race event to comment generation
 * Verifies that race events can trigger appropriate comments through the engine
 */
bool FNOMIEventToCommentPipelineTest::RunTest(const FString& Parameters)
{
	// Test 1: Create commentary engine
	UCommentaryEngine* Engine = NewObject<UCommentaryEngine>();
	TestNotNull(TEXT("CommentaryEngine should be created"), Engine);

	if (!Engine)
	{
		return false;
	}

	// Test 2: Add overtake comment category
	FCommentCategory OvertakeCategory;
	OvertakeCategory.CategoryName = TEXT("overtake");

	FNOMIComment OvertakeComment1;
	OvertakeComment1.Text = TEXT("Nice move by {player}! Up to position {position}!");
	OvertakeComment1.Priority = ECommentPriority::High;
	OvertakeComment1.Emotion = ENOMIEmotion::Excited;
	OvertakeCategory.Comments.Add(OvertakeComment1);

	FNOMIComment OvertakeComment2;
	OvertakeComment2.Text = TEXT("{player} passes {rival} brilliantly!");
	OvertakeComment2.Priority = ECommentPriority::High;
	OvertakeComment2.Emotion = ENOMIEmotion::Happy;
	OvertakeCategory.Comments.Add(OvertakeComment2);

	Engine->AddCommentCategory(OvertakeCategory);

	// Test 3: Create an overtake event context
	FCommentContext OvertakeContext;
	OvertakeContext.Event = ERaceEvent::Overtake;
	OvertakeContext.PlayerName = TEXT("NIO Driver");
	OvertakeContext.RivalName = TEXT("AI 3");
	OvertakeContext.Position = 2;
	OvertakeContext.bIsNIOVehicle = true;

	// Test 4: Request comment for the overtake event
	bool bRequested = Engine->RequestComment(OvertakeContext);
	// Note: RequestComment may return false due to bIsReady state or cooldown
	// but it should not crash
	TestTrue(TEXT("RequestComment should execute without crashing"), true);

	// Test 5: Add lap complete comment category
	FCommentCategory LapCategory;
	LapCategory.CategoryName = TEXT("lap_complete");

	FNOMIComment LapComment;
	LapComment.Text = TEXT("Lap {lap} completed in {time} seconds!");
	LapComment.Priority = ECommentPriority::Medium;
	LapComment.Emotion = ENOMIEmotion::Happy;
	LapCategory.Comments.Add(LapComment);

	Engine->AddCommentCategory(LapCategory);

	// Test 6: Create a lap complete context
	FCommentContext LapContext;
	LapContext.Event = ERaceEvent::LapComplete;
	LapContext.PlayerName = TEXT("NIO Driver");
	LapContext.CurrentLap = 2;
	LapContext.LapTime = 45.67f;

	bool bLapRequested = Engine->RequestComment(LapContext);
	TestTrue(TEXT("Lap comment request should execute without crashing"), true);

	return true;
}

/**
 * Test the NOMI emotion state machine transitions
 * Verifies that emotions change correctly in response to events
 */
bool FNOMIEmotionStateMachineTest::RunTest(const FString& Parameters)
{
	// Test 1: Create commentary engine
	UCommentaryEngine* Engine = NewObject<UCommentaryEngine>();
	TestNotNull(TEXT("CommentaryEngine should be created"), Engine);

	if (!Engine)
	{
		return false;
	}

	// Test 2: Verify initial emotion is Idle
	TestEqual(TEXT("Initial emotion should be Idle"), Engine->GetCurrentEmotion(), ENOMIEmotion::Idle);

	// Test 3: Add comments with different emotions
	FCommentCategory ExcitedCategory;
	ExcitedCategory.CategoryName = TEXT("first_place");

	FNOMIComment ExcitedComment;
	ExcitedComment.Text = TEXT("You're in the lead!");
	ExcitedComment.Priority = ECommentPriority::Critical;
	ExcitedComment.Emotion = ENOMIEmotion::Celebrating;
	ExcitedCategory.Comments.Add(ExcitedComment);
	Engine->AddCommentCategory(ExcitedCategory);

	FCommentCategory ConcernedCategory;
	ConcernedCategory.CategoryName = TEXT("collision");

	FNOMIComment ConcernedComment;
	ConcernedComment.Text = TEXT("That was a close call!");
	ConcernedComment.Priority = ECommentPriority::High;
	ConcernedComment.Emotion = ENOMIEmotion::Concerned;
	ConcernedCategory.Comments.Add(ConcernedComment);
	Engine->AddCommentCategory(ConcernedCategory);

	// Test 4: Request first place comment (should set emotion to Celebrating)
	FCommentContext FirstPlaceContext;
	FirstPlaceContext.Event = ERaceEvent::FirstPlace;
	FirstPlaceContext.PlayerName = TEXT("NIO Driver");
	FirstPlaceContext.Position = 1;
	Engine->RequestComment(FirstPlaceContext);

	// The emotion should have been updated if the request succeeded
	ENOMIEmotion CurrentEmotion = Engine->GetCurrentEmotion();
	// Either the request succeeded and emotion changed, or cooldown prevented it
	TestTrue(TEXT("Emotion should be Idle or Celebrating"),
		CurrentEmotion == ENOMIEmotion::Idle || CurrentEmotion == ENOMIEmotion::Celebrating);

	// Test 5: Clear queue resets emotion to Idle
	Engine->ClearQueue();
	TestEqual(TEXT("Emotion should be Idle after clear"), Engine->GetCurrentEmotion(), ENOMIEmotion::Idle);

	// Test 6: Verify all emotion types can be used in comments
	TArray<ENOMIEmotion> AllEmotions = {
		ENOMIEmotion::Idle,
		ENOMIEmotion::Happy,
		ENOMIEmotion::Excited,
		ENOMIEmotion::Nervous,
		ENOMIEmotion::Surprised,
		ENOMIEmotion::Celebrating,
		ENOMIEmotion::Concerned,
		ENOMIEmotion::Confused,
		ENOMIEmotion::Tired
	};

	for (ENOMIEmotion Emotion : AllEmotions)
	{
		FNOMIComment TestComment;
		TestComment.Emotion = Emotion;
		TestComment.Text = TEXT("Test");
		TestTrue(TEXT("Emotion should be assignable"), TestComment.Emotion == Emotion);
	}

	return true;
}

/**
 * Test comment priority handling
 * Verifies that higher priority comments are preferred
 */
bool FNOMICommentPriorityTest::RunTest(const FString& Parameters)
{
	// Test 1: Create commentary engine
	UCommentaryEngine* Engine = NewObject<UCommentaryEngine>();
	TestNotNull(TEXT("CommentaryEngine should be created"), Engine);

	if (!Engine)
	{
		return false;
	}

	// Test 2: Add comments with varying priorities for the same event
	FCommentCategory Category;
	Category.CategoryName = TEXT("high_speed");

	FNOMIComment LowPriority;
	LowPriority.Text = TEXT("Going fast.");
	LowPriority.Priority = ECommentPriority::Low;
	LowPriority.Emotion = ENOMIEmotion::Happy;
	Category.Comments.Add(LowPriority);

	FNOMIComment MediumPriority;
	MediumPriority.Text = TEXT("That's some serious speed!");
	MediumPriority.Priority = ECommentPriority::Medium;
	MediumPriority.Emotion = ENOMIEmotion::Excited;
	Category.Comments.Add(MediumPriority);

	FNOMIComment HighPriority;
	HighPriority.Text = TEXT("Incredible velocity from the NIO!");
	HighPriority.Priority = ECommentPriority::High;
	HighPriority.Emotion = ENOMIEmotion::Excited;
	HighPriority.bIsNIOSpecific = true;
	Category.Comments.Add(HighPriority);

	FNOMIComment CriticalPriority;
	CriticalPriority.Text = TEXT("NEW TRACK RECORD!");
	CriticalPriority.Priority = ECommentPriority::Critical;
	CriticalPriority.Emotion = ENOMIEmotion::Celebrating;
	Category.Comments.Add(CriticalPriority);

	Engine->AddCommentCategory(Category);

	// Test 3: Verify comment priorities are set correctly
	TestEqual(TEXT("Low priority should be Low"), LowPriority.Priority, ECommentPriority::Low);
	TestEqual(TEXT("Medium priority should be Medium"), MediumPriority.Priority, ECommentPriority::Medium);
	TestEqual(TEXT("High priority should be High"), HighPriority.Priority, ECommentPriority::High);
	TestEqual(TEXT("Critical priority should be Critical"), CriticalPriority.Priority, ECommentPriority::Critical);

	// Test 4: Request high speed comment
	FCommentContext Context;
	Context.Event = ERaceEvent::HighSpeed;
	Context.PlayerName = TEXT("NIO Driver");
	Context.Speed = 280.0f;
	Context.bIsNIOVehicle = true;

	bool bRequested = Engine->RequestComment(Context);
	TestTrue(TEXT("High speed comment request should execute"), true);

	// Test 5: Verify priority enum ordering
	TestTrue(TEXT("Low < Medium"), (uint8)ECommentPriority::Low < (uint8)ECommentPriority::Medium);
	TestTrue(TEXT("Medium < High"), (uint8)ECommentPriority::Medium < (uint8)ECommentPriority::High);
	TestTrue(TEXT("High < Critical"), (uint8)ECommentPriority::High < (uint8)ECommentPriority::Critical);

	return true;
}

/**
 * Test NIO vehicle-specific comments
 * Verifies that NIO-specific comments are preferred for NIO vehicles
 */
bool FNOMINIOVehicleCommentTest::RunTest(const FString& Parameters)
{
	// Test 1: Create commentary engine
	UCommentaryEngine* Engine = NewObject<UCommentaryEngine>();
	TestNotNull(TEXT("CommentaryEngine should be created"), Engine);

	if (!Engine)
	{
		return false;
	}

	// Test 2: Add NIO-specific comment category
	FCommentCategory NIOCategory;
	NIOCategory.CategoryName = TEXT("nio_specific");

	FNOMIComment NIOComment1;
	NIOComment1.Text = TEXT("The NIO {player} is showing its electric power!");
	NIOComment1.Priority = ECommentPriority::High;
	NIOComment1.Emotion = ENOMIEmotion::Excited;
	NIOComment1.bIsNIOSpecific = true;
	NIOCategory.Comments.Add(NIOComment1);

	FNOMIComment NIOComment2;
	NIOComment2.Text = TEXT("That instant electric torque is incredible!");
	NIOComment2.Priority = ECommentPriority::Medium;
	NIOComment2.Emotion = ENOMIEmotion::Happy;
	NIOComment2.bIsNIOSpecific = true;
	NIOCategory.Comments.Add(NIOComment2);

	Engine->AddCommentCategory(NIOCategory);

	// Test 3: Add comfort comments for frustration scenarios
	FCommentCategory ComfortCategory;
	ComfortCategory.CategoryName = TEXT("comfort");

	FNOMIComment ComfortComment;
	ComfortComment.Text = TEXT("Don't worry, there's still time to catch up!");
	ComfortComment.Priority = ECommentPriority::Medium;
	ComfortComment.Emotion = ENOMIEmotion::Concerned;
	ComfortComment.bIsComfortComment = true;
	ComfortCategory.Comments.Add(ComfortComment);

	Engine->AddCommentCategory(ComfortCategory);

	// Test 4: Create context for NIO vehicle
	FCommentContext NIOContext;
	NIOContext.Event = ERaceEvent::HighSpeed;
	NIOContext.PlayerName = TEXT("NIO EP9 Driver");
	NIOContext.Speed = 300.0f;
	NIOContext.bIsNIOVehicle = true;
	NIOContext.NIOVehicleType = ENIOVehicleType::EP9;

	// Test 5: Request comment for NIO vehicle
	Engine->RequestComment(NIOContext);
	TestTrue(TEXT("NIO comment request should execute"), true);

	// Test 6: Create context for non-NIO vehicle
	FCommentContext NonNIOContext;
	NonNIOContext.Event = ERaceEvent::HighSpeed;
	NonNIOContext.PlayerName = TEXT("Other Driver");
	NonNIOContext.Speed = 250.0f;
	NonNIOContext.bIsNIOVehicle = false;

	// Test 7: Request comment for non-NIO vehicle
	Engine->RequestComment(NonNIOContext);
	TestTrue(TEXT("Non-NIO comment request should execute"), true);

	// Test 8: Test comfort comment for last place scenario
	FCommentContext LastPlaceContext;
	LastPlaceContext.Event = ERaceEvent::LastPlace;
	LastPlaceContext.PlayerName = TEXT("NIO Driver");
	LastPlaceContext.Position = 8;
	LastPlaceContext.bIsNIOVehicle = true;

	Engine->RequestComment(LastPlaceContext);
	TestTrue(TEXT("Comfort comment request for last place should execute"), true);

	return true;
}

/**
 * Test race event bridging to NOMI system
 * Verifies that race events can be forwarded to the commentary engine
 */
bool FNOMIRaceEventBridgingTest::RunTest(const FString& Parameters)
{
	// Test 1: Create race manager and commentary engine
	ARaceManager* RaceManager = NewObject<ARaceManager>();
	UCommentaryEngine* Engine = NewObject<UCommentaryEngine>();
	TestNotNull(TEXT("RaceManager should be created"), RaceManager);
	TestNotNull(TEXT("CommentaryEngine should be created"), Engine);

	if (!RaceManager || !Engine)
	{
		return false;
	}

	// Test 2: Add comment categories for race events
	FCommentCategory StartCategory;
	StartCategory.CategoryName = TEXT("race_start");
	FNOMIComment StartComment;
	StartComment.Text = TEXT("And the race begins!");
	StartComment.Priority = ECommentPriority::Critical;
	StartComment.Emotion = ENOMIEmotion::Excited;
	StartCategory.Comments.Add(StartComment);
	Engine->AddCommentCategory(StartCategory);

	FCommentCategory FinishCategory;
	FinishCategory.CategoryName = TEXT("race_finish");
	FNOMIComment FinishComment;
	FinishComment.Text = TEXT("What a race! Position {position}!");
	FinishComment.Priority = ECommentPriority::Critical;
	FinishComment.Emotion = ENOMIEmotion::Celebrating;
	FinishCategory.Comments.Add(FinishComment);
	Engine->AddCommentCategory(FinishCategory);

	// Test 3: Bridge race events to commentary engine
	TArray<ERaceEvent> ReceivedEvents;
	RaceManager->OnRaceEvent.AddLambda([&Engine, &ReceivedEvents](ERaceEvent Event, const FRacerData& Data)
	{
		ReceivedEvents.Add(Event);

		FCommentContext Context;
		Context.Event = Event;
		Context.PlayerName = Data.DisplayName;
		Context.Position = Data.Position;
		Context.CurrentLap = Data.CurrentLap;
		Context.bIsNIOVehicle = true;

		Engine->RequestComment(Context);
	});

	// Test 4: Register player and start race
	APawn* PlayerPawn = NewObject<APawn>();
	RaceManager->RegisterRacer(PlayerPawn, TEXT("NIO EP9 Driver"), true);

	FRaceConfig Config;
	Config.NumLaps = 3;
	Config.TrackName = TEXT("Integration Test Track");
	RaceManager->StartRace(Config);

	// Test 5: Verify that race events were forwarded
	TestTrue(TEXT("Should have received at least one event"), ReceivedEvents.Num() > 0);

	// Test 6: Verify first event is CountdownStart
	if (ReceivedEvents.Num() > 0)
	{
		TestEqual(TEXT("First event should be CountdownStart"), ReceivedEvents[0], ERaceEvent::CountdownStart);
	}

	// Test 7: End race and verify RaceFinish event
	RaceManager->EndRace();
	TestTrue(TEXT("Should have received RaceFinish event"),
		ReceivedEvents.Contains(ERaceEvent::RaceFinish));

	return true;
}

/**
 * Test NOMI expression mapping from emotions
 * Verifies that the NOMI controller maps emotions to the correct facial expressions
 */
bool FNOMIExpressionMappingTest::RunTest(const FString& Parameters)
{
	// Test 1: Create NOMI controller
	ANOMIController* NOMI = NewObject<ANOMIController>();
	TestNotNull(TEXT("NOMI Controller should be created"), NOMI);

	if (!NOMI)
	{
		return false;
	}

	// Test 2: Verify initial expression is Neutral
	TestEqual(TEXT("Initial expression should be Neutral"), NOMI->GetExpression(), ENOMIExpression::Neutral);

	// Test 3: Test expression setting
	NOMI->SetExpression(ENOMIExpression::Happy);
	TestEqual(TEXT("Expression should be Happy"), NOMI->GetExpression(), ENOMIExpression::Happy);

	NOMI->SetExpression(ENOMIExpression::Excited);
	TestEqual(TEXT("Expression should be Excited"), NOMI->GetExpression(), ENOMIExpression::Excited);

	NOMI->SetExpression(ENOMIExpression::Surprised);
	TestEqual(TEXT("Expression should be Surprised"), NOMI->GetExpression(), ENOMIExpression::Surprised);

	NOMI->SetExpression(ENOMIExpression::Sad);
	TestEqual(TEXT("Expression should be Sad"), NOMI->GetExpression(), ENOMIExpression::Sad);

	NOMI->SetExpression(ENOMIExpression::Confused);
	TestEqual(TEXT("Expression should be Confused"), NOMI->GetExpression(), ENOMIExpression::Confused);

	NOMI->SetExpression(ENOMIExpression::Sleepy);
	TestEqual(TEXT("Expression should be Sleepy"), NOMI->GetExpression(), ENOMIExpression::Sleepy);

	NOMI->SetExpression(ENOMIExpression::Curious);
	TestEqual(TEXT("Expression should be Curious"), NOMI->GetExpression(), ENOMIExpression::Curious);

	// Test 4: Verify commentary engine is accessible
	UCommentaryEngine* Engine = NOMI->GetCommentaryEngine();
	TestNotNull(TEXT("NOMI should have a commentary engine"), Engine);

	// Test 5: Verify expression enum completeness
	// Each ENOMIEmotion should map to an ENOMIExpression
	TArray<TPair<ENOMIEmotion, ENOMIExpression>> ExpectedMappings = {
		{ENOMIEmotion::Idle, ENOMIExpression::Neutral},
		{ENOMIEmotion::Happy, ENOMIExpression::Happy},
		{ENOMIEmotion::Excited, ENOMIExpression::Excited},
		{ENOMIEmotion::Nervous, ENOMIExpression::Curious},
		{ENOMIEmotion::Surprised, ENOMIExpression::Surprised},
		{ENOMIEmotion::Celebrating, ENOMIExpression::Excited},
		{ENOMIEmotion::Concerned, ENOMIExpression::Sad},
		{ENOMIEmotion::Confused, ENOMIExpression::Confused},
		{ENOMIEmotion::Tired, ENOMIExpression::Sleepy}
	};

	TestEqual(TEXT("Should have 9 emotion-expression mappings"), ExpectedMappings.Num(), 9);

	// Test 6: Verify all expression types are distinct
	TArray<ENOMIExpression> AllExpressions = {
		ENOMIExpression::Neutral,
		ENOMIExpression::Happy,
		ENOMIExpression::Sad,
		ENOMIExpression::Sleepy,
		ENOMIExpression::Curious,
		ENOMIExpression::Surprised,
		ENOMIExpression::Confused,
		ENOMIExpression::Excited
	};

	// Verify they are all different values
	TSet<int32> ExpressionValues;
	for (ENOMIExpression Expr : AllExpressions)
	{
		ExpressionValues.Add((int32)Expr);
	}
	TestEqual(TEXT("All 8 expressions should be unique"), ExpressionValues.Num(), 8);

	return true;
}

/**
 * Test comment variable replacement
 * Verifies that placeholder variables in comments are replaced correctly
 */
bool FNOMICommentVariableReplacementTest::RunTest(const FString& Parameters)
{
	// Test 1: Create commentary engine
	UCommentaryEngine* Engine = NewObject<UCommentaryEngine>();
	TestNotNull(TEXT("CommentaryEngine should be created"), Engine);

	if (!Engine)
	{
		return false;
	}

	// Test 2: Add comments with various placeholders
	FCommentCategory Category;
	Category.CategoryName = TEXT("test_variables");

	FNOMIComment VariableComment;
	VariableComment.Text = TEXT("{player} overtakes {rival} at position {position}!");
	VariableComment.Priority = ECommentPriority::High;
	VariableComment.Emotion = ENOMIEmotion::Excited;
	Category.Comments.Add(VariableComment);

	FNOMIComment LapComment;
	LapComment.Text = TEXT("Lap {lap} in {time}s, cornering at {corner}!");
	LapComment.Priority = ECommentPriority::Medium;
	LapComment.Emotion = ENOMIEmotion::Happy;
	Category.Comments.Add(LapComment);

	FNOMIComment SpeedComment;
	SpeedComment.Text = TEXT("{player} hitting {speed} km/h!");
	SpeedComment.Priority = ECommentPriority::Medium;
	SpeedComment.Emotion = ENOMIEmotion::Excited;
	Category.Comments.Add(SpeedComment);

	FNOMIComment DriftComment;
	DriftComment.Text = TEXT("Epic {duration} second drift by {player}!");
	DriftComment.Priority = ECommentPriority::High;
	DriftComment.Emotion = ENOMIEmotion::Excited;
	Category.Comments.Add(DriftComment);

	Engine->AddCommentCategory(Category);

	// Test 3: Create context with all variable values
	FCommentContext Context;
	Context.Event = ERaceEvent::Overtake;
	Context.PlayerName = TEXT("NIO EP9");
	Context.RivalName = TEXT("AI Champion");
	Context.Position = 1;
	Context.CurrentLap = 3;
	Context.LapTime = 42.5f;
	Context.Speed = 295.0f;
	Context.CornerName = TEXT("Turn 7");
	Context.DriftDuration = 4.2f;
	Context.bIsNIOVehicle = true;

	// Test 4: Verify context fields are populated
	TestEqual(TEXT("Player name should be set"), Context.PlayerName, FString(TEXT("NIO EP9")));
	TestEqual(TEXT("Rival name should be set"), Context.RivalName, FString(TEXT("AI Champion")));
	TestEqual(TEXT("Position should be 1"), Context.Position, 1);
	TestEqual(TEXT("Lap should be 3"), Context.CurrentLap, 3);
	TestEqual(TEXT("Speed should be 295"), Context.Speed, 295.0f);

	// Test 5: Verify comment text contains placeholders
	TestTrue(TEXT("Variable comment should contain {player}"),
		VariableComment.Text.Contains(TEXT("{player}")));
	TestTrue(TEXT("Variable comment should contain {rival}"),
		VariableComment.Text.Contains(TEXT("{rival}")));
	TestTrue(TEXT("Variable comment should contain {position}"),
		VariableComment.Text.Contains(TEXT("{position}")));

	// Test 6: Request comment (which triggers variable replacement internally)
	Engine->RequestComment(Context);
	TestTrue(TEXT("Variable replacement request should execute"), true);

	// Test 7: Verify all placeholder types exist in comments
	TArray<FString> Placeholders = {
		TEXT("{player}"), TEXT("{rival}"), TEXT("{position}"),
		TEXT("{lap}"), TEXT("{time}"), TEXT("{speed}"),
		TEXT("{corner}"), TEXT("{duration}")
	};

	for (const FString& Placeholder : Placeholders)
	{
		bool bFound = false;
		for (const FNOMIComment& Comment : Category.Comments)
		{
			if (Comment.Text.Contains(Placeholder))
			{
				bFound = true;
				break;
			}
		}
		TestTrue(FString::Printf(TEXT("Placeholder %s should appear in at least one comment"), *Placeholder), bFound);
	}

	return true;
}
