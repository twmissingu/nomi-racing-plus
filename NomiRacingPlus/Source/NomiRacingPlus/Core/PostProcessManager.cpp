// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "PostProcessManager.h"
#include "Engine/Engine.h"
#include "NomiRacingPlus.h"

UPostProcessManager::UPostProcessManager()
{
	PrimaryComponentTick.bCanEverTick = false;

	bEnableSSAO = true;
	bEnableBloom = true;
	bEnableMotionBlur = true;
	BloomIntensity = 1.0f;
	SSAORadius = 1.0f;
	MotionBlurAmount = 0.5f;
}

void UPostProcessManager::ApplyPostProcessSettings()
{
	if (!GEngine)
	{
		UE_LOG(LogNomiPerf, Warning, TEXT("PostProcessManager: GEngine is null, cannot apply settings"));
		return;
	}

	// SSAO: 0 = off, 1 = SSAO method
	const int32 SSAOValue = bEnableSSAO ? 1 : 0;
	const FString SSAOCommand = FString::Printf(TEXT("r.AmbientOcclusion.Method %d"), SSAOValue);
	GEngine->Exec(nullptr, *SSAOCommand);
	UE_LOG(LogNomiPerf, Log, TEXT("PostProcessManager: Set SSAO method to %d"), SSAOValue);

	// Bloom: 0 = off, 1-6 = quality levels
	int32 BloomValue = 0;
	if (bEnableBloom)
	{
		BloomValue = FMath::Clamp(FMath::RoundToInt(BloomIntensity * 6.0f), 1, 6);
	}
	const FString BloomCommand = FString::Printf(TEXT("r.BloomQuality %d"), BloomValue);
	GEngine->Exec(nullptr, *BloomCommand);
	UE_LOG(LogNomiPerf, Log, TEXT("PostProcessManager: Set Bloom quality to %d"), BloomValue);

	// Motion Blur: 0 = off, 1-4 = quality
	int32 MotionBlurValue = 0;
	if (bEnableMotionBlur)
	{
		MotionBlurValue = FMath::Clamp(FMath::RoundToInt(MotionBlurAmount * 4.0f), 1, 4);
	}
	const FString MotionBlurCommand = FString::Printf(TEXT("r.MotionBlurQuality %d"), MotionBlurValue);
	GEngine->Exec(nullptr, *MotionBlurCommand);
	UE_LOG(LogNomiPerf, Log, TEXT("PostProcessManager: Set MotionBlur quality to %d"), MotionBlurValue);

	// SSR: map from bloom quality as a proxy for overall quality
	const int32 SSRValue = bEnableBloom ? FMath::Clamp(FMath::RoundToInt(BloomIntensity * 4.0f), 1, 4) : 0;
	const FString SSRCommand = FString::Printf(TEXT("r.SSR.Quality %d"), SSRValue);
	GEngine->Exec(nullptr, *SSRCommand);
	UE_LOG(LogNomiPerf, Log, TEXT("PostProcessManager: Set SSR quality to %d"), SSRValue);
}

void UPostProcessManager::SetQualityPreset(int32 Level)
{
	switch (Level)
	{
	case 0: // Low
		bEnableSSAO = false;
		BloomIntensity = 1.0f / 6.0f; // Maps to Bloom quality 1
		bEnableBloom = true;
		bEnableMotionBlur = false;
		MotionBlurAmount = 0.0f;
		UE_LOG(LogNomiPerf, Log, TEXT("PostProcessManager: Applied Low quality preset"));
		break;

	case 1: // Medium
		bEnableSSAO = true;
		BloomIntensity = 3.0f / 6.0f; // Maps to Bloom quality 3
		bEnableBloom = true;
		bEnableMotionBlur = true;
		MotionBlurAmount = 0.25f; // Maps to MotionBlur quality 1
		UE_LOG(LogNomiPerf, Log, TEXT("PostProcessManager: Applied Medium quality preset"));
		break;

	case 2: // High
		bEnableSSAO = true;
		BloomIntensity = 1.0f; // Maps to Bloom quality 6
		bEnableBloom = true;
		bEnableMotionBlur = true;
		MotionBlurAmount = 0.5f; // Maps to MotionBlur quality 2
		UE_LOG(LogNomiPerf, Log, TEXT("PostProcessManager: Applied High quality preset"));
		break;

	default:
		UE_LOG(LogNomiPerf, Warning, TEXT("PostProcessManager: Invalid quality level %d, expected 0-2"), Level);
		return;
	}

	ApplyPostProcessSettings();
}

void UPostProcessManager::ToggleSSAO(bool bEnabled)
{
	bEnableSSAO = bEnabled;
	UE_LOG(LogNomiPerf, Log, TEXT("PostProcessManager: SSAO toggled %s"), bEnabled ? TEXT("ON") : TEXT("OFF"));
	ApplyPostProcessSettings();
}

void UPostProcessManager::ToggleBloom(bool bEnabled)
{
	bEnableBloom = bEnabled;
	UE_LOG(LogNomiPerf, Log, TEXT("PostProcessManager: Bloom toggled %s"), bEnabled ? TEXT("ON") : TEXT("OFF"));
	ApplyPostProcessSettings();
}

void UPostProcessManager::ToggleMotionBlur(bool bEnabled)
{
	bEnableMotionBlur = bEnabled;
	UE_LOG(LogNomiPerf, Log, TEXT("PostProcessManager: MotionBlur toggled %s"), bEnabled ? TEXT("ON") : TEXT("OFF"));
	ApplyPostProcessSettings();
}
