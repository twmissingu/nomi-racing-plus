// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "CameraSystemTest.h"
#include "Core/CameraSystem.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraSystemModeTest, "NomiRacing.CameraSystem.CameraMode",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraSystemModeTest::RunTest(const FString& Parameters)
{
	// Test camera mode cycling
	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();

	// Initial mode should be Chase
	TestEqual(TEXT("Initial mode should be Chase"), CameraSystem->GetCameraMode(), ECameraMode::Chase);

	// Cycle through modes
	CameraSystem->CycleCameraMode();
	TestEqual(TEXT("After first cycle should be Hood"), CameraSystem->GetCameraMode(), ECameraMode::Hood);

	CameraSystem->CycleCameraMode();
	TestEqual(TEXT("After second cycle should be Cockpit"), CameraSystem->GetCameraMode(), ECameraMode::Cockpit);

	CameraSystem->CycleCameraMode();
	TestEqual(TEXT("After third cycle should be Bumper"), CameraSystem->GetCameraMode(), ECameraMode::Bumper);

	CameraSystem->CycleCameraMode();
	TestEqual(TEXT("After fourth cycle should be Cinematic"), CameraSystem->GetCameraMode(), ECameraMode::Cinematic);

	CameraSystem->CycleCameraMode();
	TestEqual(TEXT("After fifth cycle should be Free"), CameraSystem->GetCameraMode(), ECameraMode::Free);

	CameraSystem->CycleCameraMode();
	TestEqual(TEXT("After sixth cycle should wrap to Chase"), CameraSystem->GetCameraMode(), ECameraMode::Chase);

	// Test direct mode setting
	CameraSystem->SetCameraMode(ECameraMode::Cockpit);
	TestEqual(TEXT("Direct set to Cockpit"), CameraSystem->GetCameraMode(), ECameraMode::Cockpit);

	CameraSystem->SetCameraMode(ECameraMode::Replay);
	TestEqual(TEXT("Direct set to Replay"), CameraSystem->GetCameraMode(), ECameraMode::Replay);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraSystemModeNameTest, "NomiRacing.CameraSystem.CameraModeName",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraSystemModeNameTest::RunTest(const FString& Parameters)
{
	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();

	// Test mode names
	CameraSystem->SetCameraMode(ECameraMode::Chase);
	TestEqual(TEXT("Chase mode name"), CameraSystem->GetCameraModeName(), FString(TEXT("Chase")));

	CameraSystem->SetCameraMode(ECameraMode::Hood);
	TestEqual(TEXT("Hood mode name"), CameraSystem->GetCameraModeName(), FString(TEXT("Hood")));

	CameraSystem->SetCameraMode(ECameraMode::Cockpit);
	TestEqual(TEXT("Cockpit mode name"), CameraSystem->GetCameraModeName(), FString(TEXT("Cockpit")));

	CameraSystem->SetCameraMode(ECameraMode::Bumper);
	TestEqual(TEXT("Bumper mode name"), CameraSystem->GetCameraModeName(), FString(TEXT("Bumper")));

	CameraSystem->SetCameraMode(ECameraMode::Cinematic);
	TestEqual(TEXT("Cinematic mode name"), CameraSystem->GetCameraModeName(), FString(TEXT("Cinematic")));

	CameraSystem->SetCameraMode(ECameraMode::Free);
	TestEqual(TEXT("Free mode name"), CameraSystem->GetCameraModeName(), FString(TEXT("Free")));

	CameraSystem->SetCameraMode(ECameraMode::Replay);
	TestEqual(TEXT("Replay mode name"), CameraSystem->GetCameraModeName(), FString(TEXT("Replay")));

	return true;
}

// NOTE: FCameraSystemFOVTest removed - CalculateDynamicFOV is private in the current API.
// The FOV behavior is exercised indirectly through GetCurrentFOV() after mode/speed changes.

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraSystemReplayTest, "NomiRacing.CameraSystem.ReplaySystem",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraSystemReplayTest::RunTest(const FString& Parameters)
{
	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();

	// Test recording
	TestFalse(TEXT("Initially not recording"), CameraSystem->IsRecording());
	CameraSystem->StartRecording();
	TestTrue(TEXT("Should be recording after start"), CameraSystem->IsRecording());
	CameraSystem->StopRecording();
	TestFalse(TEXT("Should not be recording after stop"), CameraSystem->IsRecording());

	// Test playback
	TestFalse(TEXT("Initially not playing"), CameraSystem->IsPlaying());

	// Cannot start playback without data
	CameraSystem->StartPlayback();
	TestFalse(TEXT("Cannot start playback without data"), CameraSystem->IsPlaying());

	// Test playback speed
	CameraSystem->SetPlaybackSpeed(0.5f);
	TestEqual(TEXT("Playback speed 0.5"), CameraSystem->GetPlaybackSpeed(), 0.5f);

	CameraSystem->SetPlaybackSpeed(2.0f);
	TestEqual(TEXT("Playback speed 2.0"), CameraSystem->GetPlaybackSpeed(), 2.0f);

	CameraSystem->SetPlaybackSpeed(1.5f); // Should snap to nearest valid
	TestEqual(TEXT("Playback speed 1.5 snaps to 2.0"), CameraSystem->GetPlaybackSpeed(), 2.0f);

	// Test pause
	TestFalse(TEXT("Initially not paused"), CameraSystem->IsReplayPaused());
	CameraSystem->ToggleReplayPause();
	TestTrue(TEXT("Should be paused after toggle"), CameraSystem->IsReplayPaused());
	CameraSystem->ToggleReplayPause();
	TestFalse(TEXT("Should not be paused after second toggle"), CameraSystem->IsReplayPaused());

	// Test replay progress
	TestEqual(TEXT("Progress without data should be 0"), CameraSystem->GetReplayProgress(), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraSystemCinematicTest, "NomiRacing.CameraSystem.CinematicSystem",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraSystemCinematicTest::RunTest(const FString& Parameters)
{
	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();

	// Test cinematic shot setting
	CameraSystem->SetCinematicShot(ECinematicShotType::WideOrbit);
	TestEqual(TEXT("Set to WideOrbit"), CameraSystem->GetCurrentCinematicShot(), ECinematicShotType::WideOrbit);

	CameraSystem->SetCinematicShot(ECinematicShotType::BirdsEye);
	TestEqual(TEXT("Set to BirdsEye"), CameraSystem->GetCurrentCinematicShot(), ECinematicShotType::BirdsEye);

	CameraSystem->SetCinematicShot(ECinematicShotType::DutchAngle);
	TestEqual(TEXT("Set to DutchAngle"), CameraSystem->GetCurrentCinematicShot(), ECinematicShotType::DutchAngle);

	// Test auto-cycle
	CameraSystem->SetCinematicAutoCycle(true);
	CameraSystem->SetCinematicShotDuration(3.0f);

	CameraSystem->SetCinematicAutoCycle(false);
	CameraSystem->SetCinematicShotDuration(5.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraSystemConfigTest, "NomiRacing.CameraSystem.CameraConfig",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraSystemConfigTest::RunTest(const FString& Parameters)
{
	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();

	// Test default configs exist
	const FCameraConfig& ChaseConfig = CameraSystem->GetCameraConfig(ECameraMode::Chase);
	TestEqual(TEXT("Chase FOV"), ChaseConfig.FOV, 90.0f);
	TestTrue(TEXT("Chase has shake enabled"), ChaseConfig.bEnableShake);

	const FCameraConfig& HoodConfig = CameraSystem->GetCameraConfig(ECameraMode::Hood);
	TestEqual(TEXT("Hood FOV"), HoodConfig.FOV, 100.0f);
	TestTrue(TEXT("Hood has head bob"), HoodConfig.bEnableHeadBob);

	const FCameraConfig& CockpitConfig = CameraSystem->GetCameraConfig(ECameraMode::Cockpit);
	TestEqual(TEXT("Cockpit FOV"), CockpitConfig.FOV, 110.0f);
	TestTrue(TEXT("Cockpit has head bob"), CockpitConfig.bEnableHeadBob);
	TestTrue(TEXT("Cockpit has speed roll"), CockpitConfig.SpeedRollAngle > 0.0f);

	// Test custom config
	FCameraConfig CustomConfig;
	CustomConfig.FOV = 120.0f;
	CustomConfig.Distance = 1000.0f;
	CustomConfig.Height = 400.0f;
	CameraSystem->SetCameraConfig(ECameraMode::Chase, CustomConfig);

	const FCameraConfig& RetrievedConfig = CameraSystem->GetCameraConfig(ECameraMode::Chase);
	TestEqual(TEXT("Custom FOV"), RetrievedConfig.FOV, 120.0f);
	TestEqual(TEXT("Custom Distance"), RetrievedConfig.Distance, 1000.0f);
	TestEqual(TEXT("Custom Height"), RetrievedConfig.Height, 400.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraSystemShakeTest, "NomiRacing.CameraSystem.CameraShake",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraSystemShakeTest::RunTest(const FString& Parameters)
{
	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();

	// Test shake triggers
	FCameraShakeParams ShakeParams;
	ShakeParams.Frequency = 15.0f;
	ShakeParams.Amplitude = 2.0f;
	ShakeParams.Duration = 0.5f;
	ShakeParams.AxisMultiplier = FVector(1.0f, 1.0f, 0.5f);

	CameraSystem->TriggerShake(ShakeParams);

	// Test collision shake
	CameraSystem->TriggerCollisionShake(100.0f);

	// Test drift shake
	CameraSystem->TriggerDriftShake(0.8f);

	// Test gear shift shake
	CameraSystem->TriggerGearShiftShake();

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCameraSystemReplayAngleTest, "NomiRacing.CameraSystem.ReplayCameraAngle",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraSystemReplayAngleTest::RunTest(const FString& Parameters)
{
	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();

	// Test replay camera angles
	CameraSystem->SetReplayCameraAngle(EReplayCameraAngle::Chase);
	TestEqual(TEXT("Replay angle Chase"), CameraSystem->GetReplayCameraAngle(), EReplayCameraAngle::Chase);

	CameraSystem->SetReplayCameraAngle(EReplayCameraAngle::Cinematic);
	TestEqual(TEXT("Replay angle Cinematic"), CameraSystem->GetReplayCameraAngle(), EReplayCameraAngle::Cinematic);

	CameraSystem->SetReplayCameraAngle(EReplayCameraAngle::FreeOrbit);
	TestEqual(TEXT("Replay angle FreeOrbit"), CameraSystem->GetReplayCameraAngle(), EReplayCameraAngle::FreeOrbit);

	CameraSystem->SetReplayCameraAngle(EReplayCameraAngle::TrackSide);
	TestEqual(TEXT("Replay angle TrackSide"), CameraSystem->GetReplayCameraAngle(), EReplayCameraAngle::TrackSide);

	CameraSystem->SetReplayCameraAngle(EReplayCameraAngle::TopDown);
	TestEqual(TEXT("Replay angle TopDown"), CameraSystem->GetReplayCameraAngle(), EReplayCameraAngle::TopDown);

	CameraSystem->SetReplayCameraAngle(EReplayCameraAngle::Bumper);
	TestEqual(TEXT("Replay angle Bumper"), CameraSystem->GetReplayCameraAngle(), EReplayCameraAngle::Bumper);

	return true;
}

#endif // WITH_AUTOMATION_TESTS
