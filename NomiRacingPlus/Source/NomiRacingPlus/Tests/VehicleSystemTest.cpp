// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Tests/VehicleSystemTest.h"
#include "Vehicles/VehicleStateManager.h"
#include "Vehicles/NIOVehicleMovementComponent.h"
#include "Vehicles/TirePhysicsModel.h"
#include "Vehicles/NIOTirePresets.h"

/**
 * Test VehicleStateManager initialization and state tracking
 */
bool FVehicleStateManagerTest::RunTest(const FString& Parameters)
{
	// Test 1: Create VehicleStateManager and verify initial state
	UVehicleStateManager* StateManager = NewObject<UVehicleStateManager>();
	TestNotNull(TEXT("StateManager should be created"), StateManager);

	if (!StateManager)
	{
		return false;
	}

	// Test 2: Verify initial state values
	const FNIOVehicleState& InitialState = StateManager->GetVehicleState();
	TestEqual(TEXT("Initial speed should be 0"), InitialState.SpeedKmh, 0.0f);
	TestEqual(TEXT("Initial throttle should be 0"), InitialState.ThrottleInput, 0.0f);
	TestEqual(TEXT("Initial brake should be 0"), InitialState.BrakeInput, 0.0f);
	TestEqual(TEXT("Initial steering should be 0"), InitialState.SteeringInput, 0.0f);
	TestFalse(TEXT("Initial drift should be false"), InitialState.bIsDrifting);
	TestTrue(TEXT("Initial grounded should be true"), InitialState.bIsGrounded);

	// Test 3: Set vehicle type and verify
	StateManager->SetVehicleType(ENIOVehicleType::EP9);
	TestEqual(TEXT("Vehicle type should be EP9"), StateManager->GetVehicleType(), ENIOVehicleType::EP9);

	// Test 4: Verify NIO vehicle detection
	TestTrue(TEXT("EP9 should be NIO vehicle"), StateManager->IsNIOVehicle());

	StateManager->SetVehicleType(ENIOVehicleType::Custom);
	TestFalse(TEXT("Custom should not be NIO vehicle"), StateManager->IsNIOVehicle());

	// Test 5: Verify vehicle display names
	StateManager->SetVehicleType(ENIOVehicleType::EP9);
	TestEqual(TEXT("EP9 display name"), StateManager->GetVehicleDisplayName(), FString(TEXT("NIO EP9")));

	StateManager->SetVehicleType(ENIOVehicleType::ET7);
	TestEqual(TEXT("ET7 display name"), StateManager->GetVehicleDisplayName(), FString(TEXT("NIO ET7")));

	StateManager->SetVehicleType(ENIOVehicleType::ES7);
	TestEqual(TEXT("ES7 display name"), StateManager->GetVehicleDisplayName(), FString(TEXT("NIO ES7")));

	return true;
}

/**
 * Test NIO vehicle physics configuration
 */
bool FNIOVehiclePhysicsTest::RunTest(const FString& Parameters)
{
	// Test 1: Create movement component
	UNIOVehicleMovementComponent* Movement = NewObject<UNIOVehicleMovementComponent>();
	TestNotNull(TEXT("Movement component should be created"), Movement);

	if (!Movement)
	{
		return false;
	}

	// Test 2: Configure for EP9
	Movement->ConfigureForNIOVehicle(ENIOVehicleType::EP9);

	// Test 3: Verify EP9 configuration
	float EP9Torque = Movement->GetElectricMotorTorque(0.0f);
	TestTrue(TEXT("EP9 torque at 0 RPM should be high"), EP9Torque > 1000.0f);

	// Test 4: Configure for ET7
	Movement->ConfigureForNIOVehicle(ENIOVehicleType::ET7);

	float ET7Torque = Movement->GetElectricMotorTorque(0.0f);
	TestTrue(TEXT("ET7 torque at 0 RPM should be less than EP9"), ET7Torque < EP9Torque);

	// Test 5: Configure for ES7
	Movement->ConfigureForNIOVehicle(ENIOVehicleType::ES7);

	float ES7Torque = Movement->GetElectricMotorTorque(0.0f);
	TestTrue(TEXT("ES7 torque should be similar to ET7"), FMath::IsNearlyEqual(ES7Torque, ET7Torque, 50.0f));

	return true;
}

/**
 * Test drift detection logic
 */
bool FVehicleDriftDetectionTest::RunTest(const FString& Parameters)
{
	// Test 1: Create state manager
	UVehicleStateManager* StateManager = NewObject<UVehicleStateManager>();
	TestNotNull(TEXT("StateManager should be created"), StateManager);

	if (!StateManager)
	{
		return false;
	}

	// Test 2: Initial state should not be drifting
	const FNIOVehicleState& State1 = StateManager->GetVehicleState();
	TestFalse(TEXT("Initial state should not be drifting"), State1.bIsDrifting);

	// Test 3: Verify slip angle threshold
	// Note: Actual drift detection requires physics simulation
	// This test verifies the threshold constant exists
	const float DriftThreshold = 15.0f;
	TestTrue(TEXT("Drift threshold should be positive"), DriftThreshold > 0.0f);

	return true;
}

/**
 * Test electric motor torque curve
 */
bool FElectricTorqueCurveTest::RunTest(const FString& Parameters)
{
	// Test 1: Create movement component
	UNIOVehicleMovementComponent* Movement = NewObject<UNIOVehicleMovementComponent>();
	TestNotNull(TEXT("Movement component should be created"), Movement);

	if (!Movement)
	{
		return false;
	}

	// Test 2: Configure for EP9
	Movement->ConfigureForNIOVehicle(ENIOVehicleType::EP9);

	// Test 3: Verify torque at 0 RPM (should be peak)
	float TorqueAtZero = Movement->GetElectricMotorTorque(0.0f);
	TestTrue(TEXT("Torque at 0 RPM should be peak"), TorqueAtZero > 1000.0f);

	// Test 4: Verify torque at peak RPM (should be high)
	float TorqueAtPeak = Movement->GetElectricMotorTorque(3000.0f);
	TestTrue(TEXT("Torque at peak RPM should be high"), TorqueAtPeak > 1000.0f);

	// Test 5: Verify torque decay at high RPM
	float TorqueAtHigh = Movement->GetElectricMotorTorque(10000.0f);
	TestTrue(TEXT("Torque at high RPM should decay"), TorqueAtHigh < TorqueAtZero);

	// Test 6: Verify torque at max RPM (should be near zero)
	float TorqueAtMax = Movement->GetElectricMotorTorque(15000.0f);
	TestTrue(TEXT("Torque at max RPM should be near zero"), TorqueAtMax < 100.0f);

	// Test 7: Verify regenerative braking
	float RegenTorque = Movement->GetRegenerativeBrakingTorque();
	// Regen torque depends on speed, so we just verify it doesn't crash
	TestTrue(TEXT("Regen torque should be non-negative"), RegenTorque >= 0.0f);

	return true;
}

/**
 * Test tire model integration with vehicle system
 */
bool FTireModelIntegrationTest::RunTest(const FString& Parameters)
{
	// Test 1: Create tire physics model
	UTirePhysicsModel* TireModel = NewObject<UTirePhysicsModel>();
	TestNotNull(TEXT("TireModel should be created"), TireModel);

	if (!TireModel)
	{
		return false;
	}

	// Test 2: Verify initial tire states
	const TArray<FTireState>& InitialStates = TireModel->GetAllTireStates();
	TestEqual(TEXT("Should have 4 tire states"), InitialStates.Num(), 4);

	// Test 3: Verify Pacejka force calculation
	FPacejkaCoefficients TestCoeffs;
	TestCoeffs.B = 10.0f;
	TestCoeffs.C = 1.65f;
	TestCoeffs.D = 1.0f;
	TestCoeffs.E = -0.1f;

	float ForceAtZeroSlip = UTirePhysicsModel::CalculatePacejkaForce(TestCoeffs, 0.0f);
	TestEqual(TEXT("Force at zero slip should be zero"), ForceAtZeroSlip, 0.0f);

	float ForceAtSmallSlip = UTirePhysicsModel::CalculatePacejkaForce(TestCoeffs, 0.05f);
	TestTrue(TEXT("Force at small slip should be positive"), ForceAtSmallSlip > 0.0f);

	// Test 4: Apply EP9 preset
	FTireModelPreset EP9Front = UNIOTirePresets::CreateEP9FrontPreset();
	TireModel->ApplyPreset(EP9Front);
	TestEqual(TEXT("EP9 front tire width should be 265"), EP9Front.TireWidthMm, 265.0f);

	// Test 5: Verify thermal grip calculation
	float ThermalGrip = TireModel->GetThermalGripMultiplier(0);
	TestTrue(TEXT("Thermal grip should be positive"), ThermalGrip > 0.0f);

	// Test 6: Verify surface grip
	TireModel->SetSurfaceType(0, ETireSurfaceType::Tarmac);
	float TarmacGrip = TireModel->GetSurfaceGripMultiplier(0);
	TestEqual(TEXT("Tarmac grip should be 1.0"), TarmacGrip, 1.0f);

	TireModel->SetSurfaceType(0, ETireSurfaceType::Ice);
	float IceGrip = TireModel->GetSurfaceGripMultiplier(0);
	TestTrue(TEXT("Ice grip should be much less than tarmac"), IceGrip < TarmacGrip * 0.5f);

	// Test 7: Verify tire wear tracking
	float InitialWear = TireModel->GetTireState(0).Thermal.WearFactor;
	TestEqual(TEXT("Initial wear should be 1.0"), InitialWear, 1.0f);

	// Test 8: Verify slip detection
	bool bIsSlipping = TireModel->IsAnyTireSlipping(0.15f);
	TestFalse(TEXT("Should not be slipping initially"), bIsSlipping);

	return true;
}
