// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "CameraSystem.generated.h"

/**
 * Camera modes
 */
UENUM(BlueprintType)
enum class ECameraMode : uint8
{
	Chase       UMETA(DisplayName = "Chase Camera"),
	Hood        UMETA(DisplayName = "Hood Camera"),
	Cockpit     UMETA(DisplayName = "Cockpit Camera"),
	Bumper      UMETA(DisplayName = "Bumper Camera"),
	Free        UMETA(DisplayName = "Free Camera"),
	Cinematic   UMETA(DisplayName = "Cinematic Camera"),
	Replay      UMETA(DisplayName = "Replay Camera")
};

/**
 * Cinematic shot types for auto-directing
 */
UENUM(BlueprintType)
enum class ECinematicShotType : uint8
{
	WideOrbit         UMETA(DisplayName = "Wide Orbit"),
	LowCloseUp        UMETA(DisplayName = "Low Close-Up"),
	BirdsEye          UMETA(DisplayName = "Bird's Eye"),
	DramaticLowAngle  UMETA(DisplayName = "Dramatic Low Angle"),
	TrackingSide      UMETA(DisplayName = "Tracking Side"),
	FrontTracking     UMETA(DisplayName = "Front Tracking"),
	DutchAngle        UMETA(DisplayName = "Dutch Angle"),
	OverShoulder      UMETA(DisplayName = "Over Shoulder"),
	Helicopter        UMETA(DisplayName = "Helicopter"),
	StaticBumper      UMETA(DisplayName = "Static Bumper")
};

/**
 * Replay camera angle presets
 */
UENUM(BlueprintType)
enum class EReplayCameraAngle : uint8
{
	Chase       UMETA(DisplayName = "Chase View"),
	Cinematic   UMETA(DisplayName = "Cinematic Auto"),
	FreeOrbit   UMETA(DisplayName = "Free Orbit"),
	TrackSide   UMETA(DisplayName = "Track Side"),
	TopDown     UMETA(DisplayName = "Top Down"),
	Bumper      UMETA(DisplayName = "Bumper View")
};

/**
 * Camera configuration for each mode
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FCameraConfig
{
	GENERATED_BODY()

	// Field of view
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float FOV = 90.0f;

	// FOV multiplier at max speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SpeedFOVMultiplier = 1.2f;

	// FOV multiplier during acceleration (wider when accelerating)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float AccelerationFOVMultiplier = 1.05f;

	// Distance from vehicle (chase cam)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float Distance = 600.0f;

	// Speed-dependent distance offset (positive = further at speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SpeedDistanceOffset = 100.0f;

	// Height offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float Height = 200.0f;

	// Speed-dependent height offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SpeedHeightOffset = -30.0f;

	// Look at height offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float LookAtHeight = 100.0f;

	// Interpolation speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float InterpSpeed = 5.0f;

	// Rotation lag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float RotationLag = 8.0f;

	// Enable shake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bEnableShake = true;

	// Shake intensity at max speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ShakeIntensity = 0.5f;

	// Lag for position (smooth following)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float PositionLag = 10.0f;

	// Spring stiffness for chase cam dynamics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Spring")
	float SpringStiffness = 300.0f;

	// Spring damping for chase cam dynamics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Spring")
	float SpringDamping = 15.0f;

	// Enable head bob (for cockpit/hood modes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Effects")
	bool bEnableHeadBob = false;

	// Head bob amplitude
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Effects")
	float HeadBobAmplitude = 5.0f;

	// Head bob frequency (scaled with speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Effects")
	float HeadBobFrequency = 2.0f;

	// Dutch angle (roll) at high speed (degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Effects")
	float SpeedRollAngle = 0.0f;

	// Enable motion blur influence on FOV
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Effects")
	bool bEnableMotionBlurFOV = false;
};

/**
 * Camera Shake parameters
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FCameraShakeParams
{
	GENERATED_BODY()

	// Shake frequency (Hz)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	float Frequency = 10.0f;

	// Shake amplitude
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	float Amplitude = 1.0f;

	// Shake duration (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	float Duration = 0.5f;

	// Falloff rate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	float FalloffRate = 2.0f;

	// Per-axis amplitude multipliers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	FVector AxisMultiplier = FVector(1.0f, 1.0f, 0.5f);
};

/**
 * Replay camera data point
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FReplayDataPoint
{
	GENERATED_BODY()

	// Timestamp
	UPROPERTY(BlueprintReadOnly, Category = "Replay")
	float Timestamp = 0.0f;

	// Vehicle location
	UPROPERTY(BlueprintReadOnly, Category = "Replay")
	FVector Location = FVector::ZeroVector;

	// Vehicle rotation
	UPROPERTY(BlueprintReadOnly, Category = "Replay")
	FRotator Rotation = FRotator::ZeroRotator;

	// Camera location
	UPROPERTY(BlueprintReadOnly, Category = "Replay")
	FVector CameraLocation = FVector::ZeroVector;

	// Camera rotation
	UPROPERTY(BlueprintReadOnly, Category = "Replay")
	FRotator CameraRotation = FRotator::ZeroRotator;

	// Vehicle speed (km/h)
	UPROPERTY(BlueprintReadOnly, Category = "Replay")
	float Speed = 0.0f;

	// Vehicle acceleration (km/h per second)
	UPROPERTY(BlueprintReadOnly, Category = "Replay")
	float Acceleration = 0.0f;

	// Is drifting at this point
	UPROPERTY(BlueprintReadOnly, Category = "Replay")
	bool bIsDrifting = false;

	// Slip angle at this point
	UPROPERTY(BlueprintReadOnly, Category = "Replay")
	float SlipAngle = 0.0f;
};

/**
 * Camera System - manages all camera modes and transitions
 * Handles chase, hood, cockpit, bumper, free, cinematic, and replay cameras
 * with dynamic FOV, spring dynamics, and cinematic auto-direction
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UCameraSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraSystem();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Camera Mode Control

	// Set camera mode
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(ECameraMode NewMode);

	// Get current camera mode
	UFUNCTION(BlueprintCallable, Category = "Camera")
	ECameraMode GetCameraMode() const { return CurrentMode; }

	// Cycle to next camera mode
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void CycleCameraMode();

	// Get camera mode name
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FString GetCameraModeName() const;

	// Camera Configuration

	// Set camera config for mode
	UFUNCTION(BlueprintCallable, Category = "Camera|Config")
	void SetCameraConfig(ECameraMode Mode, const FCameraConfig& Config);

	// Get camera config for mode
	UFUNCTION(BlueprintCallable, Category = "Camera|Config")
	const FCameraConfig& GetCameraConfig(ECameraMode Mode) const;

	// Camera Shake

	// Trigger camera shake
	UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
	void TriggerShake(const FCameraShakeParams& Params);

	// Trigger collision shake
	UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
	void TriggerCollisionShake(float ImpactForce);

	// Trigger drift shake
	UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
	void TriggerDriftShake(float DriftIntensity);

	// Trigger gear shift shake
	UFUNCTION(BlueprintCallable, Category = "Camera|Shake")
	void TriggerGearShiftShake();

	// Cinematic System

	// Set cinematic shot type (overrides auto-cycle)
	UFUNCTION(BlueprintCallable, Category = "Camera|Cinematic")
	void SetCinematicShot(ECinematicShotType ShotType);

	// Enable/disable auto-cycle for cinematic shots
	UFUNCTION(BlueprintCallable, Category = "Camera|Cinematic")
	void SetCinematicAutoCycle(bool bEnable) { bCinematicAutoCycle = bEnable; }

	// Set cinematic shot duration
	UFUNCTION(BlueprintCallable, Category = "Camera|Cinematic")
	void SetCinematicShotDuration(float Duration) { CinematicShotDuration = FMath::Max(Duration, 1.0f); }

	// Get current cinematic shot type
	UFUNCTION(BlueprintCallable, Category = "Camera|Cinematic")
	ECinematicShotType GetCurrentCinematicShot() const { return CurrentCinematicShot; }

	// Replay System

	// Start recording replay
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	void StartRecording();

	// Stop recording replay
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	void StopRecording();

	// Start playback
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	void StartPlayback();

	// Stop playback
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	void StopPlayback();

	// Is recording?
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	bool IsRecording() const { return bIsRecording; }

	// Is playing?
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	bool IsPlaying() const { return bIsPlaying; }

	// Get replay duration
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	float GetReplayDuration() const;

	// Set playback time (for timeline scrubbing)
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	void SetPlaybackTime(float Time);

	// Get current playback time
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	float GetPlaybackTime() const { return PlaybackTime; }

	// Set playback speed (0.25, 0.5, 1.0, 2.0, 4.0)
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	void SetPlaybackSpeed(float Speed);

	// Get playback speed
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	float GetPlaybackSpeed() const { return PlaybackSpeed; }

	// Set replay camera angle
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	void SetReplayCameraAngle(EReplayCameraAngle Angle);

	// Get replay camera angle
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	EReplayCameraAngle GetReplayCameraAngle() const { return ReplayCameraAngle; }

	// Toggle pause during replay
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	void ToggleReplayPause();

	// Is replay paused?
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	bool IsReplayPaused() const { return bIsReplayPaused; }

	// Get replay progress (0-1)
	UFUNCTION(BlueprintCallable, Category = "Camera|Replay")
	float GetReplayProgress() const;

	// Look-back support

	// Set look-back state
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetLookBack(bool bLookBack);

	// Get current FOV
	UFUNCTION(BlueprintCallable, Category = "Camera")
	float GetCurrentFOV() const { return CurrentFOV; }

	// Get current speed-based FOV offset (for UI effects)
	UFUNCTION(BlueprintCallable, Category = "Camera")
	float GetSpeedFOVOffset() const { return CurrentFOV - GetCameraConfig(CurrentMode).FOV; }

protected:
	// Camera component reference
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	// Current camera mode
	UPROPERTY(BlueprintReadOnly, Category = "Camera")
	ECameraMode CurrentMode = ECameraMode::Chase;

	// Camera configs for each mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Config")
	TMap<ECameraMode, FCameraConfig> CameraConfigs;

	// Current shake state
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Shake")
	bool bIsShaking = false;

	UPROPERTY(BlueprintReadOnly, Category = "Camera|Shake")
	float ShakeTimer = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Camera|Shake")
	FCameraShakeParams CurrentShake;

	// Replay data
	UPROPERTY(BlueprintReadOnly, Category = "Camera|Replay")
	TArray<FReplayDataPoint> ReplayData;

	UPROPERTY(BlueprintReadOnly, Category = "Camera|Replay")
	bool bIsRecording = false;

	UPROPERTY(BlueprintReadOnly, Category = "Camera|Replay")
	bool bIsPlaying = false;

	UPROPERTY(BlueprintReadOnly, Category = "Camera|Replay")
	float PlaybackTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Camera|Replay")
	float RecordingTime = 0.0f;

private:
	// Update camera based on mode
	void UpdateCamera(float DeltaTime);

	// Update chase camera with spring dynamics
	void UpdateChaseCamera(float DeltaTime, const FCameraConfig& Config);

	// Update hood camera
	void UpdateHoodCamera(float DeltaTime, const FCameraConfig& Config);

	// Update cockpit camera
	void UpdateCockpitCamera(float DeltaTime, const FCameraConfig& Config);

	// Update bumper camera
	void UpdateBumperCamera(float DeltaTime, const FCameraConfig& Config);

	// Update free camera
	void UpdateFreeCamera(float DeltaTime, const FCameraConfig& Config);

	// Update cinematic camera with auto-direction
	void UpdateCinematicCamera(float DeltaTime, const FCameraConfig& Config);

	// Update replay camera
	void UpdateReplayCamera(float DeltaTime);

	// Update camera shake
	void UpdateShake(float DeltaTime);

	// Update mode transition blend
	void UpdateTransition(float DeltaTime);

	// Calculate dynamic FOV based on speed and acceleration
	float CalculateDynamicFOV(float BaseFOV, float Speed, float MaxSpeed, float Acceleration) const;

	// Record replay data point
	void RecordReplayPoint();

	// Interpolate replay data
	FReplayDataPoint InterpolateReplayData(float Time) const;

	// Get vehicle speed in km/h
	float GetVehicleSpeed() const;

	// Get vehicle max speed in km/h
	float GetVehicleMaxSpeed() const;

	// Get vehicle acceleration (km/h per second)
	float GetVehicleAcceleration() const;

	// Initialize default camera configs
	void InitializeDefaultConfigs();

	// Calculate cinematic shot position
	FVector CalculateCinematicShotPosition(ECinematicShotType ShotType, const FVector& VehicleLocation,
		const FRotator& VehicleRotation, float Angle, float Distance, float Height) const;

	// Current camera state (blended output)
	FVector CurrentCameraLocation = FVector::ZeroVector;
	FRotator CurrentCameraRotation = FRotator::ZeroRotator;
	float CurrentFOV = 90.0f;

	// Target camera state (per-mode computed)
	FVector TargetCameraLocation = FVector::ZeroVector;
	FRotator TargetCameraRotation = FRotator::ZeroRotator;
	float TargetFOV = 90.0f;

	// Mode transition blend
	bool bIsTransitioning = false;
	float TransitionAlpha = 0.0f;
	float TransitionDuration = 0.5f;
	FVector TransitionStartLocation = FVector::ZeroVector;
	FRotator TransitionStartRotation = FRotator::ZeroRotator;
	float TransitionStartFOV = 90.0f;

	// Free camera orbit state
	float FreeCamOrbitYaw = 0.0f;
	float FreeCamOrbitPitch = -15.0f;

	// Cinematic camera state
	float CinematicTimer = 0.0f;
	float CinematicOrbitSpeed = 20.0f;
	float CinematicAngle = 0.0f;
	ECinematicShotType CurrentCinematicShot = ECinematicShotType::WideOrbit;
	float CinematicShotTimer = 0.0f;
	float CinematicShotDuration = 4.0f;
	bool bCinematicAutoCycle = true;
	int32 CinematicShotIndex = 0;
	float CinematicTransitionAlpha = 0.0f;
	FVector CinematicTransitionStart = FVector::ZeroVector;
	FRotator CinematicTransitionStartRot = FRotator::ZeroRotator;

	// Look-back state
	bool bLookingBack = false;

	// Speed tracking for acceleration calculation
	float PreviousSpeed = 0.0f;
	float CurrentAcceleration = 0.0f;
	float AccelerationSmoothing = 0.1f;

	// Speed shake accumulator
	float SpeedShakeAccumulator = 0.0f;

	// Collision shake
	float CollisionShakeTimer = 0.0f;
	float CollisionShakeIntensity = 0.0f;

	// Drift shake
	float DriftShakeTimer = 0.0f;
	float DriftShakeIntensity = 0.0f;

	// Gear shift shake
	float GearShiftShakeTimer = 0.0f;

	// Head bob accumulator
	float HeadBobAccumulator = 0.0f;

	// Spring dynamics for chase cam
	FVector SpringVelocity = FVector::ZeroVector;
	FVector SpringOffset = FVector::ZeroVector;

	// Replay playback speed
	float PlaybackSpeed = 1.0f;

	// Replay camera angle
	EReplayCameraAngle ReplayCameraAngle = EReplayCameraAngle::Chase;

	// Replay orbit state (for free orbit during replay)
	float ReplayOrbitYaw = 0.0f;
	float ReplayOrbitPitch = -20.0f;

	// Replay pause state
	bool bIsReplayPaused = false;

	// Max replay duration (seconds)
	UPROPERTY(EditAnywhere, Category = "Camera|Replay")
	float MaxReplayDuration = 300.0f;

	// Replay record interval (seconds)
	UPROPERTY(EditAnywhere, Category = "Camera|Replay")
	float ReplayRecordInterval = 0.033f; // ~30 Hz

	// Replay record timer
	float ReplayRecordTimer = 0.0f;

	// Vehicle reference
	UPROPERTY()
	TObjectPtr<AActor> VehicleActor;
};
