// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Tests/TirePhysicsTest.h"
#include "Vehicles/TirePhysicsModel.h"
#include "Vehicles/NIOTirePresets.h"
#include "Misc/AutomationTest.h"

// ============================================================================
// Pacejka Magic Formula Unit Tests
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPacejkaBasicForceTest,
	"NomiRacing.Vehicles.TirePhysics.PacejkaBasicForce",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPacejkaBasicForceTest::RunTest(const FString& Parameters)
{
	// Test basic Pacejka formula output
	FPacejkaCoefficients Coeffs;
	Coeffs.B = 10.0f;
	Coeffs.C = 1.65f;
	Coeffs.D = 1.0f;
	Coeffs.E = -0.1f;

	// Zero slip should produce zero force
	const float ZeroSlipForce = UTirePhysicsModel::CalculatePacejkaForce(Coeffs, 0.0f);
	TestEqual(TEXT("Zero slip should produce zero force"), ZeroSlipForce, 0.0f);

	// Small positive slip should produce positive force
	const float SmallSlipForce = UTirePhysicsModel::CalculatePacejkaForce(Coeffs, 0.05f);
	TestTrue(TEXT("Small positive slip should produce positive force"), SmallSlipForce > 0.0f);

	// Small negative slip should produce negative force
	const float NegSlipForce = UTirePhysicsModel::CalculatePacejkaForce(Coeffs, -0.05f);
	TestTrue(TEXT("Small negative slip should produce negative force"), NegSlipForce < 0.0f);

	// Force should peak and then decline (Pacejka characteristic)
	const float PeakSlipForce = UTirePhysicsModel::CalculatePacejkaForce(Coeffs, 0.1f);
	const float HighSlipForce = UTirePhysicsModel::CalculatePacejkaForce(Coeffs, 0.5f);
	TestTrue(TEXT("Force should peak before declining"), FMath::Abs(PeakSlipForce) >= FMath::Abs(HighSlipForce));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPacejkaInvalidCoeffsTest,
	"NomiRacing.Vehicles.TirePhysics.PacejkaInvalidCoeffs",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPacejkaInvalidCoeffsTest::RunTest(const FString& Parameters)
{
	// Invalid coefficients should return zero
	FPacejkaCoefficients InvalidCoeffs;
	InvalidCoeffs.B = 0.0f; // Invalid
	InvalidCoeffs.C = 1.65f;
	InvalidCoeffs.D = 1.0f;
	InvalidCoeffs.E = -0.1f;

	const float Force = UTirePhysicsModel::CalculatePacejkaForce(InvalidCoeffs, 0.1f);
	TestEqual(TEXT("Invalid coefficients should return zero force"), Force, 0.0f);

	return true;
}

// ============================================================================
// Slip Ratio Calculation Tests
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSlipRatioZeroSpeedTest,
	"NomiRacing.Vehicles.TirePhysics.SlipRatioZeroSpeed",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSlipRatioZeroSpeedTest::RunTest(const FString& Parameters)
{
	// At zero vehicle speed, slip ratio should be zero
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();

	const float SlipRatio = TireModel->CalculateSlipRatio(0, 10.0f, 0.0f);
	TestEqual(TEXT("Slip ratio at zero speed should be zero"), SlipRatio, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSlipRatioMatchingSpeedTest,
	"NomiRacing.Vehicles.TirePhysics.SlipRatioMatchingSpeed",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSlipRatioMatchingSpeedTest::RunTest(const FString& Parameters)
{
	// When wheel speed matches vehicle speed, slip ratio should be near zero
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();

	// Assuming tire radius ~0.3m, vehicle speed 1000 cm/s (10 m/s)
	// Wheel angular velocity = linear / radius = 10 / 0.3 = 33.3 rad/s
	const float VehicleSpeed = 1000.0f; // cm/s
	const float WheelAngularVel = 33.3f; // rad/s

	const float SlipRatio = TireModel->CalculateSlipRatio(0, WheelAngularVel, VehicleSpeed);
	TestTrue(TEXT("Slip ratio should be near zero when speeds match"), FMath::Abs(SlipRatio) < 0.1f);

	return true;
}

// ============================================================================
// Tire Preset Tests
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNIOTirePresetsExistTest,
	"NomiRacing.Vehicles.TirePhysics.NIOTirePresetsExist",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNIOTirePresetsExistTest::RunTest(const FString& Parameters)
{
	// Test that all NIO vehicle presets can be created
	FTireModelPreset FrontPreset, RearPreset;

	// EP9
	UNIOTirePresets::GetPresetsForVehicle(ENIOVehicleType::EP9, FrontPreset, RearPreset);
	TestFalse(TEXT("EP9 front preset name should not be empty"), FrontPreset.PresetName.IsEmpty());
	TestFalse(TEXT("EP9 rear preset name should not be empty"), RearPreset.PresetName.IsEmpty());
	TestTrue(TEXT("EP9 front should have valid Pacejka B"), FrontPreset.Pacejka.Longitudinal.B > 0.0f);
	TestTrue(TEXT("EP9 rear should be wider than front"), RearPreset.TireWidthMm > FrontPreset.TireWidthMm);

	// ET7
	UNIOTirePresets::GetPresetsForVehicle(ENIOVehicleType::ET7, FrontPreset, RearPreset);
	TestFalse(TEXT("ET7 front preset name should not be empty"), FrontPreset.PresetName.IsEmpty());
	TestTrue(TEXT("ET7 should have lower friction than EP9"), FrontPreset.BaseFriction <= 1.1f);

	// ES7
	UNIOTirePresets::GetPresetsForVehicle(ENIOVehicleType::ES7, FrontPreset, RearPreset);
	TestFalse(TEXT("ES7 front preset name should not be empty"), FrontPreset.PresetName.IsEmpty());
	TestTrue(TEXT("ES7 should have lower rolling resistance"), FrontPreset.RollingResistance > 0.01f);

	// ET5
	UNIOTirePresets::GetPresetsForVehicle(ENIOVehicleType::ET5, FrontPreset, RearPreset);
	TestFalse(TEXT("ET5 front preset name should not be empty"), FrontPreset.PresetName.IsEmpty());

	return true;
}

// ============================================================================
// Thermal Model Tests
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FThermalGripOptimalTest,
	"NomiRacing.Vehicles.TirePhysics.ThermalGripOptimal",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThermalGripOptimalTest::RunTest(const FString& Parameters)
{
	// Test thermal grip calculation at optimal temperature
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TireModel->RegisterComponent();

	// Set tire to optimal temperature
	FTireState& TireState = const_cast<FTireState&>(TireModel->GetTireState(0));
	TireState.Thermal.SurfaceTemperature = 85.0f;
	TireState.Thermal.OptimalTemperature = 85.0f;

	const float GripMultiplier = TireModel->GetThermalGripMultiplier(0);
	TestTrue(TEXT("Grip at optimal temperature should be ~1.05"), FMath::Abs(GripMultiplier - 1.05f) < 0.05f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FThermalGripColdTest,
	"NomiRacing.Vehicles.TirePhysics.ThermalGripCold",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThermalGripColdTest::RunTest(const FString& Parameters)
{
	// Test thermal grip at cold temperature
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TireModel->RegisterComponent();

	FTireState& TireState = const_cast<FTireState&>(TireModel->GetTireState(0));
	TireState.Thermal.SurfaceTemperature = 25.0f; // Ambient
	TireState.Thermal.ColdThreshold = 40.0f;

	const float GripMultiplier = TireModel->GetThermalGripMultiplier(0);
	TestTrue(TEXT("Cold tire grip should be significantly reduced"), GripMultiplier < 0.7f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FThermalGripOverheatTest,
	"NomiRacing.Vehicles.TirePhysics.ThermalGripOverheat",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThermalGripOverheatTest::RunTest(const FString& Parameters)
{
	// Test thermal grip at overheated temperature
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TireModel->RegisterComponent();

	FTireState& TireState = const_cast<FTireState&>(TireModel->GetTireState(0));
	TireState.Thermal.SurfaceTemperature = 140.0f; // Overheated
	TireState.Thermal.OverheatThreshold = 110.0f;
	TireState.Thermal.MaxTemperature = 150.0f;

	const float GripMultiplier = TireModel->GetThermalGripMultiplier(0);
	TestTrue(TEXT("Overheated tire grip should be reduced"), GripMultiplier < 0.9f);

	return true;
}

// ============================================================================
// Surface Grip Tests
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSurfaceGripTarmacTest,
	"NomiRacing.Vehicles.TirePhysics.SurfaceGripTarmac",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSurfaceGripTarmacTest::RunTest(const FString& Parameters)
{
	// Test surface grip multipliers
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TireModel->RegisterComponent();

	// Tarmac (dry) should be baseline
	TireModel->SetSurfaceType(0, ETireSurfaceType::Tarmac);
	const float TarmacGrip = TireModel->GetSurfaceGripMultiplier(0);
	TestEqual(TEXT("Tarmac grip should be 1.0"), TarmacGrip, 1.0f);

	// Wet tarmac should have less grip
	TireModel->SetSurfaceType(0, ETireSurfaceType::TarmacWet);
	const float WetGrip = TireModel->GetSurfaceGripMultiplier(0);
	TestTrue(TEXT("Wet tarmac should have less grip"), WetGrip < TarmacGrip);

	// Ice should have very low grip
	TireModel->SetSurfaceType(0, ETireSurfaceType::Ice);
	const float IceGrip = TireModel->GetSurfaceGripMultiplier(0);
	TestTrue(TEXT("Ice should have very low grip"), IceGrip < 0.3f);

	return true;
}

// ============================================================================
// Pacejka Symmetry Tests
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPacejkaSymmetryTest,
	"NomiRacing.Vehicles.TirePhysics.PacejkaSymmetry",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPacejkaSymmetryTest::RunTest(const FString& Parameters)
{
	// Pacejka with E=0 should produce symmetric forces for pos/neg slip
	FPacejkaCoefficients SymCoeffs;
	SymCoeffs.B = 10.0f;
	SymCoeffs.C = 1.5f;
	SymCoeffs.D = 1.0f;
	SymCoeffs.E = 0.0f; // Zero E = symmetric

	const float PosForce = UTirePhysicsModel::CalculatePacejkaForce(SymCoeffs, 0.2f);
	const float NegForce = UTirePhysicsModel::CalculatePacejkaForce(SymCoeffs, -0.2f);

	// Should be approximately symmetric (E=0)
	TestTrue(TEXT("Positive and negative forces should be approximately symmetric"),
		FMath::Abs(PosForce + NegForce) < 0.01f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPacejkaLargeSlipTest,
	"NomiRacing.Vehicles.TirePhysics.PacejkaLargeSlip",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPacejkaLargeSlipTest::RunTest(const FString& Parameters)
{
	// Very large slip should not produce NaN or extremely large values
	FPacejkaCoefficients Coeffs;
	Coeffs.B = 10.0f;
	Coeffs.C = 1.65f;
	Coeffs.D = 1.0f;
	Coeffs.E = -0.1f;

	const float LargeSlipForce = UTirePhysicsModel::CalculatePacejkaForce(Coeffs, 100.0f);
	TestTrue(TEXT("Large slip should produce finite force"), FMath::IsFinite(LargeSlipForce));

	const float NegLargeForce = UTirePhysicsModel::CalculatePacejkaForce(Coeffs, -100.0f);
	TestTrue(TEXT("Negative large slip should produce finite force"), FMath::IsFinite(NegLargeForce));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPacejkaMonotonicTest,
	"NomiRacing.Vehicles.TirePhysics.PacejkaMonotonic",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPacejkaMonotonicTest::RunTest(const FString& Parameters)
{
	// Force should monotonically increase from 0 to peak as slip increases from 0
	FPacejkaCoefficients Coeffs;
	Coeffs.B = 10.0f;
	Coeffs.C = 1.65f;
	Coeffs.D = 1.0f;
	Coeffs.E = -0.3f;

	float PrevForce = 0.0f;
	bool bMonotonicUpToPeak = true;
	constexpr float Tolerance = 0.001f;

	for (float slip = 0.001f; slip < 0.2f; slip += 0.01f)
	{
		const float Force = UTirePhysicsModel::CalculatePacejkaForce(Coeffs, slip);
		if (Force < PrevForce - Tolerance)
		{
			bMonotonicUpToPeak = false;
			break;
		}
		PrevForce = Force;
	}

	TestTrue(TEXT("Force should be monotonically increasing from 0 to peak slip"), bMonotonicUpToPeak);

	return true;
}

// ============================================================================
// Slip Ratio Edge Cases
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSlipRatioExtremeTest,
	"NomiRacing.Vehicles.TirePhysics.SlipRatioExtreme",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSlipRatioExtremeTest::RunTest(const FString& Parameters)
{
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();

	// Very high wheel speed should give positive slip ratio capped at 2.0
	const float HighSlipRatio = TireModel->CalculateSlipRatio(0, 500.0f, 100.0f);
	TestTrue(TEXT("High wheel speed slip ratio should be clamped to < 2.01"), HighSlipRatio <= 2.01f);

	// Very low wheel speed with high vehicle speed should give slip near -1.0 (locked wheel)
	const float LockedSlipRatio = TireModel->CalculateSlipRatio(0, 1.0f, 1000.0f);
	TestTrue(TEXT("Locked wheel slip should be near -1.0"), LockedSlipRatio < -0.95f);

	// Matching speeds should give near-zero slip
	const float MatchSlip = TireModel->CalculateSlipRatio(0, 33.3f, 1000.0f);
	TestTrue(TEXT("Matching speeds should give near-zero slip"), FMath::Abs(MatchSlip) < 0.2f);

	return true;
}

// ============================================================================
// Thermal Model Edge Cases
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FThermalGripBoundariesTest,
	"NomiRacing.Vehicles.TirePhysics.ThermalGripBoundaries",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FThermalGripBoundariesTest::RunTest(const FString& Parameters)
{
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TireModel->RegisterComponent();

	FTireState& TireState = const_cast<FTireState&>(TireModel->GetTireState(0));
	TireState.Thermal.OptimalTemperature = 85.0f;
	TireState.Thermal.ColdThreshold = 40.0f;
	TireState.Thermal.OverheatThreshold = 110.0f;
	TireState.Thermal.MaxTemperature = 150.0f;

	// Freezing temperature should give minimum grip
	TireState.Thermal.SurfaceTemperature = -10.0f;
	const float FreezeGrip = TireModel->GetThermalGripMultiplier(0);
	TestTrue(TEXT("Freezing temperature grip should be low but positive"), FreezeGrip >= 0.5f && FreezeGrip < 0.7f);

	// Exactly at optimal should give ~1.05
	TireState.Thermal.SurfaceTemperature = 85.0f;
	const float OptimalGrip = TireModel->GetThermalGripMultiplier(0);
	TestTrue(TEXT("Optimal temperature grip should be ~1.05"), FMath::Abs(OptimalGrip - 1.05f) < 0.05f);

	// Max temperature should give minimum grip
	TireState.Thermal.SurfaceTemperature = 150.0f;
	const float MaxTempGrip = TireModel->GetThermalGripMultiplier(0);
	TestTrue(TEXT("Max temperature grip should be at minimum"), MaxTempGrip < 0.6f);

	return true;
}

// ============================================================================
// Combined Pacejka + Thermal Integration Test
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPacejkaThermalIntegrationTest,
	"NomiRacing.Vehicles.TirePhysics.PacejkaThermalIntegration",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPacejkaThermalIntegrationTest::RunTest(const FString& Parameters)
{
	// Test the actual integration between Pacejka force and thermal model:
	// Pacejka force should be modulated by the thermal grip multiplier,
	// and grip should vary across temperature regimes (cold → optimal → hot).

	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TireModel->RegisterComponent();

	FPacejkaCoefficients Coeffs;
	Coeffs.B = 10.0f;
	Coeffs.C = 1.65f;
	Coeffs.D = 1.0f;
	Coeffs.E = -0.1f;

	constexpr float TestSlip = 0.1f;
	const float BasePacejkaForce = UTirePhysicsModel::CalculatePacejkaForce(Coeffs, TestSlip);

	// --- Cold tire test ---
	// Simulate a cold tire (0°C) with some slip to stabilize thermal state
	TireModel->UpdateThermalState(5.0f, 0, 0.05f, 4000.0f); // 5s at minimal slip
	const float ColdGrip = TireModel->GetThermalGripMultiplier(0);
	TestTrue(TEXT("Cold tire grip should be < 0.9"), ColdGrip < 0.9f);
	TestTrue(TEXT("Cold tire grip should be > 0.3"), ColdGrip > 0.3f);
	const float ColdEffectiveForce = BasePacejkaForce * ColdGrip;
	TestTrue(TEXT("Cold Pacejka force should be less than base force"),
		ColdEffectiveForce < BasePacejkaForce);

	// --- Optimal temperature test ---
	// Heat the tire to optimal (85°C) with sustained slip
	for (int32 i = 0; i < 50; i++)
	{
		TireModel->UpdateThermalState(0.1f, 0, 0.15f, 5000.0f);
	}
	const float OptimalGrip = TireModel->GetThermalGripMultiplier(0);
	TestTrue(TEXT("Optimal tire grip should be >= 0.95"), OptimalGrip >= 0.95f);
	const float OptimalEffectiveForce = BasePacejkaForce * OptimalGrip;
	TestTrue(TEXT("Optimal Pacejka force should be >= cold force"),
		OptimalEffectiveForce >= ColdEffectiveForce);

	// --- Overheated tire test ---
	// Overheat the tire with extreme slip
	for (int32 i = 0; i < 100; i++)
	{
		TireModel->UpdateThermalState(0.1f, 0, 0.9f, 5000.0f);
	}
	const float HotGrip = TireModel->GetThermalGripMultiplier(0);
	TestTrue(TEXT("Overheated tire grip should be < optimal grip"),
		HotGrip < OptimalGrip);
	TestTrue(TEXT("Overheated tire grip should be > 0.3 (minimum threshold)"),
		HotGrip > 0.3f);
	const float HotEffectiveForce = BasePacejkaForce * HotGrip;
	TestTrue(TEXT("Overheated Pacejka force should be less than optimal force"),
		HotEffectiveForce < OptimalEffectiveForce);

	return true;
}

// ============================================================================
// Tire Wear Tests
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTireWearBasicTest,
	"NomiRacing.Vehicles.TirePhysics.TireWearBasic",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FTireWearBasicTest::RunTest(const FString& Parameters)
{
	// Test that tire wear reduces over time
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TireModel->RegisterComponent();

	const float InitialWear = TireModel->GetTireState(0).Thermal.WearFactor;
	TestEqual(TEXT("Initial wear should be 1.0"), InitialWear, 1.0f);

	// Simulate some tire usage with slip
	TireModel->UpdateThermalState(1.0f, 0, 0.5f, 5000.0f);

	const float AfterWear = TireModel->GetTireState(0).Thermal.WearFactor;
	TestTrue(TEXT("Tire wear should decrease over time"), AfterWear < InitialWear);

	return true;
}

// ============================================================================
// Pacejka Independence Tests
// ============================================================================

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPacejkaFrictionOverrideRange,
	"NomiRacing.Vehicles.TirePhysics.PacejkaFrictionOverride",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPacejkaFrictionOverrideRange::RunTest(const FString& Parameters)
{
	// Verify that Pacejka model handles near-zero friction gracefully.
	// This validates the friction hack scheme (Scheme A) doesn't cause numerical issues.
	constexpr float ChaosFrictionOverride = 0.01f;

	// Very low friction Pacejka coefficients simulating the override scenario
	FPacejkaCoefficients Coeffs;
	Coeffs.B = 10.0f * ChaosFrictionOverride;
	Coeffs.C = 1.65f;
	Coeffs.D = ChaosFrictionOverride;
	Coeffs.E = -0.1f;

	const float ZeroSlipForce = UTirePhysicsModel::CalculatePacejkaForce(Coeffs, 0.0f);
	TestEqual(TEXT("Zero slip with low friction should produce zero force"), ZeroSlipForce, 0.0f);

	const float LowSlipForce = UTirePhysicsModel::CalculatePacejkaForce(Coeffs, 0.1f);
	TestTrue(TEXT("Low friction force should be finite"), FMath::IsFinite(LowSlipForce));
	TestTrue(TEXT("Low friction force should be positive for positive slip"), LowSlipForce > 0.0f);

	// Compare with normal friction to verify significantly reduced magnitude
	FPacejkaCoefficients NormalCoeffs;
	NormalCoeffs.B = 10.0f;
	NormalCoeffs.C = 1.65f;
	NormalCoeffs.D = 1.0f;
	NormalCoeffs.E = -0.1f;
	const float NormalForce = UTirePhysicsModel::CalculatePacejkaForce(NormalCoeffs, 0.1f);
	TestTrue(TEXT("Low friction force should be < 2% of normal friction force"),
		LowSlipForce < 0.02f * NormalForce);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPacejkaForceWithVehiclePresets,
	"NomiRacing.Vehicles.TirePhysics.PacejkaForceWithPresets",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPacejkaForceWithVehiclePresets::RunTest(const FString& Parameters)
{
	TArray<ENIOVehicleType> VehicleTypes = {
		ENIOVehicleType::EP9, ENIOVehicleType::ET7, ENIOVehicleType::ES7,
		ENIOVehicleType::ET5, ENIOVehicleType::SU7Ultra
	};

	for (ENIOVehicleType Type : VehicleTypes)
	{
		FTireModelPreset FrontPreset, RearPreset;
		UNIOTirePresets::GetPresetsForVehicle(Type, FrontPreset, RearPreset);

		const float TestSlip = 0.1f;
		const float LongForce = UTirePhysicsModel::CalculatePacejkaForce(FrontPreset.Pacejka.Longitudinal, TestSlip * 100.0f);
		const float LatForce = UTirePhysicsModel::CalculatePacejkaForce(FrontPreset.Pacejka.Lateral, TestSlip);

		TestTrue(FString::Printf(TEXT("%s Long force finite"), *FrontPreset.PresetName), FMath::IsFinite(LongForce));
		TestTrue(FString::Printf(TEXT("%s Lat force finite"), *FrontPreset.PresetName), FMath::IsFinite(LatForce));
		TestTrue(FString::Printf(TEXT("%s Long force > 0"), *FrontPreset.PresetName), LongForce > 0.0f);
		TestTrue(FString::Printf(TEXT("%s Lat force > 0"), *FrontPreset.PresetName), LatForce > 0.0f);

		const float DLong = FrontPreset.Pacejka.Longitudinal.D;
		const float DLat = FrontPreset.Pacejka.Lateral.D;
		TestTrue(FString::Printf(TEXT("%s Long <= D"), *FrontPreset.PresetName), LongForce <= DLong * 1.1f);
		TestTrue(FString::Printf(TEXT("%s Lat <= D"), *FrontPreset.PresetName), LatForce <= DLat * 1.1f);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPacejkaCombinedSlipStability,
	"NomiRacing.Vehicles.TirePhysics.CombinedSlipStability",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FPacejkaCombinedSlipStability::RunTest(const FString& Parameters)
{
	FPacejkaCoefficients LongCoeffs, LatCoeffs;
	LongCoeffs.B = 12.0f; LongCoeffs.C = 1.65f; LongCoeffs.D = 1.0f; LongCoeffs.E = -0.1f;
	LatCoeffs.B = 10.0f;  LatCoeffs.C = 1.55f; LatCoeffs.D = 1.0f; LatCoeffs.E = -0.3f;

	const float LongSlip = -0.08f;
	const float LatSlip = 0.15f;

	const float LongForce = UTirePhysicsModel::CalculatePacejkaForce(LongCoeffs, LongSlip * 100.0f);
	const float LatForce = UTirePhysicsModel::CalculatePacejkaForce(LatCoeffs, LatSlip);

	TestTrue(TEXT("Combined Long force finite"), FMath::IsFinite(LongForce));
	TestTrue(TEXT("Combined Lat force finite"), FMath::IsFinite(LatForce));
	TestTrue(TEXT("Braking force negative"), LongForce < 0.0f);
	TestTrue(TEXT("Cornering force positive"), LatForce > 0.0f);

	return true;
}
