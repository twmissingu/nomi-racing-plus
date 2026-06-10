// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NIOVehicleMovementComponent.h"
#include "VehicleStateManager.h"
#include "NIOTirePresets.h"
#include "NomiRacingPlus.h"
#include "Core/NomiErrorHandler.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "DrawDebugHelpers.h"

UNIOVehicleMovementComponent::UNIOVehicleMovementComponent()
{
	// Electric vehicles have instant response
	MaxMotorRPM = 12000.0f;
	TorqueDecayRPM = 3000.0f;

	// Configure default engine torque curve for electric motor
	FRichCurve TorqueCurveData;
	TorqueCurveData.Keys.Add(FRichCurveKey(0.0f, 1480.0f));      // Peak torque at 0 RPM
	TorqueCurveData.Keys.Add(FRichCurveKey(3000.0f, 1480.0f));   // Constant to 3000 RPM
	TorqueCurveData.Keys.Add(FRichCurveKey(12000.0f, 0.0f));     // Decay to 0 at max RPM
	EngineSetup.TorqueCurve.EditorCurveData = TorqueCurveData;
	EngineSetup.MaxTorque = 1480.0f;
	EngineSetup.MaxRPM = 12000.0f;

	// Single-speed transmission (electric vehicles don't shift)
	TransmissionSetup.bUseAutomaticGears = true;
	TransmissionSetup.FinalRatio = 3.0f;
	TransmissionSetup.ForwardGearRatios = { 1.0f };
	TransmissionSetup.ReverseGearRatios = { 1.0f };

	// Default 4-wheel setup (FL, FR, RL, RR)
	WheelSetups.SetNum(4);
	WheelSetups[0].BoneName = NAME_None;
	WheelSetups[1].BoneName = NAME_None;
	WheelSetups[2].BoneName = NAME_None;
	WheelSetups[3].BoneName = NAME_None;

	// AWD: all wheels get drive torque
	DifferentialSetup.DifferentialType = EVehicleDifferential::AllWheelDrive;
}

void UNIOVehicleMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize tire model on play
	InitializeTireModel();

	// Apply the configured tire force scheme
	ApplyTireForceScheme();

	// Initialize debug velocity to avoid first-frame acceleration spike
	if (AActor* Owner = GetOwner())
	{
		const FVector InitVel = Owner->GetVelocity();
		ForceDebugPreviousVelocity = InitVel;
		CapturePreviousVelocity = InitVel;
	}

	UE_LOG(LogNomiVehicle, Log, TEXT("NIOVehicleMovementComponent BeginPlay, TireModel: %s, Scheme: %d, OverrideChaos: %s"),
		TireModel ? TEXT("Valid") : TEXT("Null"), (int32)TireForceScheme,
		bOverrideChaosTireWithPacejka ? TEXT("true") : TEXT("false"));
}

void UNIOVehicleMovementComponent::BeginDestroy()
{
	StopForceCapture();
	Super::BeginDestroy();
}

// ============================================================================
// Objective Metrics Tests (Iter 0 Tuning)
// ============================================================================

void UNIOVehicleMovementComponent::RunAllMetrics()
{
	UE_LOG(LogNomiVehicle, Log, TEXT("=== METRICS: Running all metrics for vehicle ==="));
	UE_LOG(LogNomiVehicle, Log, TEXT("Vehicle scheme: %d, ForwardSpeed: %.1f km/h"),
		(int32)TireForceScheme, GetForwardSpeed() * 0.036f);

	// Reset all metrics state
	MetricsState = FMetricsTestState();

	// Start with acceleration test (works from standstill)
	MetricsState.bAutoChain = true; // Auto-chain to braking after accel
	RunAccelerationTest();
}

void UNIOVehicleMovementComponent::RunAccelerationTest()
{
	MetricsState = FMetricsTestState();
	MetricsState.bActive = true;
	MetricsState.TestName = TEXT("0-100 km/h");
	if (UWorld* World = GetWorld())
	{
		MetricsState.StartTime = World->GetTimeSeconds();
	}
	MetricsState.StartSpeedKmh = GetForwardSpeed() * 0.036f;
	MetricsState.ResultUnit = TEXT("s");

	// Reset throttle/brake state for the test
	SetThrottleInput(1.0f);
	SetBrakeInput(0.0f);

	UE_LOG(LogNomiVehicle, Log, TEXT("=== METRICS: Starting 0-100 km/h acceleration test ==="));
}

void UNIOVehicleMovementComponent::RunBrakingTest()
{
	MetricsState = FMetricsTestState();
	MetricsState.bActive = true;
	MetricsState.TestName = TEXT("100-0 km/h");
	if (UWorld* World = GetWorld())
	{
		MetricsState.StartTime = World->GetTimeSeconds();
	}
	MetricsState.StartDistanceCm = 0.0f;
	MetricsState.StartSpeedKmh = GetForwardSpeed() * 0.036f;
	MetricsState.ResultUnit = TEXT("m");

	if (MetricsState.StartSpeedKmh < 100.0f)
	{
		UE_LOG(LogNomiVehicle, Warning, TEXT("=== METRICS: Braking test needs >100 km/h (current: %.1f km/h) ==="), MetricsState.StartSpeedKmh);
		MetricsState.bActive = false;
		return;
	}

	SetThrottleInput(0.0f);
	SetBrakeInput(1.0f);

	UE_LOG(LogNomiVehicle, Log, TEXT("=== METRICS: Starting 100-0 km/h braking test from %.1f km/h ==="), MetricsState.StartSpeedKmh);
}

FString UNIOVehicleMovementComponent::GetLastMetricsReport() const
{
	if (!MetricsState.bCompleted)
	{
		return TEXT("No completed metrics test.");
	}
	return FString::Printf(TEXT("%s: %.2f %s"), *MetricsState.TestName, MetricsState.ResultValue, *MetricsState.ResultUnit);
}

void UNIOVehicleMovementComponent::UpdateMetricsTest(float DeltaTime)
{
	if (!MetricsState.bActive || MetricsState.bCompleted)
	{
		return;
	}

	const float SpeedKmh = GetForwardSpeed() * 0.036f;
	UWorld* World = GetWorld();

	if (MetricsState.TestName == TEXT("0-100 km/h"))
	{
		// Ensure throttle is held at max for consistency
		SetThrottleInput(1.0f);
		SetBrakeInput(0.0f);

		if (SpeedKmh >= 100.0f)
		{
			const float Elapsed = (World ? World->GetTimeSeconds() : 0.0f) - MetricsState.StartTime;
			MetricsState.ResultValue = Elapsed;
			MetricsState.bCompleted = true;
			MetricsState.bActive = false;
			SetThrottleInput(0.0f);
			UE_LOG(LogNomiVehicle, Log, TEXT("=== METRICS RESULT: 0-100 km/h = %.2f s ==="), Elapsed);

			// Auto-chain to braking test if in full metrics mode
			if (MetricsState.bAutoChain)
			{
				RunBrakingTest();
			}
		}
		else if (SpeedKmh < 0.1f && (World ? World->GetTimeSeconds() : 0.0f) - MetricsState.StartTime > 5.0f)
		{
			// Timeout: vehicle didn't accelerate
			MetricsState.bCompleted = true;
			MetricsState.bActive = false;
			UE_LOG(LogNomiVehicle, Warning, TEXT("=== METRICS: 0-100 km/h test TIMEOUT (speed stuck at %.1f km/h) ==="), SpeedKmh);
		}
	}
	else if (MetricsState.TestName == TEXT("100-0 km/h"))
	{
		SetThrottleInput(0.0f);
		SetBrakeInput(1.0f);

		// Accumulate braking distance each frame (odometer tracking)
		MetricsState.StartDistanceCm += FMath::Abs(GetForwardSpeed()) * DeltaTime;

		if (SpeedKmh <= 0.5f)
		{
			// Braking complete: report accumulated distance in meters
			MetricsState.ResultValue = MetricsState.StartDistanceCm / 100.0f;
			MetricsState.bCompleted = true;
			MetricsState.bActive = false;
			SetBrakeInput(0.0f);
			UE_LOG(LogNomiVehicle, Log, TEXT("=== METRICS RESULT: 100-0 km/h braking = %.1f m ==="), MetricsState.ResultValue);
			if (MetricsState.bAutoChain)
			{
				UE_LOG(LogNomiVehicle, Log, TEXT("=== METRICS: All metrics complete ==="));
			}
		}
	}
}

void UNIOVehicleMovementComponent::ApplyTireForceScheme()
{
	// When using Default scheme, apply the production Config flags
	if (TireForceScheme == ENIOTireForceScheme::Default)
	{
		EnableWheelFriction(bChaosUseInternalVehiclePhysics);
		const float FrictionOverride = bOverrideChaosTireWithPacejka ? ChaosTireFrictionOverride : 1.0f;
		for (int32 i = 0; i < WheelSetups.Num(); i++)
		{
			SetWheelFrictionMultiplier(i, FrictionOverride);
		}
		UE_LOG(LogNomiVehicle, Log, TEXT("TireForceScheme = Default (Config): Override=%s, InternalPhysics=%s, Friction=%.4f"),
			bOverrideChaosTireWithPacejka ? TEXT("true") : TEXT("false"),
			bChaosUseInternalVehiclePhysics ? TEXT("true") : TEXT("false"),
			FrictionOverride);
		return;
	}

	switch (TireForceScheme)
	{
	case ENIOTireForceScheme::FrictionHack:
		// Scheme A: Keep Chaos running but reduce its tire friction to near zero.
		// Pacejka forces will dominate; Chaos solver still adds some CPU overhead.
		EnableWheelFriction(true);  // Keep Chaos solver running
		for (int32 i = 0; i < WheelSetups.Num(); i++)
		{
			SetWheelFrictionMultiplier(i, ChaosTireFrictionOverride);
		}
		UE_LOG(LogNomiVehicle, Log, TEXT("TireForceScheme = A (FrictionHack): Chaos friction=%.4f"),
			ChaosTireFrictionOverride);
		break;

	case ENIOTireForceScheme::NoInternalPhysics:
		// Scheme B: Skip Chaos internal tire solver entirely.
		// bUseInternalVehiclePhysics=false → Chaos skips its tire integration.
		// Only Pacejka forces from ApplyTireForces will affect the vehicle.
		EnableWheelFriction(false);
		// Restore default friction in case we switched from Scheme A/C
		for (int32 i = 0; i < WheelSetups.Num(); i++)
		{
			SetWheelFrictionMultiplier(i, 1.0f);
		}
		UE_LOG(LogNomiVehicle, Log, TEXT("TireForceScheme = B (NoInternalPhysics): bUseInternalVehiclePhysics=false"));
		break;

	case ENIOTireForceScheme::Both:
		// Scheme C: Double safety — disable internal physics AND set friction to near zero.
		EnableWheelFriction(false);
		for (int32 i = 0; i < WheelSetups.Num(); i++)
		{
			SetWheelFrictionMultiplier(i, ChaosTireFrictionOverride);
		}
		UE_LOG(LogNomiVehicle, Log, TEXT("TireForceScheme = C (Both): bUseInternalVehiclePhysics=false + friction=%.4f"),
			ChaosTireFrictionOverride);
		break;

	default:
		// Safety fallback: restore Chaos defaults
		EnableWheelFriction(true);
		for (int32 i = 0; i < WheelSetups.Num(); i++)
		{
			SetWheelFrictionMultiplier(i, 1.0f);
		}
		UE_LOG(LogNomiVehicle, Log, TEXT("TireForceScheme = unknown (%d), falling back to Chaos defaults"),
			(int32)TireForceScheme);
		break;
	}
}

void UNIOVehicleMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// NOTE: Super::TickComponent runs the Chaos vehicle physics simulation first.
	// When bOverrideChaosTireWithPacejka is true (production mode):
	//   - Chaos tire friction is pinned to near 0 each frame via SetTireFriction
	//   - bChaosUseInternalVehiclePhysics=false skips the internal tire solver
	//   - Only ApplyTireForces() Pacejka forces affect the vehicle
	// When diagnostics are active (TireForceScheme != Default):
	//   - ApplyTireForceScheme() controls the exact isolation strategy
	// See: ApplyTireForceScheme(), ApplyTireForces()
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Re-apply friction override if we're in production override mode
	// This is defensive: Chaos may reset friction internally between frames
	if (bOverrideChaosTireWithPacejka && TireForceScheme == ENIOTireForceScheme::Default)
	{
		for (int32 i = 0; i < WheelSetups.Num(); i++)
		{
			SetWheelFrictionMultiplier(i, ChaosTireFrictionOverride);
		}
	}

	// Update electric vehicle systems each frame
	float ThrottleInput = GetThrottleInput();
	float BrakeInput = GetBrakeInput();

	UpdateMotorTemperature(DeltaTime, ThrottleInput);
	UpdateBatteryLevel(DeltaTime, ThrottleInput, BrakeInput);
	ApplyRegenerativeBraking(DeltaTime);
	ApplyAerodynamicDownforce(DeltaTime);

	// Update active metrics test (if any)
	UpdateMetricsTest(DeltaTime);

	// Update tire model if available
	if (TireModel)
	{
		ApplyTireForces(DeltaTime);

		// Debug force logging (throttled)
		if (bEnableTireForceDebugLog)
		{
			ForceDebugFrameCounter++;
			if (ForceDebugLogInterval <= 0 || (ForceDebugFrameCounter % ForceDebugLogInterval) == 0)
			{
				LogTireForces(DeltaTime);
			}
		}

		// CSV data capture
		if (bEnableForceDataCapture && ForceCaptureFile)
		{
			CaptureFrameIndex++;
			WriteForceCaptureFrame(DeltaTime);
		}

		// Debug force vector visualization
		if (bDrawDebugForceVectors)
		{
			DrawDebugForceVectors();
		}
	}
}

void UNIOVehicleMovementComponent::InitializeTireModel()
{
	if (TireModel)
	{
		return; // Already initialized
	}

	// Create tire physics model component
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	TireModel = NewObject<UTirePhysicsModel>(Owner, TEXT("TirePhysicsModel"));
	if (!TireModel)
	{
		NomiError::Log(ENomiErrorSeverity::Error, TEXT("Vehicle"), TEXT("Failed to create TirePhysicsModel"));
		return;
	}
	TireModel->RegisterComponent();

	// Apply appropriate presets based on vehicle type
	// Default to EP9 preset, can be overridden via ConfigureForNIOVehicle
	FrontTirePreset = UNIOTirePresets::CreateEP9FrontPreset();
	RearTirePreset = UNIOTirePresets::CreateEP9RearPreset();

	// Apply front preset as baseline (covers common parameters)
	TireModel->ApplyPreset(FrontTirePreset);

	// The tire model uses front preset coefficients by default.
	// Rear axle forces are differentiated in ApplyTireForces via grip scaling
	// based on the rear tire preset's friction coefficient.

	UE_LOG(LogNomiVehicle, Log, TEXT("Tire physics model initialized with front/rear presets"));
}

void UNIOVehicleMovementComponent::ApplyTireForces(float DeltaTime)
{
	if (!TireModel)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UPrimitiveComponent* VehicleBody = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
	if (!VehicleBody)
	{
		return;
	}

	const FTransform VehicleTransform = Owner->GetActorTransform();
	const TArray<FTireState>& TireStates = TireModel->GetAllTireStates();

	// Apply rear tire grip scaling based on rear preset characteristics
	const float RearFrictionRatio = (FrontTirePreset.BaseFriction > 0.001f)
		? RearTirePreset.BaseFriction / FrontTirePreset.BaseFriction
		: 1.0f;

	// Vehicle direction vectors are constant per frame — hoist above loop
	const FVector VehicleForward = Owner->GetActorForwardVector();
	const FVector VehicleRight = Owner->GetActorRightVector();

	// Process each wheel and apply forces to the vehicle body
	for (int32 i = 0; i < FMath::Min(TireStates.Num(), 4); i++)
	{
		const FTireState& State = TireStates[i];

		if (!State.bIsGrounded || State.WheelLoad <= 0.0f)
		{
			continue;
		}

		// Get wheel offset in local space
		const FVector WheelLocalPos = TireModel->GetWheelOffset(i);
		const FVector WheelWorldPos = VehicleTransform.TransformPosition(WheelLocalPos);

		// Get tire forces (already calculated by TirePhysicsModel in TickComponent)
		float LongForceN = State.LongitudinalForce;
		float LatForceN = State.LateralForce;

		// Apply rear friction scaling for rear wheels
		if (i >= 2)
		{
			LongForceN *= RearFrictionRatio;
			LatForceN *= RearFrictionRatio;
		}

		// Convert forces from Newtons to UE units (kg*cm/s^2)
		// 1 N = 1 kg*m/s^2 = 100 kg*cm/s^2
		const float ForceScale = 100.0f;
		LongForceN *= ForceScale;
		LatForceN *= ForceScale;

		// Calculate steering angle for front wheels
		float SteerAngleDeg = 0.0f;
		if (i <= 1) // Front wheels
		{
			SteerAngleDeg = GetSteeringInput() * 30.0f; // Max ~30 degrees
		}

		// Rotate wheel forward direction by steering angle
		const FQuat SteerRotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(SteerAngleDeg));
		const FVector WheelForward = SteerRotation.RotateVector(VehicleForward);
		const FVector WheelRight = SteerRotation.RotateVector(VehicleRight);

		// Calculate force vectors in world space
		// Longitudinal force acts along wheel forward direction
		// Lateral force acts along wheel right direction (perpendicular to forward)
		const FVector LongForceVector = WheelForward * LongForceN;
		const FVector LatForceVector = WheelRight * LatForceN;
		const FVector TotalForce = LongForceVector + LatForceVector;

		// Apply force at wheel position (creates both linear and angular impulse)
		VehicleBody->AddForceAtLocation(TotalForce, WheelWorldPos, NAME_None);
	}
}

void UNIOVehicleMovementComponent::LogTireForces(float DeltaTime)
{
	if (!TireModel)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Compute vehicle acceleration from velocity delta
	const FVector CurrentVelocity = Owner->GetVelocity();
	const FVector Accel = (CurrentVelocity - ForceDebugPreviousVelocity) / FMath::Max(DeltaTime, 0.001f);
	ForceDebugPreviousVelocity = CurrentVelocity;

	const TArray<FTireState>& TireStates = TireModel->GetAllTireStates();
	const int32 WheelCount = FMath::Min(TireStates.Num(), 4);

	const float ForwardSpeedCmS = GetForwardSpeed();
	const FVector VehicleForward = Owner->GetActorForwardVector();
	const float ForwardAccelCmS2 = FVector::DotProduct(Accel, VehicleForward);

	// Prepare snapshot
	ForceDebugSnapshot.ResizeForWheelCount(WheelCount);
	ForceDebugSnapshot.ForwardSpeedCmS = ForwardSpeedCmS;
	ForceDebugSnapshot.ForwardAccelCmS2 = ForwardAccelCmS2;

	float TotalPacejkaLong = 0.0f;
	float TotalPacejkaLat = 0.0f;

	UE_LOG(LogNomiVehicle, Verbose, TEXT("=== Tire Force Debug [Frame %d] ==="), ForceDebugFrameCounter);
	UE_LOG(LogNomiVehicle, Verbose, TEXT("Vehicle: Speed=%.1f cm/s (%.1f km/h), ForwardAccel=%.1f cm/s^2"),
		ForwardSpeedCmS, ForwardSpeedCmS * 0.036f, ForwardAccelCmS2);

	for (int32 i = 0; i < WheelCount; i++)
	{
		const FTireState& State = TireStates[i];
		FWheelForceDebugEntry& Entry = ForceDebugSnapshot.WheelEntries[i];

		Entry.bGrounded = State.bIsGrounded;
		Entry.SlipRatio = State.SlipRatio;
		Entry.SlipAngleDeg = State.SlipAngleDeg;
		Entry.WheelLoad = State.WheelLoad;
		Entry.PacejkaLongForce = State.LongitudinalForce;
		Entry.PacejkaLatForce = State.LateralForce;

		if (!State.bIsGrounded)
		{
			continue;
		}

		// Apply force scaling (same as ApplyTireForces does)
		const float RearFrictionRatio = (FrontTirePreset.BaseFriction > 0.001f)
			? RearTirePreset.BaseFriction / FrontTirePreset.BaseFriction
			: 1.0f;
		float ScaledLong = State.LongitudinalForce * 100.0f;
		float ScaledLat = State.LateralForce * 100.0f;
		if (i >= 2) // Rear wheels
		{
			ScaledLong *= RearFrictionRatio;
			ScaledLat *= RearFrictionRatio;
		}
		TotalPacejkaLong += ScaledLong;
		TotalPacejkaLat += ScaledLat;

		UE_LOG(LogNomiVehicle, Verbose, TEXT("  Wheel[%d] %s: SR=%.3f SA=%.1fdeg Load=%.0f N | Pacejka: Long=%.1f Lat=%.1f"),
			i, State.bIsGrounded ? TEXT("GRD") : TEXT("AIR"),
			State.SlipRatio, State.SlipAngleDeg, State.WheelLoad,
			State.LongitudinalForce, State.LateralForce);
	}

	ForceDebugSnapshot.TotalPacejkaLongForce = TotalPacejkaLong;
	ForceDebugSnapshot.TotalPacejkaLatForce = TotalPacejkaLat;

	UE_LOG(LogNomiVehicle, Verbose, TEXT("  Total Pacejka applied: Long=%.1f Lat=%.1f (UE force units)"),
		TotalPacejkaLong, TotalPacejkaLat);
	UE_LOG(LogNomiVehicle, Verbose, TEXT("  Inferred total vehicle accel: %.1f cm/s^2 (%.2f G)"),
		ForwardAccelCmS2, ForwardAccelCmS2 / 980.0f);
	UE_LOG(LogNomiVehicle, Verbose, TEXT("========================================"));
}

void UNIOVehicleMovementComponent::StartForceCapture(const FString& ScenarioLabel)
{
	// Close any previous capture
	StopForceCapture();

	CaptureScenarioLabel = ScenarioLabel;
	CaptureFrameIndex = 0;
	if (AActor* Owner = GetOwner())
	{
		CapturePreviousVelocity = Owner->GetVelocity();
	}

	// Create CSV file in project's saved directory
	const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	const FString FileName = FString::Printf(TEXT("ForceCapture_%s_%s.csv"), *ScenarioLabel, *Timestamp);
	const FString FilePath = FPaths::ProjectSavedDir() / TEXT("ForceCaptures") / FileName;

	// Ensure directory exists
	IFileManager& FileManager = IFileManager::Get();
	const FString DirPath = FPaths::GetPath(FilePath);
	FileManager.MakeDirectory(*DirPath, true);

	ForceCaptureFile = FileManager.CreateFileWriter(*FilePath, FILEWRITE_NoFail);
	if (!ForceCaptureFile)
	{
		UE_LOG(LogNomiVehicle, Error, TEXT("Failed to create force capture file: %s"), *FilePath);
		return;
	}

	// Write CSV header
	FString Header = TEXT("Frame,Scheme,SpeedCmS,AccelCmS2,");
	for (int32 i = 0; i < 4; i++)
	{
		Header += FString::Printf(TEXT("W%d_GRD,W%d_SR,W%d_SA,W%d_Load,W%d_PacejkaLong,W%d_PacejkaLat,"),
			i, i, i, i, i, i);
	}
	Header += TEXT("TotalPacejkaLong,TotalPacejkaLat\n");
	FTCHARToUTF8 Converter(*Header);
	ForceCaptureFile->Serialize((void*)Converter.Get(), Converter.Length());

	UE_LOG(LogNomiVehicle, Log, TEXT("=== Force capture STARTED: %s -> %s"), *ScenarioLabel, *FilePath);
}

void UNIOVehicleMovementComponent::StopForceCapture()
{
	if (ForceCaptureFile)
	{
		ForceCaptureFile->Close();
		delete ForceCaptureFile;
		ForceCaptureFile = nullptr;

		UE_LOG(LogNomiVehicle, Log, TEXT("=== Force capture STOPPED: %s (%d frames)"),
			*CaptureScenarioLabel, CaptureFrameIndex);
		CaptureScenarioLabel.Empty();
	}
}

void UNIOVehicleMovementComponent::WriteForceCaptureFrame(float DeltaTime)
{
	if (!ForceCaptureFile || !TireModel)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner) return;

	const TArray<FTireState>& TireStates = TireModel->GetAllTireStates();
	const int32 WheelCount = FMath::Min(TireStates.Num(), 4);

	// Compute acceleration (separate from LogTireForces to avoid shared-state race)
	const FVector CurrentVelocity = Owner->GetVelocity();
	const FVector Accel = (CurrentVelocity - CapturePreviousVelocity) / FMath::Max(DeltaTime, 0.001f);
	CapturePreviousVelocity = CurrentVelocity;
	const float ForwardSpeed = GetForwardSpeed();
	const float ForwardAccel = FVector::DotProduct(Accel, Owner->GetActorForwardVector());

	// Compute rear friction ratio
	const float RearFrictionRatio = (FrontTirePreset.BaseFriction > 0.001f)
		? RearTirePreset.BaseFriction / FrontTirePreset.BaseFriction
		: 1.0f;

	FString Line = FString::Printf(TEXT("%d,%d,%.1f,%.1f,"),
		CaptureFrameIndex, (int32)TireForceScheme, ForwardSpeed, ForwardAccel);

	float TotalLong = 0.0f, TotalLat = 0.0f;

	for (int32 i = 0; i < WheelCount; i++)
	{
		const FTireState& State = TireStates[i];
		float ScaledLong = State.LongitudinalForce * 100.0f;
		float ScaledLat = State.LateralForce * 100.0f;
		if (i >= 2) { ScaledLong *= RearFrictionRatio; ScaledLat *= RearFrictionRatio; }
		TotalLong += ScaledLong;
		TotalLat += ScaledLat;

		Line += FString::Printf(TEXT("%d,%.4f,%.2f,%.0f,%.1f,%.1f,"),
			State.bIsGrounded ? 1 : 0,
			State.SlipRatio, State.SlipAngleDeg, State.WheelLoad,
			ScaledLong, ScaledLat);
	}

	// Pad missing wheels (if fewer than 4)
	for (int32 i = WheelCount; i < 4; i++)
	{
		Line += TEXT("0,0,0,0,0,0,");
	}

	Line += FString::Printf(TEXT("%.1f,%.1f\n"), TotalLong, TotalLat);

	FTCHARToUTF8 Converter(*Line);
	ForceCaptureFile->Serialize((void*)Converter.Get(), Converter.Length());
}

void UNIOVehicleMovementComponent::DrawDebugForceVectors()
{
#if ENABLE_DRAW_DEBUG
	if (!TireModel)
	{
		return;
	}

	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return;
	}

	const FTransform VehicleTransform = Owner->GetActorTransform();
	const TArray<FTireState>& AllStates = TireModel->GetAllTireStates();
	const int32 WheelCount = FMath::Min(AllStates.Num(), 4);

	// Vehicle direction vectors (same frame as ApplyTireForces)
	const FVector VehicleForward = Owner->GetActorForwardVector();
	const FVector VehicleRight = Owner->GetActorRightVector();

	// Force scaling for visualization (UE units to draw size)
	constexpr float ForceDrawScale = 0.001f;
	constexpr float MinArrowLength = 10.0f;
	constexpr float MaxArrowLength = 300.0f;

	// Compute rear friction ratio (same as ApplyTireForces for consistent visualization)
	const float RearFrictionRatio = (FrontTirePreset.BaseFriction > 0.001f)
		? RearTirePreset.BaseFriction / FrontTirePreset.BaseFriction
		: 1.0f;

	for (int32 i = 0; i < WheelCount; i++)
	{
		const FTireState& State = AllStates[i];
		const FVector WheelLocalPos = TireModel->GetWheelOffset(i);
		const FVector WheelWorldPos = VehicleTransform.TransformPosition(WheelLocalPos);

		if (!State.bIsGrounded)
		{
			// Draw small yellow dot for airborne wheels
			DrawDebugPoint(World, WheelWorldPos, 8.0f, FColor::Yellow, false, -1.0f, SDPG_Foreground);
			continue;
		}

		// Apply rear friction ratio scaling (same as ApplyTireForces)
		const float FrictionScale = (i >= 2) ? RearFrictionRatio : 1.0f;
		const float ScaledLongForce = State.LongitudinalForce * FrictionScale;
		const float ScaledLatForce = State.LateralForce * FrictionScale;

		// Calculate steering angle (same as ApplyTireForces)
		float SteerAngleDeg = 0.0f;
		if (i <= 1)
		{
			SteerAngleDeg = GetSteeringInput() * 30.0f;
		}
		const FQuat SteerRotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(SteerAngleDeg));
		const FVector WheelForward = SteerRotation.RotateVector(VehicleForward);
		const FVector WheelRight = SteerRotation.RotateVector(VehicleRight);

		// Scale forces for visualization
		const float LongDrawLen = FMath::Clamp(FMath::Abs(ScaledLongForce) * ForceDrawScale, MinArrowLength, MaxArrowLength);
		const float LatDrawLen = FMath::Clamp(FMath::Abs(ScaledLatForce) * ForceDrawScale, MinArrowLength, MaxArrowLength);

		// Longitudinal force arrow: Red (forward) / DarkRed (backward)
		const FVector LongDir = ScaledLongForce >= 0.0f ? WheelForward : -WheelForward;
		const FColor LongColor = ScaledLongForce >= 0.0f ? FColor::Red : FColor(128, 0, 0);
		DrawDebugDirectionalArrow(World, WheelWorldPos, WheelWorldPos + LongDir * LongDrawLen, LongDrawLen * 0.3f, LongColor, false, -1.0f, SDPG_Foreground);

		// Lateral force arrow: Blue (right) / DarkBlue (left)
		const FVector LatDir = ScaledLatForce >= 0.0f ? WheelRight : -WheelRight;
		const FColor LatColor = ScaledLatForce >= 0.0f ? FColor::Blue : FColor(0, 0, 128);
		DrawDebugDirectionalArrow(World, WheelWorldPos, WheelWorldPos + LatDir * LatDrawLen, LatDrawLen * 0.3f, LatColor, false, -1.0f, SDPG_Foreground);

		// Wheel load indicator: green line downward (length proportional to load)
		const float LoadDrawLen = FMath::Clamp(State.WheelLoad * 0.005f, 10.0f, 150.0f);
		DrawDebugLine(World, WheelWorldPos, WheelWorldPos - FVector(0, 0, LoadDrawLen), FColor::Green, false, -1.0f, SDPG_Foreground);
	}

	// Draw vehicle forward vector for reference (white)
	const FVector VehiclePos = Owner->GetActorLocation();
	DrawDebugDirectionalArrow(World, VehiclePos, VehiclePos + VehicleForward * 100.0f, 30.0f, FColor::White, false, -1.0f, SDPG_Foreground);
#endif // ENABLE_DRAW_DEBUG
}

const FTireState& UNIOVehicleMovementComponent::GetTireState(int32 WheelIndex) const
{
	static const FTireState EmptyState;
	if (TireModel)
	{
		return TireModel->GetTireState(WheelIndex);
	}
	return EmptyState;
}

bool UNIOVehicleMovementComponent::IsAnyTireSlipping(float Threshold) const
{
	if (TireModel)
	{
		return TireModel->IsAnyTireSlipping(Threshold);
	}
	return false;
}

const FTireEffectsState& UNIOVehicleMovementComponent::GetTireEffectsState() const
{
	if (!TireModel)
	{
		return CachedEffectsState;
	}

	const TArray<FTireState>& AllStates = TireModel->GetAllTireStates();
	const int32 WheelCount = AllStates.Num();

	// Resize arrays only if needed (avoid repeated allocations)
	CachedEffectsState.ResizeForWheelCount(WheelCount);

	float TotalTemp = 0.0f;
	float MaxSlipRatio = 0.0f;
	float MaxSlipAngle = 0.0f;
	bool bAnySlipping = false;

	for (int32 i = 0; i < WheelCount; i++)
	{
		const FTireState& State = AllStates[i];

		CachedEffectsState.WheelGrounded[i] = State.bIsGrounded;
		CachedEffectsState.WheelSlipRatios[i] = State.SlipRatio;
		CachedEffectsState.WheelSlipAngles[i] = State.SlipAngleDeg;
		CachedEffectsState.WheelTemperatures[i] = State.Thermal.SurfaceTemperature;

		// Track maximums for aggregated effects
		const float AbsSlipRatio = FMath::Abs(State.SlipRatio);
		if (AbsSlipRatio > MaxSlipRatio)
		{
			MaxSlipRatio = AbsSlipRatio;
		}

		const float AbsSlipAngle = FMath::Abs(State.SlipAngleDeg);
		if (AbsSlipAngle > MaxSlipAngle)
		{
			MaxSlipAngle = AbsSlipAngle;
		}

		TotalTemp += State.Thermal.SurfaceTemperature;

		// Check if this wheel is slipping beyond threshold
		if (AbsSlipRatio > 0.15f || AbsSlipAngle > 10.0f)
		{
			bAnySlipping = true;
		}
	}

	// Set aggregated values
	CachedEffectsState.bAnyTireSlipping = bAnySlipping;
	CachedEffectsState.MaxSlipRatio = MaxSlipRatio;
	CachedEffectsState.MaxSlipAngleDeg = MaxSlipAngle;
	CachedEffectsState.AverageTireTemperature = (WheelCount > 0) ? (TotalTemp / WheelCount) : 25.0f;

	return CachedEffectsState;
}

float UNIOVehicleMovementComponent::GetElectricMotorTorque(float CurrentRPM) const
{
	return CalculateElectricTorqueCurve(CurrentRPM);
}

float UNIOVehicleMovementComponent::CalculateElectricTorqueCurve(float RPM) const
{
	// Electric motor torque curve:
	// - 0 to TorqueDecayRPM: constant peak torque
	// - TorqueDecayRPM to MaxMotorRPM: linear decay to 0
	// Note: Uses GetEffectiveMotorTorque() to apply battery level without mutating PeakMotorTorque

	const float EffectiveTorque = GetEffectiveMotorTorque();

	if (RPM <= 0.0f)
	{
		// Static start: full torque
		return EffectiveTorque;
	}

	if (RPM <= TorqueDecayRPM)
	{
		// Constant torque region
		return EffectiveTorque;
	}

	if (RPM >= MaxMotorRPM)
	{
		// Beyond max RPM: no torque
		return 0.0f;
	}

	// Linear decay region
	float DecayFactor = 1.0f - ((RPM - TorqueDecayRPM) / (MaxMotorRPM - TorqueDecayRPM));
	return EffectiveTorque * DecayFactor;
}

float UNIOVehicleMovementComponent::GetRegenerativeBrakingTorque() const
{
	// Regenerative braking torque is proportional to vehicle speed
	float SpeedKmh = GetForwardSpeed() * 0.036f; // cm/s to km/h

	if (SpeedKmh < 5.0f)
	{
		// No regen at very low speeds
		return 0.0f;
	}

	// Max regen torque is a fraction of peak motor torque
	float MaxRegenTorque = PeakMotorTorque * RegenBrakingStrength;

	// Scale with speed (more regen at higher speeds)
	float SpeedFactor = FMath::Clamp(SpeedKmh / 100.0f, 0.0f, 1.0f);

	return MaxRegenTorque * SpeedFactor;
}

void UNIOVehicleMovementComponent::ApplyRegenerativeBraking(float DeltaTime)
{
	float BrakeInput = GetBrakeInput();

	if (BrakeInput > 0.01f)
	{
		float RegenTorque = GetRegenerativeBrakingTorque() * BrakeInput;

		// Apply regen as engine braking when throttle is released
		if (GetThrottleInput() < 0.01f)
		{
			// Apply reverse torque to all wheels to simulate regenerative braking
			// This creates a natural deceleration effect
			float SpeedKmh = GetForwardSpeed() * 0.036f; // cm/s to km/h
			if (SpeedKmh > 1.0f) // Only apply when moving
			{
				// Reduce motor RPM to simulate energy recovery
				// The Chaos vehicle system handles the actual physics
				// Apply regenerative braking torque via additional brake force
				SetBrakeInput(FMath::Clamp(BrakeInput + (RegenTorque / PeakMotorTorque) * 0.1f, 0.0f, 1.0f));
			}
		}
	}
}

void UNIOVehicleMovementComponent::UpdateMotorTemperature(float DeltaTime, float ThrottleInput)
{
	// Heat generation: proportional to throttle input
	float HeatGeneration = ThrottleInput * MotorHeatRate * DeltaTime;

	// Cooling: always active, proportional to speed
	float SpeedKmh = GetForwardSpeed() * 0.036f;
	float CoolingFactor = FMath::Clamp(SpeedKmh / 200.0f, 0.2f, 1.0f);
	float Cooling = MotorCoolRate * CoolingFactor * DeltaTime;

	// Update temperature
	MotorTemperature = FMath::Clamp(MotorTemperature + HeatGeneration - Cooling, 20.0f, 100.0f);
}

void UNIOVehicleMovementComponent::UpdateBatteryLevel(float DeltaTime, float ThrottleInput, float RegenInput)
{
	// Battery discharge
	float Discharge = ThrottleInput * BatteryDischargeRate * DeltaTime;

	// Battery regeneration
	float Regen = RegenInput * BatteryRegenRate * DeltaTime;

	// Update battery level
	BatteryLevel = FMath::Clamp(BatteryLevel - Discharge + Regen, 0.0f, 100.0f);
}

float UNIOVehicleMovementComponent::GetEffectiveMotorTorque() const
{
	// Returns motor torque with battery level applied (without mutating PeakMotorTorque)
	float EffectiveTorque = PeakMotorTorque;

	// Reduce power output when battery is very low
	if (BatteryLevel < 10.0f)
	{
		float PowerReduction = BatteryLevel / 10.0f;
		EffectiveTorque *= PowerReduction;
	}

	return EffectiveTorque;
}

namespace AeroConstants
{
	constexpr float CmPerSecToMetersPerSec = 0.01f;  // 1 cm/s = 0.01 m/s
	constexpr float AirDensityKgPerM3 = 1.225f;      // kg/m^3 at sea level
	constexpr float GravityCmPerSec2 = 980.0f;        // cm/s^2
	constexpr float MinSpeedForAeroMs = 1.0f;         // m/s
}

void UNIOVehicleMovementComponent::ApplyAerodynamicDownforce(float DeltaTime)
{
	if (NIODownforceCoefficient <= 0.0f)
	{
		return;
	}

	float SpeedMs = GetForwardSpeed() * AeroConstants::CmPerSecToMetersPerSec;

	if (SpeedMs < AeroConstants::MinSpeedForAeroMs)
	{
		return;
	}

	// Downforce = 0.5 * rho * v^2 * A * Cl
	// Where: rho = air density, v = speed, A = frontal area, Cl = downforce coefficient
	float DynamicPressure = 0.5f * AeroConstants::AirDensityKgPerM3 * SpeedMs * SpeedMs;
	float DownforceNewtons = DynamicPressure * FrontalArea * NIODownforceCoefficient;

	// Convert Newtons to UE force units (kg*cm/s^2)
	// 1 Newton = 1 kg*m/s^2 = 100 kg*cm/s^2 (UE uses cm as base unit)
	float DownforceUE = DownforceNewtons * 100.0f;

	// Apply downforce as additional gravity
	if (AActor* Owner = GetOwner())
	{
		if (UPrimitiveComponent* VehicleBody = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
		{
			FVector DownforceVector = FVector(0.0f, 0.0f, -DownforceUE);
			VehicleBody->AddForce(DownforceVector);
		}
	}
}

void UNIOVehicleMovementComponent::ConfigureForNIOVehicle(ENIOVehicleType VehicleType)
{
	switch (VehicleType)
	{
	case ENIOVehicleType::EP9:
		// Hypercar configuration
		PeakMotorTorque = 1480.0f;  // 4 motors combined
		TorqueDecayRPM = 4000.0f;
		MaxMotorRPM = 15000.0f;
		RegenBrakingStrength = 0.4f;
		NIODownforceCoefficient = 3.5f;  // High downforce
		NIODragCoefficient = 0.30f;
		FrontalArea = 2.0f;
		BatteryDischargeRate = 0.08f;  // Higher drain for more power

		// Apply EP9 tire presets (semi-slick performance tires)
		FrontTirePreset = UNIOTirePresets::CreateEP9FrontPreset();
		RearTirePreset = UNIOTirePresets::CreateEP9RearPreset();
		if (TireModel)
		{
			TireModel->ApplyPreset(FrontTirePreset);
		}
		break;

	case ENIOVehicleType::ET7:
		// Luxury sedan configuration
		PeakMotorTorque = 850.0f;
		TorqueDecayRPM = 3500.0f;
		MaxMotorRPM = 12000.0f;
		RegenBrakingStrength = 0.3f;
		NIODownforceCoefficient = 0.5f;
		NIODragCoefficient = 0.23f;  // Very aerodynamic
		FrontalArea = 2.3f;
		BatteryDischargeRate = 0.04f;

		// Apply ET7 tire presets (touring performance tires)
		FrontTirePreset = UNIOTirePresets::CreateET7FrontPreset();
		RearTirePreset = UNIOTirePresets::CreateET7RearPreset();
		if (TireModel)
		{
			TireModel->ApplyPreset(FrontTirePreset);
		}
		break;

	case ENIOVehicleType::ES7:
		// SUV configuration
		PeakMotorTorque = 850.0f;
		TorqueDecayRPM = 3000.0f;
		MaxMotorRPM = 12000.0f;
		RegenBrakingStrength = 0.25f;
		NIODownforceCoefficient = 0.3f;
		NIODragCoefficient = 0.32f;  // Higher drag (SUV shape)
		FrontalArea = 2.8f;
		BatteryDischargeRate = 0.05f;

		// Apply ES7 tire presets (SUV performance tires)
		FrontTirePreset = UNIOTirePresets::CreateES7FrontPreset();
		RearTirePreset = UNIOTirePresets::CreateES7RearPreset();
		if (TireModel)
		{
			TireModel->ApplyPreset(FrontTirePreset);
		}
		break;

	case ENIOVehicleType::ET5:
		// Sport sedan configuration
		PeakMotorTorque = 700.0f;
		TorqueDecayRPM = 3500.0f;
		MaxMotorRPM = 12000.0f;
		RegenBrakingStrength = 0.3f;
		NIODownforceCoefficient = 0.4f;
		NIODragCoefficient = 0.24f;
		FrontalArea = 2.2f;
		BatteryDischargeRate = 0.04f;

		// Apply ET5 tire presets (sport sedan tires)
		FrontTirePreset = UNIOTirePresets::CreateET5FrontPreset();
		RearTirePreset = UNIOTirePresets::CreateET5RearPreset();
		if (TireModel)
		{
			TireModel->ApplyPreset(FrontTirePreset);
		}
		break;

	case ENIOVehicleType::SU7Ultra:
		// Xiaomi SU7 Ultra - Super Sedan
		PeakMotorTorque = 1770.0f;  // 1548 HP
		TorqueDecayRPM = 5000.0f;
		MaxMotorRPM = 16000.0f;
		RegenBrakingStrength = 0.35f;
		NIODownforceCoefficient = 2.8f;
		NIODragCoefficient = 0.25f;
		FrontalArea = 2.1f;
		BatteryDischargeRate = 0.09f;

		// Use EP9 tire presets as high-performance default
		FrontTirePreset = UNIOTirePresets::CreateEP9FrontPreset();
		RearTirePreset = UNIOTirePresets::CreateEP9RearPreset();
		if (TireModel)
		{
			TireModel->ApplyPreset(FrontTirePreset);
		}
		break;

	default:
		// Default to ET7 configuration
		PeakMotorTorque = 850.0f;
		TorqueDecayRPM = 3500.0f;
		MaxMotorRPM = 12000.0f;
		RegenBrakingStrength = 0.3f;
		NIODownforceCoefficient = 0.5f;
		NIODragCoefficient = 0.23f;
		FrontalArea = 2.3f;
		BatteryDischargeRate = 0.04f;
		break;
	}

	UE_LOG(LogNomiVehicle, Log, TEXT("Configured for NIO vehicle type: %d"), (int32)VehicleType);
}
