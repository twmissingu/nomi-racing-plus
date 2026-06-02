// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

/**
 * Tire physics test utilities
 */
class FTirePhysicsTestUtils
{
public:
	// Create a test tire model with default settings
	static class UTirePhysicsModel* CreateTestTireModel();

	// Validate Pacejka coefficients are within expected ranges
	static bool ValidatePacejkaCoefficients(const struct FPacejkaCoefficients& Coeffs);
};
