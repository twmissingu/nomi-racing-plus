// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Vehicles/VehicleStateManager.h"
#include "Vehicles/NIOVehicleMovementComponent.h"
#include "Vehicles/TirePhysicsModel.h"

/**
 * Vehicle System Tests
 * Tests for vehicle state management and NIO vehicle physics
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVehicleStateManagerTest,
	"NomiRacingPlus.Vehicle.StateManager",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNIOVehiclePhysicsTest,
	"NomiRacingPlus.Vehicle.NIOPhysics",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVehicleDriftDetectionTest,
	"NomiRacingPlus.Vehicle.DriftDetection",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FElectricTorqueCurveTest,
	"NomiRacingPlus.Vehicle.ElectricTorque",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTireModelIntegrationTest,
	"NomiRacingPlus.Vehicle.TireModelIntegration",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
)
