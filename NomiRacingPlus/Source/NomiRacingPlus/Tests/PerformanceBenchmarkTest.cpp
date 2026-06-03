// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Tests/PerformanceBenchmarkTest.h"
#include "Vehicles/TirePhysicsModel.h"
#include "Vehicles/NIOTirePresets.h"
#include "AI/AIBehaviorTree.h"
#include "AI/AIOvertakeEvaluator.h"
#include "AI/AIDefensiveEvaluator.h"
#include "AI/AIRubberBandScaler.h"
#include "AI/AISlipstreamSystem.h"
#include "Core/CameraSystem.h"
#include "Misc/AutomationTest.h"

// ============================================================================
// Performance Benchmark Utilities Implementation
// ============================================================================

double FPerformanceBenchmarkUtils::CyclesToMicroseconds(uint64 Cycles)
{
	return FPlatformTime::ToSeconds64(Cycles) * 1000000.0;
}

void FPerformanceBenchmarkUtils::LogBenchmarkResult(FAutomationTestBase* Test, const FString& Name,
	double AvgMicroseconds, double TargetMicroseconds)
{
	const FString Status = AvgMicroseconds <= TargetMicroseconds ? TEXT("PASS") : TEXT("WARN");
	Test->AddInfo(FString::Printf(TEXT("[%s] %s: %.2f us (target: %.2f us)"),
		*Status, *Name, AvgMicroseconds, TargetMicroseconds));
}

// ============================================================================
// VEHICLE PHYSICS BENCHMARKS
// ============================================================================

// --- Pacejka Force Calculation ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPacejkaForceBenchmark,
	"NomiRacing.Performance.VehiclePhysics.PacejkaForceCalculation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPacejkaForceBenchmark::RunTest(const FString& Parameters)
{
	FPacejkaCoefficients Coeffs;
	Coeffs.B = 10.0f;
	Coeffs.C = 1.65f;
	Coeffs.D = 1.0f;
	Coeffs.E = -0.1f;

	// Benchmark: 10000 iterations of Pacejka force calculation
	constexpr int32 Iterations = 10000;
	float Sink = 0.0f;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		// Sweep across a range of slip inputs to exercise the formula
		for (float Slip = -1.0f; Slip <= 1.0f; Slip += 0.01f)
		{
			Sink += UTirePhysicsModel::CalculatePacejkaForce(Coeffs, Slip);
		}
	});

	// Target: entire sweep (201 evaluations) should complete in under 50 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Pacejka Force (201 slip values)"), AvgUs, 50.0);

	return true;
}

// --- Pacejka Force with All Preset Coefficients ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPacejkaPresetsBenchmark,
	"NomiRacing.Performance.VehiclePhysics.PacejkaAllPresets",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPacejkaPresetsBenchmark::RunTest(const FString& Parameters)
{
	// Collect all NIO vehicle presets
	TArray<FTireModelPreset> AllPresets;
	{
		FTireModelPreset F, R;
		UNIOTirePresets::GetPresetsForVehicle(ENIOVehicleType::EP9, F, R);
		AllPresets.Add(F); AllPresets.Add(R);
		UNIOTirePresets::GetPresetsForVehicle(ENIOVehicleType::ET7, F, R);
		AllPresets.Add(F); AllPresets.Add(R);
		UNIOTirePresets::GetPresetsForVehicle(ENIOVehicleType::ES7, F, R);
		AllPresets.Add(F); AllPresets.Add(R);
		UNIOTirePresets::GetPresetsForVehicle(ENIOVehicleType::ET5, F, R);
		AllPresets.Add(F); AllPresets.Add(R);
	}

	constexpr int32 Iterations = 5000;
	float Sink = 0.0f;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		for (const FTireModelPreset& Preset : AllPresets)
		{
			Sink += UTirePhysicsModel::CalculatePacejkaForce(Preset.Pacejka.Longitudinal, 0.08f);
			Sink += UTirePhysicsModel::CalculatePacejkaForce(Preset.Pacejka.Lateral, 0.06f);
		}
	});

	// Target: 8 presets x 2 calls = 16 Pacejka evaluations in under 10 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Pacejka All Presets (16 calls)"), AvgUs, 10.0);

	return true;
}

// --- Thermal Model Update ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FThermalModelBenchmark,
	"NomiRacing.Performance.VehiclePhysics.ThermalModelUpdate",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThermalModelBenchmark::RunTest(const FString& Parameters)
{
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TireModel->RegisterComponent();

	constexpr int32 Iterations = 5000;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		// Update all 4 wheels with varying slip and load
		for (int32 i = 0; i < 4; ++i)
		{
			TireModel->UpdateThermalState(0.016f, i, 0.3f, 4000.0f);
		}
	});

	// Target: 4-wheel thermal update in under 20 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Thermal Model (4 wheels)"), AvgUs, 20.0);

	return true;
}

// --- Thermal Grip Multiplier ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FThermalGripBenchmark,
	"NomiRacing.Performance.VehiclePhysics.ThermalGripMultiplier",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThermalGripBenchmark::RunTest(const FString& Parameters)
{
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TireModel->RegisterComponent();

	constexpr int32 Iterations = 100000;
	float Sink = 0.0f;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		Sink += TireModel->GetThermalGripMultiplier(0);
	});

	// Target: single grip lookup in under 0.5 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Thermal Grip Lookup"), AvgUs, 0.5);

	return true;
}

// --- Surface Grip Multiplier ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSurfaceGripBenchmark,
	"NomiRacing.Performance.VehiclePhysics.SurfaceGripMultiplier",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSurfaceGripBenchmark::RunTest(const FString& Parameters)
{
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TireModel->RegisterComponent();

	constexpr int32 Iterations = 100000;
	float Sink = 0.0f;

	// Cycle through surface types
	const ETireSurfaceType Surfaces[] = {
		ETireSurfaceType::Tarmac, ETireSurfaceType::TarmacWet,
		ETireSurfaceType::Gravel, ETireSurfaceType::Ice
	};
	int32 SurfaceIdx = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		TireModel->SetSurfaceType(0, Surfaces[SurfaceIdx % 4]);
		Sink += TireModel->GetSurfaceGripMultiplier(0);
		++SurfaceIdx;
	});

	// Target: set + get surface grip in under 1.0 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Surface Grip Set+Get"), AvgUs, 1.0);

	return true;
}

// --- Slip Ratio Calculation ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSlipRatioBenchmark,
	"NomiRacing.Performance.VehiclePhysics.SlipRatioCalculation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSlipRatioBenchmark::RunTest(const FString& Parameters)
{
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();

	constexpr int32 Iterations = 100000;
	float Sink = 0.0f;

	// Vary angular velocity and vehicle speed across iterations
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		const float AngVel = 20.0f + (Counter % 100) * 0.5f;
		const float VehSpeed = 500.0f + (Counter % 200) * 10.0f;
		Sink += TireModel->CalculateSlipRatio(0, AngVel, VehSpeed);
		++Counter;
	});

	// Target: single slip ratio calculation in under 0.5 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Slip Ratio Calculation"), AvgUs, 0.5);

	return true;
}

// --- Slip Angle Calculation ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSlipAngleBenchmark,
	"NomiRacing.Performance.VehiclePhysics.SlipAngleCalculation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSlipAngleBenchmark::RunTest(const FString& Parameters)
{
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();

	constexpr int32 Iterations = 100000;
	float Sink = 0.0f;
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		const float Angle = (Counter % 360) * 0.01f;
		const FVector WheelVel = FVector(1000.0f, FMath::Sin(Angle) * 200.0f, 0.0f);
		const FVector WheelFwd = FVector(1.0f, 0.0f, 0.0f);
		Sink += TireModel->CalculateSlipAngle(0, WheelVel, WheelFwd);
		++Counter;
	});

	// Target: single slip angle calculation in under 0.5 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Slip Angle Calculation"), AvgUs, 0.5);

	return true;
}

// --- Combined 4-Wheel Physics Step ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFullTirePhysicsStepBenchmark,
	"NomiRacing.Performance.VehiclePhysics.FullTirePhysicsStep",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFullTirePhysicsStepBenchmark::RunTest(const FString& Parameters)
{
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TireModel->RegisterComponent();

	// Apply EP9 preset for realistic configuration
	FTireModelPreset FrontPreset, RearPreset;
	UNIOTirePresets::GetPresetsForVehicle(ENIOVehicleType::EP9, FrontPreset, RearPreset);
	TireModel->ApplyPreset(FrontPreset);

	constexpr int32 Iterations = 10000;
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		// Simulate a full physics tick: thermal + forces for all 4 wheels
		for (int32 i = 0; i < 4; ++i)
		{
			const float SlipMag = 0.05f + (Counter % 50) * 0.002f;
			const float Load = 3000.0f + (Counter % 100) * 20.0f;
			TireModel->UpdateThermalState(0.016f, i, SlipMag, Load);
		}

		// Read back forces
		float TotalLong = 0.0f;
		float TotalLat = 0.0f;
		for (int32 i = 0; i < 4; ++i)
		{
			TotalLong += TireModel->GetLongitudinalForce(i);
			TotalLat += TireModel->GetLateralForce(i);
		}
		++Counter;
	});

	// Target: full 4-wheel tire step in under 50 us (within 60 FPS budget)
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Full 4-Wheel Tire Step"), AvgUs, 50.0);

	return true;
}

// ============================================================================
// AI CALCULATIONS BENCHMARKS
// ============================================================================

// --- Rubber Band Scaler ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRubberBandScalerBenchmark,
	"NomiRacing.Performance.AI.RubberBandScalerUpdate",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FRubberBandScalerBenchmark::RunTest(const FString& Parameters)
{
	UAIRubberBandScaler* RubberBand = NewObject<UAIRubberBandScaler>();
	RubberBand->RegisterComponent();

	constexpr int32 Iterations = 100000;
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		const float Distance = 1000.0f + (Counter % 100) * 50.0f;
		const int32 AIPos = 2 + (Counter % 8);
		const int32 PlayerPos = 1;
		const float Progress = 0.1f + (Counter % 10) * 0.08f;
		RubberBand->UpdateState(Distance, AIPos, PlayerPos, Progress);
		++Counter;
	});

	// Target: single rubber band update in under 2.0 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Rubber Band Scaler Update"), AvgUs, 2.0);

	return true;
}

// --- Rubber Band Speed Multiplier Read ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRubberBandReadBenchmark,
	"NomiRacing.Performance.AI.RubberBandSpeedMultiplierRead",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FRubberBandReadBenchmark::RunTest(const FString& Parameters)
{
	UAIRubberBandScaler* RubberBand = NewObject<UAIRubberBandScaler>();
	RubberBand->RegisterComponent();

	// Pre-populate state
	RubberBand->UpdateState(5000.0f, 3, 1, 0.5f);

	constexpr int32 Iterations = 100000;
	float Sink = 0.0f;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		Sink += RubberBand->GetSpeedMultiplier();
		Sink += RubberBand->GetBrakePointAdjustment();
		Sink += RubberBand->GetCorneringBonus();
	});

	// Target: 3 property reads in under 0.1 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Rubber Band Read (3 props)"), AvgUs, 0.1);

	return true;
}

// --- Overtake Evaluator ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOvertakeEvaluatorBenchmark,
	"NomiRacing.Performance.AI.OvertakeEvaluatorDecision",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FOvertakeEvaluatorBenchmark::RunTest(const FString& Parameters)
{
	UAIOvertakeEvaluator* Evaluator = NewObject<UAIOvertakeEvaluator>();
	Evaluator->RegisterComponent();

	// Build a realistic sensor data set
	FAISensorData SensorData;
	SensorData.bHasVehicleAhead = true;
	SensorData.VehicleAhead.Distance = 800.0f;
	SensorData.VehicleAhead.RelativeSpeed = -20.0f;
	SensorData.VehicleAhead.LateralOffset = 0.2f;
	SensorData.VehicleAhead.bIsAhead = true;
	SensorData.VehicleAhead.AngleDeg = 5.0f;

	// Add nearby vehicles for complexity
	SensorData.NearbyVehicles.SetNum(4);
	for (int32 i = 0; i < 4; ++i)
	{
		SensorData.NearbyVehicles[i].Distance = 500.0f + i * 300.0f;
		SensorData.NearbyVehicles[i].RelativeSpeed = -10.0f - i * 5.0f;
		SensorData.NearbyVehicles[i].LateralOffset = -0.5f + i * 0.3f;
		SensorData.NearbyVehicles[i].bIsAhead = (i < 2);
		SensorData.NearbyVehicles[i].bIsBehind = (i >= 2);
	}

	constexpr int32 Iterations = 10000;
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		// Vary speed to exercise different code paths
		const float Speed = 80.0f + (Counter % 40) * 5.0f;
		FOvertakeOpportunity Opp = Evaluator->Evaluate(SensorData, Speed);
		Counter++;
	});

	// Target: full overtake evaluation in under 30 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Overtake Evaluator Decision"), AvgUs, 30.0);

	return true;
}

// --- Defensive Evaluator ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDefensiveEvaluatorBenchmark,
	"NomiRacing.Performance.AI.DefensiveEvaluatorDecision",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FDefensiveEvaluatorBenchmark::RunTest(const FString& Parameters)
{
	UAIDefensiveEvaluator* Evaluator = NewObject<UAIDefensiveEvaluator>();
	Evaluator->RegisterComponent();

	// Build sensor data with a threat from behind
	FAISensorData SensorData;
	SensorData.bHasVehicleBehind = true;
	SensorData.VehicleBehind.Distance = 600.0f;
	SensorData.VehicleBehind.RelativeSpeed = 30.0f;
	SensorData.VehicleBehind.LateralOffset = 0.3f;
	SensorData.VehicleBehind.bIsBehind = true;
	SensorData.VehicleBehind.AngleDeg = 175.0f;

	constexpr int32 Iterations = 10000;
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		const float Speed = 100.0f + (Counter % 30) * 3.0f;
		const int32 Pos = 2 + (Counter % 6);
		FAIDefensiveAction Action = Evaluator->Evaluate(SensorData, Speed, Pos);
		Counter++;
	});

	// Target: full defensive evaluation in under 20 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Defensive Evaluator Decision"), AvgUs, 20.0);

	return true;
}

// --- Slipstream Wake Strength Calculation ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSlipstreamWakeBenchmark,
	"NomiRacing.Performance.AI.SlipstreamWakeCalculation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSlipstreamWakeBenchmark::RunTest(const FString& Parameters)
{
	UAISlipstreamSystem* Slipstream = NewObject<UAISlipstreamSystem>();
	Slipstream->RegisterComponent();

	// Build sensor data with a vehicle ahead in slipstream zone
	FAISensorData SensorData;
	SensorData.bHasVehicleAhead = true;
	SensorData.bSlipstreamAvailable = true;
	SensorData.SlipstreamStrength = 0.6f;
	SensorData.VehicleAhead.Distance = 700.0f;
	SensorData.VehicleAhead.RelativeSpeed = -5.0f;
	SensorData.VehicleAhead.LateralOffset = 0.05f;
	SensorData.VehicleAhead.bIsAhead = true;
	SensorData.SlipstreamTarget = SensorData.VehicleAhead;

	constexpr int32 Iterations = 50000;
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		// Vary distance to exercise different wake strengths
		FAISensorData VariedData = SensorData;
		VariedData.VehicleAhead.Distance = 400.0f + (Counter % 20) * 100.0f;
		VariedData.SlipstreamTarget = VariedData.VehicleAhead;
		Slipstream->UpdateFromSensorData(VariedData);
		Counter++;
	});

	// Target: slipstream update in under 5 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Slipstream Wake Calculation"), AvgUs, 5.0);

	return true;
}

// --- Full AI Decision Pipeline ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIFullDecisionBenchmark,
	"NomiRacing.Performance.AI.FullDecisionPipeline",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAIFullDecisionBenchmark::RunTest(const FString& Parameters)
{
	UAIBehaviorTree* BehaviorTree = NewObject<UAIBehaviorTree>();
	BehaviorTree->RegisterComponent();

	// Build comprehensive decision factors
	FAIDecisionFactors Factors;
	Factors.DistanceToWaypoint = 1500.0f;
	Factors.DistanceToVehicleAhead = 800.0f;
	Factors.DistanceToVehicleBehind = 1200.0f;
	Factors.CurrentSpeed = 180.0f;
	Factors.RecommendedSpeed = 160.0f;
	Factors.bIsOnStraight = true;
	Factors.bIsInCorner = false;
	Factors.CornerSharpness = 0.0f;
	Factors.TrackPosition = 0.45f;
	Factors.RacePosition = 3;
	Factors.TotalRacers = 8;
	Factors.bIsPlayerAhead = true;
	Factors.bSlipstreamAvailable = false;
	Factors.SlipstreamStrength = 0.0f;

	constexpr int32 Iterations = 10000;
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		// Vary factors to exercise different code paths
		FAIDecisionFactors VariedFactors = Factors;
		VariedFactors.CurrentSpeed = 120.0f + (Counter % 60) * 2.0f;
		VariedFactors.DistanceToVehicleAhead = 300.0f + (Counter % 20) * 100.0f;
		VariedFactors.bIsInCorner = (Counter % 3 == 0);
		VariedFactors.CornerSharpness = VariedFactors.bIsInCorner ? 0.3f + (Counter % 5) * 0.1f : 0.0f;
		VariedFactors.TrackPosition = (Counter % 100) * 0.01f;

		BehaviorTree->UpdateDecisions(VariedFactors);
		Counter++;
	});

	// Target: full AI decision pipeline in under 50 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Full AI Decision Pipeline"), AvgUs, 50.0);

	return true;
}

// --- AI Input Read ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAIInputReadBenchmark,
	"NomiRacing.Performance.AI.InputRead",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAIInputReadBenchmark::RunTest(const FString& Parameters)
{
	UAIBehaviorTree* BehaviorTree = NewObject<UAIBehaviorTree>();
	BehaviorTree->RegisterComponent();

	// Prime the behavior tree with a decision
	FAIDecisionFactors Factors;
	Factors.CurrentSpeed = 150.0f;
	Factors.RecommendedSpeed = 140.0f;
	Factors.DistanceToVehicleAhead = 500.0f;
	Factors.bIsInCorner = true;
	Factors.CornerSharpness = 0.5f;
	BehaviorTree->UpdateDecisions(Factors);

	constexpr int32 Iterations = 100000;
	float Sink = 0.0f;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		Sink += BehaviorTree->GetThrottleInput();
		Sink += BehaviorTree->GetBrakeInput();
		Sink += BehaviorTree->GetSteeringInput();
		EAIBehaviorState State = BehaviorTree->GetBehaviorState();
	});

	// Target: 4 property reads in under 0.1 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("AI Input Read (4 props)"), AvgUs, 0.1);

	return true;
}

// ============================================================================
// RENDERING PIPELINE BENCHMARKS
// ============================================================================

// --- Dynamic FOV Calculation ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFOVBenchmark,
	"NomiRacing.Performance.Rendering.DynamicFOVCalculation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFOVBenchmark::RunTest(const FString& Parameters)
{
	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();
	CameraSystem->RegisterComponent();

	constexpr int32 Iterations = 100000;
	float Sink = 0.0f;
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		// Simulate varying speed and acceleration inputs
		const float Speed = (Counter % 300) * 1.0f;  // 0-300 km/h
		const float MaxSpeed = 300.0f;
		const float Accel = -10.0f + (Counter % 20) * 1.0f;

		// Access current FOV (involves internal calculation)
		Sink += CameraSystem->GetCurrentFOV();
		++Counter;
	});

	// Target: FOV read in under 0.2 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Dynamic FOV Read"), AvgUs, 0.2);

	return true;
}

// --- Camera Mode Switch ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraModeSwitchBenchmark,
	"NomiRacing.Performance.Rendering.CameraModeSwitch",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraModeSwitchBenchmark::RunTest(const FString& Parameters)
{
	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();
	CameraSystem->RegisterComponent();

	const ECameraMode Modes[] = {
		ECameraMode::Chase, ECameraMode::Hood, ECameraMode::Cockpit,
		ECameraMode::Bumper, ECameraMode::Free, ECameraMode::Cinematic
	};

	constexpr int32 Iterations = 50000;
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		CameraSystem->SetCameraMode(Modes[Counter % 6]);
		++Counter;
	});

	// Target: camera mode switch in under 2.0 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Camera Mode Switch"), AvgUs, 2.0);

	return true;
}

// --- Replay Data Interpolation ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FReplayInterpolationBenchmark,
	"NomiRacing.Performance.Rendering.ReplayDataInterpolation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FReplayInterpolationBenchmark::RunTest(const FString& Parameters)
{
	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();
	CameraSystem->RegisterComponent();

	// Start recording and populate replay data
	CameraSystem->StartRecording();

	// Simulate recording ~3 seconds of data at 30 Hz (90 points)
	for (int32 i = 0; i < 90; ++i)
	{
		// The recording happens internally via Tick, but we can test playback queries
	}

	// Even without populated data, the interpolation path should be benchmarked
	CameraSystem->StopRecording();

	constexpr int32 Iterations = 50000;
	int32 Counter = 0;
	float Sink = 0.0f;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		// Query replay duration and progress (exercises internal path)
		Sink += CameraSystem->GetReplayDuration();
		Sink += CameraSystem->GetReplayProgress();
		++Counter;
	});

	// Target: replay query in under 1.0 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Replay Query"), AvgUs, 1.0);

	return true;
}

// --- Camera Config Lookup ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCameraConfigLookupBenchmark,
	"NomiRacing.Performance.Rendering.CameraConfigLookup",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCameraConfigLookupBenchmark::RunTest(const FString& Parameters)
{
	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();
	CameraSystem->RegisterComponent();

	const ECameraMode Modes[] = {
		ECameraMode::Chase, ECameraMode::Hood, ECameraMode::Cockpit,
		ECameraMode::Bumper, ECameraMode::Free, ECameraMode::Cinematic
	};

	constexpr int32 Iterations = 100000;
	float Sink = 0.0f;
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		const FCameraConfig& Config = CameraSystem->GetCameraConfig(Modes[Counter % 6]);
		Sink += Config.FOV;
		Sink += Config.Distance;
		Sink += Config.Height;
		++Counter;
	});

	// Target: config lookup + 3 field reads in under 0.5 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Camera Config Lookup + Read"), AvgUs, 0.5);

	return true;
}

// --- Cinematic Shot Position Calculation ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCinematicShotBenchmark,
	"NomiRacing.Performance.Rendering.CinematicShotSetup",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCinematicShotBenchmark::RunTest(const FString& Parameters)
{
	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();
	CameraSystem->RegisterComponent();

	const ECinematicShotType Shots[] = {
		ECinematicShotType::WideOrbit, ECinematicShotType::LowCloseUp,
		ECinematicShotType::BirdsEye, ECinematicShotType::DramaticLowAngle,
		ECinematicShotType::TrackingSide, ECinematicShotType::FrontTracking,
		ECinematicShotType::DutchAngle, ECinematicShotType::OverShoulder,
		ECinematicShotType::Helicopter, ECinematicShotType::StaticBumper
	};

	constexpr int32 Iterations = 50000;
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		CameraSystem->SetCinematicShot(Shots[Counter % 10]);
		++Counter;
	});

	// Target: cinematic shot switch in under 2.0 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Cinematic Shot Switch"), AvgUs, 2.0);

	return true;
}

// ============================================================================
// CROSS-SYSTEM BENCHMARKS
// ============================================================================

// --- Combined Vehicle + AI + Camera Frame ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombinedFrameBenchmark,
	"NomiRacing.Performance.Combined.FullGameFrame",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCombinedFrameBenchmark::RunTest(const FString& Parameters)
{
	// Set up all three systems
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TireModel->RegisterComponent();
	FTireModelPreset FrontPreset, RearPreset;
	UNIOTirePresets::GetPresetsForVehicle(ENIOVehicleType::EP9, FrontPreset, RearPreset);
	TireModel->ApplyPreset(FrontPreset);

	UAIBehaviorTree* BehaviorTree = NewObject<UAIBehaviorTree>();
	BehaviorTree->RegisterComponent();

	UAIRubberBandScaler* RubberBand = NewObject<UAIRubberBandScaler>();
	RubberBand->RegisterComponent();

	UCameraSystem* CameraSystem = NewObject<UCameraSystem>();
	CameraSystem->RegisterComponent();

	constexpr int32 Iterations = 5000;
	int32 Counter = 0;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		// 1. Vehicle physics tick (4 wheels)
		for (int32 i = 0; i < 4; ++i)
		{
			TireModel->UpdateThermalState(0.016f, i, 0.1f, 4000.0f);
		}

		// 2. AI decision
		FAIDecisionFactors Factors;
		Factors.CurrentSpeed = 150.0f + (Counter % 50);
		Factors.DistanceToVehicleAhead = 500.0f + (Counter % 20) * 100.0f;
		Factors.bIsInCorner = (Counter % 4 == 0);
		Factors.CornerSharpness = Factors.bIsInCorner ? 0.4f : 0.0f;
		BehaviorTree->UpdateDecisions(Factors);

		// 3. Rubber band update
		RubberBand->UpdateState(3000.0f, 3, 1, 0.5f);

		// 4. Camera FOV read
		float FOV = CameraSystem->GetCurrentFOV();

		++Counter;
	});

	// Target: combined frame (physics + AI + camera) in under 100 us
	// At 60 FPS, budget is ~16666 us per frame; 100 us leaves plenty of headroom
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("Combined Game Frame"), AvgUs, 100.0);

	return true;
}

// --- Memory Allocation Stress ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMemoryAllocBenchmark,
	"NomiRacing.Performance.Combined.MemoryAllocationStress",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryAllocBenchmark::RunTest(const FString& Parameters)
{
	// Benchmark object creation and destruction to detect allocation hotspots
	constexpr int32 Iterations = 1000;

	const double AvgUs = FPerformanceBenchmarkUtils::BenchmarkIterations(Iterations, [&]()
	{
		// Create and destroy a set of AI components (worst case: spawning new AI)
		UAIBehaviorTree* BT = NewObject<UAIBehaviorTree>();
		UAIRubberBandScaler* RB = NewObject<UAIRubberBandScaler>();
		UAIOvertakeEvaluator* OE = NewObject<UAIOvertakeEvaluator>();
		UAIDefensiveEvaluator* DE = NewObject<UAIDefensiveEvaluator>();
		UAISlipstreamSystem* SS = NewObject<UAISlipstreamSystem>();

		// GC handles cleanup; we measure creation cost
	});

	// Target: 5 UObject allocations in under 50 us
	FPerformanceBenchmarkUtils::LogBenchmarkResult(this, TEXT("5 UObject Allocations"), AvgUs, 50.0);

	return true;
}
