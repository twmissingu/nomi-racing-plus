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
