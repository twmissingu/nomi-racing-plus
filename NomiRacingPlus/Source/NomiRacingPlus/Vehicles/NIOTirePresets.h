// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Vehicles/TirePhysicsModel.h"
#include "NIOTirePresets.generated.h"

/**
 * Factory class for creating NIO vehicle tire presets
 * Provides pre-tuned Pacejka coefficients based on real tire data
 */
UCLASS(BlueprintType)
class NOMIRACINGPLUS_API UNIOTirePresets : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========================================================================
	// EP9 Hypercar Presets
	// ========================================================================

	/**
	 * NIO EP9 Front Tire - Michelin Pilot Sport Cup 2
	 * 265/35R20 - High performance, semi-slick
	 */
	static FTireModelPreset CreateEP9FrontPreset()
	{
		FTireModelPreset Preset;
		Preset.PresetName = TEXT("EP9 Front");

		// Pacejka coefficients tuned for semi-slick performance tire
		// Higher B values = more responsive turn-in
		Preset.Pacejka.Longitudinal.B = 14.0f;
		Preset.Pacejka.Longitudinal.C = 1.65f;
		Preset.Pacejka.Longitudinal.D = 1.0f;
		Preset.Pacejka.Longitudinal.E = -0.15f;

		Preset.Pacejka.Lateral.B = 12.0f;
		Preset.Pacejka.Lateral.C = 1.55f;
		Preset.Pacejka.Lateral.D = 1.0f;
		Preset.Pacejka.Lateral.E = -0.35f;

		Preset.Pacejka.CombinedSlipFactor = 0.88f;

		// Dimensions
		Preset.TireWidthMm = 265;
		Preset.AspectRatio = 35;
		Preset.RimDiameterInches = 20;

		// Properties
		Preset.PressurePSI = 33.0f;
		Preset.BaseFriction = 1.15f;
		Preset.RollingResistance = 0.012f;
		Preset.TireMassKg = 11.0f;

		// Thermal
		Preset.OptimalTemp = 90.0f;
		Preset.ColdThreshold = 45.0f;
		Preset.OverheatThreshold = 115.0f;
		Preset.WearRate = 0.015f; // Semi-slick wears faster

		return Preset;
	}

	/**
	 * NIO EP9 Rear Tire - Michelin Pilot Sport Cup 2
	 * 325/30R20 - Wider rear for traction
	 */
	static FTireModelPreset CreateEP9RearPreset()
	{
		FTireModelPreset Preset;
		Preset.PresetName = TEXT("EP9 Rear");

		// Rear tires: slightly less responsive, more stable
		Preset.Pacejka.Longitudinal.B = 13.0f;
		Preset.Pacejka.Longitudinal.C = 1.65f;
		Preset.Pacejka.Longitudinal.D = 1.0f;
		Preset.Pacejka.Longitudinal.E = -0.12f;

		Preset.Pacejka.Lateral.B = 11.0f;
		Preset.Pacejka.Lateral.C = 1.60f;
		Preset.Pacejka.Lateral.D = 1.0f;
		Preset.Pacejka.Lateral.E = -0.30f;

		Preset.Pacejka.CombinedSlipFactor = 0.86f;

		// Wider rear tires
		Preset.TireWidthMm = 325;
		Preset.AspectRatio = 30;
		Preset.RimDiameterInches = 20;

		Preset.PressurePSI = 31.0f;
		Preset.BaseFriction = 1.18f; // More grip from wider contact patch
		Preset.RollingResistance = 0.014f;
		Preset.TireMassKg = 13.0f;

		Preset.OptimalTemp = 90.0f;
		Preset.ColdThreshold = 45.0f;
		Preset.OverheatThreshold = 115.0f;
		Preset.WearRate = 0.015f;

		return Preset;
	}

	// ========================================================================
	// ET7 Luxury Sedan Presets
	// ========================================================================

	/**
	 * NIO ET7 Front Tire - Continental SportContact 6
	 * 245/45R20 - Performance touring
	 */
	static FTireModelPreset CreateET7FrontPreset()
	{
		FTireModelPreset Preset;
		Preset.PresetName = TEXT("ET7 Front");

		// Comfortable touring tire: less aggressive than EP9
		Preset.Pacejka.Longitudinal.B = 10.0f;
		Preset.Pacejka.Longitudinal.C = 1.65f;
		Preset.Pacejka.Longitudinal.D = 1.0f;
		Preset.Pacejka.Longitudinal.E = -0.10f;

		Preset.Pacejka.Lateral.B = 9.0f;
		Preset.Pacejka.Lateral.C = 1.50f;
		Preset.Pacejka.Lateral.D = 1.0f;
		Preset.Pacejka.Lateral.E = -0.25f;

		Preset.Pacejka.CombinedSlipFactor = 0.82f;

		Preset.TireWidthMm = 245;
		Preset.AspectRatio = 45;
		Preset.RimDiameterInches = 20;

		Preset.PressurePSI = 36.0f; // Higher pressure for comfort/efficiency
		Preset.BaseFriction = 1.05f;
		Preset.RollingResistance = 0.010f; // Low rolling resistance for EV range
		Preset.TireMassKg = 12.0f;

		Preset.OptimalTemp = 80.0f;
		Preset.ColdThreshold = 35.0f;
		Preset.OverheatThreshold = 105.0f;
		Preset.WearRate = 0.008f; // Longer lasting

		return Preset;
	}

	/**
	 * NIO ET7 Rear Tire
	 * 275/40R20
	 */
	static FTireModelPreset CreateET7RearPreset()
	{
		FTireModelPreset Preset;
		Preset.PresetName = TEXT("ET7 Rear");

		Preset.Pacejka.Longitudinal.B = 9.5f;
		Preset.Pacejka.Longitudinal.C = 1.65f;
		Preset.Pacejka.Longitudinal.D = 1.0f;
		Preset.Pacejka.Longitudinal.E = -0.08f;

		Preset.Pacejka.Lateral.B = 8.5f;
		Preset.Pacejka.Lateral.C = 1.50f;
		Preset.Pacejka.Lateral.D = 1.0f;
		Preset.Pacejka.Lateral.E = -0.22f;

		Preset.Pacejka.CombinedSlipFactor = 0.80f;

		Preset.TireWidthMm = 275;
		Preset.AspectRatio = 40;
		Preset.RimDiameterInches = 20;

		Preset.PressurePSI = 35.0f;
		Preset.BaseFriction = 1.08f;
		Preset.RollingResistance = 0.011f;
		Preset.TireMassKg = 13.0f;

		Preset.OptimalTemp = 80.0f;
		Preset.ColdThreshold = 35.0f;
		Preset.OverheatThreshold = 105.0f;
		Preset.WearRate = 0.008f;

		return Preset;
	}

	// ========================================================================
	// ES7 SUV Presets
	// ========================================================================

	/**
	 * NIO ES7 Front Tire - Pirelli P Zero
	 * 265/45R21 - Performance SUV tire
	 */
	static FTireModelPreset CreateES7FrontPreset()
	{
		FTireModelPreset Preset;
		Preset.PresetName = TEXT("ES7 Front");

		// SUV tire: higher profile, more compliant
		Preset.Pacejka.Longitudinal.B = 9.0f;
		Preset.Pacejka.Longitudinal.C = 1.60f;
		Preset.Pacejka.Longitudinal.D = 1.0f;
		Preset.Pacejka.Longitudinal.E = -0.08f;

		Preset.Pacejka.Lateral.B = 8.0f;
		Preset.Pacejka.Lateral.C = 1.45f;
		Preset.Pacejka.Lateral.D = 1.0f;
		Preset.Pacejka.Lateral.E = -0.20f;

		Preset.Pacejka.CombinedSlipFactor = 0.78f;

		Preset.TireWidthMm = 265;
		Preset.AspectRatio = 45;
		Preset.RimDiameterInches = 21;

		Preset.PressurePSI = 38.0f;
		Preset.BaseFriction = 1.02f;
		Preset.RollingResistance = 0.013f;
		Preset.TireMassKg = 14.0f;

		Preset.OptimalTemp = 78.0f;
		Preset.ColdThreshold = 32.0f;
		Preset.OverheatThreshold = 100.0f;
		Preset.WearRate = 0.010f;

		return Preset;
	}

	/**
	 * NIO ES7 Rear Tire
	 * 295/40R21
	 */
	static FTireModelPreset CreateES7RearPreset()
	{
		FTireModelPreset Preset;
		Preset.PresetName = TEXT("ES7 Rear");

		Preset.Pacejka.Longitudinal.B = 8.5f;
		Preset.Pacejka.Longitudinal.C = 1.60f;
		Preset.Pacejka.Longitudinal.D = 1.0f;
		Preset.Pacejka.Longitudinal.E = -0.07f;

		Preset.Pacejka.Lateral.B = 7.5f;
		Preset.Pacejka.Lateral.C = 1.45f;
		Preset.Pacejka.Lateral.D = 1.0f;
		Preset.Pacejka.Lateral.E = -0.18f;

		Preset.Pacejka.CombinedSlipFactor = 0.76f;

		Preset.TireWidthMm = 295;
		Preset.AspectRatio = 40;
		Preset.RimDiameterInches = 21;

		Preset.PressurePSI = 37.0f;
		Preset.BaseFriction = 1.05f;
		Preset.RollingResistance = 0.014f;
		Preset.TireMassKg = 15.0f;

		Preset.OptimalTemp = 78.0f;
		Preset.ColdThreshold = 32.0f;
		Preset.OverheatThreshold = 100.0f;
		Preset.WearRate = 0.010f;

		return Preset;
	}

	// ========================================================================
	// ET5 Sport Sedan Presets
	// ========================================================================

	/**
	 * NIO ET5 Front Tire - Michelin Pilot Sport 4S
	 * 245/40R20
	 */
	static FTireModelPreset CreateET5FrontPreset()
	{
		FTireModelPreset Preset;
		Preset.PresetName = TEXT("ET5 Front");

		// Sport sedan: balanced between comfort and performance
		Preset.Pacejka.Longitudinal.B = 11.0f;
		Preset.Pacejka.Longitudinal.C = 1.65f;
		Preset.Pacejka.Longitudinal.D = 1.0f;
		Preset.Pacejka.Longitudinal.E = -0.12f;

		Preset.Pacejka.Lateral.B = 10.0f;
		Preset.Pacejka.Lateral.C = 1.55f;
		Preset.Pacejka.Lateral.D = 1.0f;
		Preset.Pacejka.Lateral.E = -0.28f;

		Preset.Pacejka.CombinedSlipFactor = 0.84f;

		Preset.TireWidthMm = 245;
		Preset.AspectRatio = 40;
		Preset.RimDiameterInches = 20;

		Preset.PressurePSI = 35.0f;
		Preset.BaseFriction = 1.10f;
		Preset.RollingResistance = 0.011f;
		Preset.TireMassKg = 11.5f;

		Preset.OptimalTemp = 85.0f;
		Preset.ColdThreshold = 40.0f;
		Preset.OverheatThreshold = 110.0f;
		Preset.WearRate = 0.012f;

		return Preset;
	}

	/**
	 * NIO ET5 Rear Tire
	 * 275/35R20
	 */
	static FTireModelPreset CreateET5RearPreset()
	{
		FTireModelPreset Preset;
		Preset.PresetName = TEXT("ET5 Rear");

		Preset.Pacejka.Longitudinal.B = 10.5f;
		Preset.Pacejka.Longitudinal.C = 1.65f;
		Preset.Pacejka.Longitudinal.D = 1.0f;
		Preset.Pacejka.Longitudinal.E = -0.10f;

		Preset.Pacejka.Lateral.B = 9.5f;
		Preset.Pacejka.Lateral.C = 1.55f;
		Preset.Pacejka.Lateral.D = 1.0f;
		Preset.Pacejka.Lateral.E = -0.25f;

		Preset.Pacejka.CombinedSlipFactor = 0.82f;

		Preset.TireWidthMm = 275;
		Preset.AspectRatio = 35;
		Preset.RimDiameterInches = 20;

		Preset.PressurePSI = 34.0f;
		Preset.BaseFriction = 1.12f;
		Preset.RollingResistance = 0.012f;
		Preset.TireMassKg = 12.5f;

		Preset.OptimalTemp = 85.0f;
		Preset.ColdThreshold = 40.0f;
		Preset.OverheatThreshold = 110.0f;
		Preset.WearRate = 0.012f;

		return Preset;
	}

	// ========================================================================
	// Wet Weather Preset
	// ========================================================================

	/**
	 * Wet weather tire preset - reduced grip, better water evacuation
	 * Can be applied to any vehicle when rain is active
	 */
	static FTireModelPreset CreateWetWeatherPreset(const FTireModelPreset& BasePreset)
	{
		FTireModelPreset WetPreset = BasePreset;
		WetPreset.PresetName = BasePreset.PresetName + TEXT(" (Wet)");

		// Reduce Pacejka peak forces for wet conditions
		WetPreset.Pacejka.Longitudinal.D *= 0.7f;
		WetPreset.Pacejka.Lateral.D *= 0.7f;

		// Less stiff response in wet
		WetPreset.Pacejka.Longitudinal.B *= 0.85f;
		WetPreset.Pacejka.Lateral.B *= 0.85f;

		// Lower base friction
		WetPreset.BaseFriction *= 0.7f;

		// Higher rolling resistance (water drag)
		WetPreset.RollingResistance *= 1.3f;

		// Tires run cooler in wet (water cooling)
		WetPreset.OptimalTemp -= 10.0f;
		WetPreset.ColdThreshold -= 5.0f;
		WetPreset.OverheatThreshold -= 10.0f;

		return WetPreset;
	}

	// ========================================================================
	// Utility Functions
	// ========================================================================

	/**
	 * Get front and rear presets for a specific NIO vehicle type
	 */
	UFUNCTION(BlueprintCallable, Category = "NIO Tire Presets", meta = (AutoCreateRefTerm = "VehicleType"))
	static void GetPresetsForVehicle(ENIOVehicleType VehicleType, FTireModelPreset& OutFront, FTireModelPreset& OutRear)
	{
		switch (VehicleType)
		{
		case ENIOVehicleType::EP9:
			OutFront = CreateEP9FrontPreset();
			OutRear = CreateEP9RearPreset();
			break;

		case ENIOVehicleType::ET7:
			OutFront = CreateET7FrontPreset();
			OutRear = CreateET7RearPreset();
			break;

		case ENIOVehicleType::ES7:
			OutFront = CreateES7FrontPreset();
			OutRear = CreateES7RearPreset();
			break;

		case ENIOVehicleType::ET5:
			OutFront = CreateET5FrontPreset();
			OutRear = CreateET5RearPreset();
			break;

		default:
			// Default to sport sedan
			OutFront = CreateET7FrontPreset();
			OutRear = CreateET7RearPreset();
			break;
		}
	}
};
