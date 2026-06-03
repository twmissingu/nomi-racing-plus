// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PostProcessManager.generated.h"

/**
 * Manages post-processing settings for the racing game.
 * Controls SSAO, Bloom, Motion Blur, and SSR quality presets.
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UPostProcessManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UPostProcessManager();

	/** Enable Screen Space Ambient Occlusion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcessing")
	bool bEnableSSAO;

	/** Enable Bloom effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcessing")
	bool bEnableBloom;

	/** Enable Motion Blur effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcessing")
	bool bEnableMotionBlur;

	/** Bloom intensity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcessing", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float BloomIntensity;

	/** SSAO radius for ambient occlusion sampling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcessing", meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float SSAORadius;

	/** Motion blur strength multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcessing", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float MotionBlurAmount;

	/** Apply current post-process settings via console commands */
	UFUNCTION(BlueprintCallable, Category = "PostProcessing")
	void ApplyPostProcessSettings();

	/**
	 * Set a quality preset for post-processing effects.
	 * @param Level 0=Low, 1=Medium, 2=High
	 */
	UFUNCTION(BlueprintCallable, Category = "PostProcessing")
	void SetQualityPreset(int32 Level);

	/** Toggle SSAO on or off */
	UFUNCTION(BlueprintCallable, Category = "PostProcessing")
	void ToggleSSAO(bool bEnabled);

	/** Toggle Bloom on or off */
	UFUNCTION(BlueprintCallable, Category = "PostProcessing")
	void ToggleBloom(bool bEnabled);

	/** Toggle Motion Blur on or off */
	UFUNCTION(BlueprintCallable, Category = "PostProcessing")
	void ToggleMotionBlur(bool bEnabled);
};
