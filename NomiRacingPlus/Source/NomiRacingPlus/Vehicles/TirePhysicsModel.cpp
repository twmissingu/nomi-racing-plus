// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "TirePhysicsModel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "NomiRacingPlus.h"

// ============================================================================
// Pacejka Magic Formula Constants
// ============================================================================
namespace PacejkaConstants
{
	// Slip ratio input scaling (converts raw slip to Pacejka domain)
	constexpr float SlipRatioScale = 100.0f;

	// Slip angle input scaling (converts degrees to Pacejka domain)
	constexpr float SlipAngleScale = 1.0f;

	// Minimum speed for slip calculations (cm/s)
	constexpr float MinSpeedForSlipCmS = 50.0f;

	// Thermal model constants
	constexpr float AmbientTemperature = 25.0f;
	constexpr float HeatTransferRateCore = 0.05f;    // Core temp change rate
	constexpr float HeatTransferRateSurface = 0.2f;   // Surface temp change rate
	constexpr float CoolingRateNatural = 0.01f;        // Natural cooling per second
	constexpr float CoolingRateSpeedFactor = 0.0005f;  // Additional cooling per cm/s
	constexpr float HeatFromSlipRatio = 50.0f;         // Heat generation per unit slip
	constexpr float HeatFromSlipAngle = 2.0f;          // Heat generation per degree slip
	constexpr float WearHeatThreshold = 120.0f;        // Temperature above which wear accelerates
}

// ============================================================================
// FPacejkaCoefficients
// ============================================================================
bool FPacejkaCoefficients::IsValid() const
{
	return B > 0.0f && C > 0.0f && D > 0.0f;
}

// ============================================================================
// UTirePhysicsModel - Constructor
// ============================================================================
UTirePhysicsModel::UTirePhysicsModel()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	// Default sport tire Pacejka coefficients
	// These values are tuned for a high-performance street tire
	PacejkaCoefficients.Longitudinal.B = 12.0f;
	PacejkaCoefficients.Longitudinal.C = 1.65f;
	PacejkaCoefficients.Longitudinal.D = 1.0f;
	PacejkaCoefficients.Longitudinal.E = -0.1f;

	PacejkaCoefficients.Lateral.B = 10.0f;
	PacejkaCoefficients.Lateral.C = 1.55f;
	PacejkaCoefficients.Lateral.D = 1.0f;
	PacejkaCoefficients.Lateral.E = -0.3f;

	PacejkaCoefficients.CombinedSlipFactor = 0.85f;
}

// ============================================================================
// UTirePhysicsModel - Lifecycle
// ============================================================================
void UTirePhysicsModel::BeginPlay()
{
	Super::BeginPlay();

	// Initialize tire states array
	TireStates.SetNum(NumWheels);
	for (int32 i = 0; i < NumWheels; i++)
	{
		TireStates[i].WheelIndex = i;
		TireStates[i].Thermal.OptimalTemperature = CurrentPreset.OptimalTemp;
		TireStates[i].Thermal.ColdThreshold = CurrentPreset.ColdThreshold;
		TireStates[i].Thermal.OverheatThreshold = CurrentPreset.OverheatThreshold;
		TireStates[i].Thermal.CoreTemperature = PacejkaConstants::AmbientTemperature;
		TireStates[i].Thermal.SurfaceTemperature = PacejkaConstants::AmbientTemperature;
		TireStates[i].Thermal.WearFactor = 1.0f;
	}

	// Initialize default surface parameters
	InitializeDefaultSurfaceParams();

	// Cache vehicle movement component
	if (AActor* Owner = GetOwner())
	{
		VehicleMovement = Owner->FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
	}

	UE_LOG(LogNomiVehicle, Log, TEXT("TirePhysicsModel initialized with %d wheels"), NumWheels);
}

void UTirePhysicsModel::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!VehicleMovement)
	{
		return;
	}

	// Update slip calculations for all wheels
	UpdateSlipCalculations(DeltaTime);

	// Update thermal model
	UpdateThermalModel(DeltaTime);
}

// ============================================================================
// Pacejka Magic Formula
// ============================================================================
float UTirePhysicsModel::CalculatePacejkaForce(const FPacejkaCoefficients& Coeffs, float SlipInput)
{
	if (!Coeffs.IsValid())
	{
		return 0.0f;
	}

	const float B = Coeffs.B;
	const float C = Coeffs.C;
	const float D = Coeffs.D;
	const float E = Coeffs.E;

	// Pacejka Magic Formula:
	// F = D * sin(C * arctan(B*x - E*(B*x - arctan(B*x))))
	const float Bx = B * SlipInput;
	const float InnerArc = Bx - E * (Bx - FMath::Atan(Bx));
	const float Force = D * FMath::Sin(C * FMath::Atan(InnerArc));

	return Force;
}

// ============================================================================
// Slip Ratio Calculation
// ============================================================================
float UTirePhysicsModel::CalculateSlipRatio(int32 WheelIndex, float WheelAngularVelocity, float VehicleForwardSpeed) const
{
	// Slip Ratio = (WheelLinearVelocity - VehicleVelocity) / |VehicleVelocity|
	// For driven wheels: positive = wheelspin, negative = locked
	// For free-rolling wheels: close to 0

	// Need tire radius to convert angular to linear velocity
	// Using preset dimensions
	const float TireRadiusM = ((CurrentPreset.TireWidthMm * CurrentPreset.AspectRatio / 100.0f) * 2.0f +
		CurrentPreset.RimDiameterInches * 25.4f) / 2000.0f;

	// Convert wheel angular velocity (rad/s) to linear velocity (cm/s)
	const float WheelLinearVelocity = WheelAngularVelocity * TireRadiusM * 100.0f;

	// Avoid division by zero at very low speeds
	if (FMath::Abs(VehicleForwardSpeed) < PacejkaConstants::MinSpeedForSlipCmS)
	{
		return 0.0f;
	}

	// Slip ratio calculation
	const float SlipRatio = (WheelLinearVelocity - VehicleForwardSpeed) / FMath::Abs(VehicleForwardSpeed);

	return FMath::Clamp(SlipRatio, -2.0f, 2.0f);
}

// ============================================================================
// Slip Angle Calculation
// ============================================================================
float UTirePhysicsModel::CalculateSlipAngle(int32 WheelIndex, const FVector& WheelVelocity, const FVector& WheelForward) const
{
	// Slip angle = atan2(lateral_velocity, longitudinal_velocity)
	// Positive = understeer direction, Negative = oversteer direction

	const FVector VelNorm = WheelVelocity.GetSafeNormal();
	const FVector FwdNorm = WheelForward.GetSafeNormal();

	if (VelNorm.IsNearlyZero() || FwdNorm.IsNearlyZero())
	{
		return 0.0f;
	}

	// Decompose velocity into longitudinal and lateral components relative to wheel
	const float LongitudinalVel = FVector::DotProduct(WheelVelocity, FwdNorm);
	const FVector LateralDir = FVector::CrossProduct(FVector::UpVector, FwdNorm).GetSafeNormal();
	const float LateralVel = FVector::DotProduct(WheelVelocity, LateralDir);

	if (FMath::Abs(LongitudinalVel) < PacejkaConstants::MinSpeedForSlipCmS)
	{
		return 0.0f;
	}

	// Slip angle in radians, converted to degrees
	const float SlipAngleRad = FMath::Atan2(LateralVel, LongitudinalVel);
	const float SlipAngleDeg = FMath::RadiansToDegrees(SlipAngleRad);

	return FMath::Clamp(SlipAngleDeg, -90.0f, 90.0f);
}

// ============================================================================
// Force Outputs
// ============================================================================
float UTirePhysicsModel::GetLongitudinalForce(int32 WheelIndex) const
{
	if (!TireStates.IsValidIndex(WheelIndex))
	{
		return 0.0f;
	}

	return TireStates[WheelIndex].LongitudinalForce;
}

float UTirePhysicsModel::GetLateralForce(int32 WheelIndex) const
{
	if (!TireStates.IsValidIndex(WheelIndex))
	{
		return 0.0f;
	}

	return TireStates[WheelIndex].LateralForce;
}

// ============================================================================
// Thermal Model
// ============================================================================
void UTirePhysicsModel::UpdateThermalState(float DeltaTime, int32 WheelIndex, float SlipMagnitude, float WheelLoad)
{
	if (!TireStates.IsValidIndex(WheelIndex))
	{
		return;
	}

	FTireThermalState& Thermal = TireStates[WheelIndex].Thermal;

	// === Heat Generation ===
	// Heat from slip (both ratio and angle contribute)
	const float SurfaceParamsMult = SurfaceParams.Contains(TireStates[WheelIndex].SurfaceType)
		? SurfaceParams[TireStates[WheelIndex].SurfaceType].HeatGenerationMultiplier
		: 1.0f;

	const float SlipHeat = SlipMagnitude * PacejkaConstants::HeatFromSlipRatio * SurfaceParamsMult;

	// Additional heat from wheel load (more load = more friction = more heat)
	const float LoadHeat = (WheelLoad / 5000.0f) * SlipMagnitude * 5.0f;

	// Total heat input (Joules concept, simplified to temperature change)
	const float TotalHeatInput = (SlipHeat + LoadHeat) * DeltaTime;

	// === Cooling ===
	// Natural convection cooling (proportional to temperature difference from ambient)
	const float TempDiffSurface = Thermal.SurfaceTemperature - PacejkaConstants::AmbientTemperature;
	const float TempDiffCore = Thermal.CoreTemperature - PacejkaConstants::AmbientTemperature;

	// Speed-dependent cooling (airflow)
	float SpeedCoolingFactor = 1.0f;
	if (VehicleMovement)
	{
		const float SpeedCmS = FMath::Abs(VehicleMovement->GetForwardSpeed());
		SpeedCoolingFactor = 1.0f + SpeedCmS * PacejkaConstants::CoolingRateSpeedFactor;
	}

	const float SurfaceCooling = PacejkaConstants::CoolingRateNatural * SpeedCoolingFactor * TempDiffSurface * DeltaTime;
	const float CoreCooling = PacejkaConstants::CoolingRateNatural * SpeedCoolingFactor * 0.5f * TempDiffCore * DeltaTime;

	// === Temperature Update ===
	// Surface temperature responds faster
	Thermal.SurfaceTemperature += TotalHeatInput - SurfaceCooling;
	Thermal.SurfaceTemperature = FMath::Clamp(Thermal.SurfaceTemperature,
		PacejkaConstants::AmbientTemperature, Thermal.MaxTemperature);

	// Core temperature responds slower (thermal mass)
	const float CoreToSurfaceTransfer = PacejkaConstants::HeatTransferRateCore *
		(Thermal.SurfaceTemperature - Thermal.CoreTemperature) * DeltaTime;
	Thermal.CoreTemperature += CoreToSurfaceTransfer - CoreCooling;
	Thermal.CoreTemperature = FMath::Clamp(Thermal.CoreTemperature,
		PacejkaConstants::AmbientTemperature, Thermal.MaxTemperature);

	// === Calculate Thermal Grip Multiplier ===
	Thermal.ThermalGripMultiplier = CalculateThermalGrip(Thermal);

	// === Tire Wear ===
	UpdateTireWear(DeltaTime, WheelIndex, SlipMagnitude);
}

float UTirePhysicsModel::GetThermalGripMultiplier(int32 WheelIndex) const
{
	if (!TireStates.IsValidIndex(WheelIndex))
	{
		return 1.0f;
	}

	return TireStates[WheelIndex].Thermal.ThermalGripMultiplier;
}

float UTirePhysicsModel::CalculateThermalGrip(const FTireThermalState& Thermal) const
{
	const float Temp = Thermal.SurfaceTemperature;
	const float Optimal = Thermal.OptimalTemperature;
	const float Cold = Thermal.ColdThreshold;
	const float Hot = Thermal.OverheatThreshold;

	// Grip curve:
	// Below Cold: grip increases linearly from 0.6 to 1.0
	// Cold to Optimal: grip increases from 0.9 to 1.05 (peak slightly above 1.0)
	// Optimal to Hot: grip decreases from 1.05 to 0.95
	// Above Hot: grip drops sharply (overheating)

	if (Temp < Cold)
	{
		// Cold tires: reduced grip
		const float ColdFactor = FMath::Clamp((Temp - PacejkaConstants::AmbientTemperature) /
			(Cold - PacejkaConstants::AmbientTemperature), 0.0f, 1.0f);
		return 0.6f + ColdFactor * 0.4f; // 0.6 to 1.0
	}

	if (Temp < Optimal)
	{
		// Warming up: increasing grip
		const float WarmFactor = (Temp - Cold) / (Optimal - Cold);
		return 1.0f + WarmFactor * 0.05f; // 1.0 to 1.05
	}

	if (Temp < Hot)
	{
		// Optimal to hot: slight decrease
		const float HotFactor = (Temp - Optimal) / (Hot - Optimal);
		return 1.05f - HotFactor * 0.1f; // 1.05 to 0.95
	}

	// Overheating: significant grip loss
	const float OverheatFactor = FMath::Clamp((Temp - Hot) / (Thermal.MaxTemperature - Hot), 0.0f, 1.0f);
	return 0.95f - OverheatFactor * 0.45f; // 0.95 down to 0.5
}

// ============================================================================
// Surface Detection
// ============================================================================
void UTirePhysicsModel::SetSurfaceType(int32 WheelIndex, ETireSurfaceType SurfaceType)
{
	if (TireStates.IsValidIndex(WheelIndex))
	{
		TireStates[WheelIndex].SurfaceType = SurfaceType;
	}
}

float UTirePhysicsModel::GetSurfaceGripMultiplier(int32 WheelIndex) const
{
	if (!TireStates.IsValidIndex(WheelIndex))
	{
		return 1.0f;
	}

	const ETireSurfaceType Surface = TireStates[WheelIndex].SurfaceType;
	if (SurfaceParams.Contains(Surface))
	{
		return SurfaceParams[Surface].GripMultiplier;
	}

	return 1.0f;
}

// ============================================================================
// Configuration
// ============================================================================
void UTirePhysicsModel::ApplyPreset(const FTireModelPreset& Preset)
{
	CurrentPreset = Preset;
	PacejkaCoefficients = Preset.Pacejka;

	// Update thermal parameters for all tires
	for (FTireState& State : TireStates)
	{
		State.Thermal.OptimalTemperature = Preset.OptimalTemp;
		State.Thermal.ColdThreshold = Preset.ColdThreshold;
		State.Thermal.OverheatThreshold = Preset.OverheatThreshold;
	}

	UE_LOG(LogNomiVehicle, Log, TEXT("Applied tire preset: %s"), *Preset.PresetName);
}

void UTirePhysicsModel::SetPacejkaCoefficients(const FTirePacejkaSet& NewCoeffs)
{
	PacejkaCoefficients = NewCoeffs;
}

// ============================================================================
// State Access
// ============================================================================
const FTireState& UTirePhysicsModel::GetTireState(int32 WheelIndex) const
{
	if (TireStates.IsValidIndex(WheelIndex))
	{
		return TireStates[WheelIndex];
	}

	// Return a default static state if invalid index
	static const FTireState DefaultState;
	return DefaultState;
}

bool UTirePhysicsModel::IsAnyTireSlipping(float Threshold) const
{
	for (const FTireState& State : TireStates)
	{
		if (FMath::Abs(State.SlipRatio) > Threshold || FMath::Abs(State.SlipAngleDeg) > Threshold * 90.0f)
		{
			return true;
		}
	}
	return false;
}

float UTirePhysicsModel::GetAverageTireTemperature() const
{
	if (TireStates.Num() == 0)
	{
		return PacejkaConstants::AmbientTemperature;
	}

	float TotalTemp = 0.0f;
	for (const FTireState& State : TireStates)
	{
		TotalTemp += State.Thermal.SurfaceTemperature;
	}

	return TotalTemp / static_cast<float>(TireStates.Num());
}

// ============================================================================
// Private Implementation
// ============================================================================
void UTirePhysicsModel::InitializeDefaultSurfaceParams()
{
	// Tarmac (Dry) - baseline
	FTireSurfaceParams Tarmac;
	Tarmac.GripMultiplier = 1.0f;
	Tarmac.RollingResistanceMultiplier = 1.0f;
	Tarmac.HeatGenerationMultiplier = 1.0f;
	Tarmac.SurfaceDrag = 0.0f;
	SurfaceParams.Add(ETireSurfaceType::Tarmac, Tarmac);

	// Tarmac (Wet) - reduced grip, higher rolling resistance
	FTireSurfaceParams TarmacWet;
	TarmacWet.GripMultiplier = 0.7f;
	TarmacWet.RollingResistanceMultiplier = 1.3f;
	TarmacWet.HeatGenerationMultiplier = 0.6f;
	TarmacWet.SurfaceDrag = 0.1f;
	SurfaceParams.Add(ETireSurfaceType::TarmacWet, TarmacWet);

	// Gravel - lower grip, high rolling resistance
	FTireSurfaceParams Gravel;
	Gravel.GripMultiplier = 0.6f;
	Gravel.RollingResistanceMultiplier = 2.0f;
	Gravel.HeatGenerationMultiplier = 0.8f;
	Gravel.SurfaceDrag = 0.3f;
	SurfaceParams.Add(ETireSurfaceType::Gravel, Gravel);

	// Sand - very low grip, very high rolling resistance
	FTireSurfaceParams Sand;
	Sand.GripMultiplier = 0.4f;
	Sand.RollingResistanceMultiplier = 3.5f;
	Sand.HeatGenerationMultiplier = 0.5f;
	Sand.SurfaceDrag = 0.5f;
	SurfaceParams.Add(ETireSurfaceType::Sand, Sand);

	// Grass - low grip
	FTireSurfaceParams Grass;
	Grass.GripMultiplier = 0.5f;
	Grass.RollingResistanceMultiplier = 2.5f;
	Grass.HeatGenerationMultiplier = 0.4f;
	Grass.SurfaceDrag = 0.2f;
	SurfaceParams.Add(ETireSurfaceType::Grass, Grass);

	// Concrete - slightly higher grip than tarmac
	FTireSurfaceParams Concrete;
	Concrete.GripMultiplier = 1.05f;
	Concrete.RollingResistanceMultiplier = 0.9f;
	Concrete.HeatGenerationMultiplier = 1.1f;
	Concrete.SurfaceDrag = 0.0f;
	SurfaceParams.Add(ETireSurfaceType::Concrete, Concrete);

	// Ice - very low grip
	FTireSurfaceParams Ice;
	Ice.GripMultiplier = 0.15f;
	Ice.RollingResistanceMultiplier = 0.5f;
	Ice.HeatGenerationMultiplier = 0.3f;
	Ice.SurfaceDrag = 0.0f;
	SurfaceParams.Add(ETireSurfaceType::Ice, Ice);

	// Custom - default to tarmac values
	FTireSurfaceParams Custom;
	Custom.GripMultiplier = 1.0f;
	Custom.RollingResistanceMultiplier = 1.0f;
	Custom.HeatGenerationMultiplier = 1.0f;
	Custom.SurfaceDrag = 0.0f;
	SurfaceParams.Add(ETireSurfaceType::Custom, Custom);
}

void UTirePhysicsModel::UpdateSlipCalculations(float DeltaTime)
{
	if (!VehicleMovement)
	{
		return;
	}

	// Get vehicle forward speed (cm/s)
	const float VehicleForwardSpeed = VehicleMovement->GetForwardSpeed();

	// Get vehicle transform for coordinate conversions
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	const FTransform VehicleTransform = Owner->GetActorTransform();
	const FVector VehicleForward = Owner->GetActorForwardVector();

	// Use configured wheel count (UE5.7 doesn't have GetWheelCount)
	const int32 WheelCount = FMath::Min(TireStates.Num(), 4); // Default to 4 wheels

	// Update each wheel with simplified physics
	for (int32 i = 0; i < WheelCount; i++)
	{
		FTireState& State = TireStates[i];

		// Simplified wheel state (UE5.7 compatible)
		const bool bOnGround = true; // Assume on ground for now
		const float WheelAngularVel = VehicleForwardSpeed / 0.35f; // Approximate from speed
		const float WheelLoad = 1.0f; // Simplified load

		State.bIsGrounded = bOnGround;
		State.WheelLoad = WheelLoad;
		State.WheelAngularVelocity = WheelAngularVel;

		// Calculate slip ratio (longitudinal)
		State.SlipRatio = CalculateSlipRatio(i, WheelAngularVel, VehicleForwardSpeed);

		// Calculate slip angle (lateral)
		const FVector VehicleVelocity = Owner->GetVelocity();

		// Account for steering angle on front wheels
		FVector WheelForwardDir = VehicleForward;
		if (i == 0 || i == 1) // Front wheels (FL=0, FR=1)
		{
			const float SteerAngle = VehicleMovement->GetSteeringInput() * 30.0f; // Max steering ~30 degrees
			const FQuat SteerRotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(SteerAngle));
			WheelForwardDir = SteerRotation.RotateVector(VehicleForward);
		}

		State.SlipAngleDeg = CalculateSlipAngle(i, VehicleVelocity, WheelForwardDir);

		// Calculate forces using Pacejka formula
		if (bOnGround && WheelLoad > 0.0f)
		{
			// Get grip multipliers
			const float ThermalGrip = State.Thermal.ThermalGripMultiplier;
			const float SurfaceGrip = GetSurfaceGripMultiplier(i);
			const float WearGrip = State.Thermal.WearFactor;
			const float TotalGrip = ThermalGrip * SurfaceGrip * WearGrip * CurrentPreset.BaseFriction;

			// Longitudinal force (scaled by slip ratio domain for Pacejka)
			const float LongSlipInput = State.SlipRatio * PacejkaConstants::SlipRatioScale;
			State.LongitudinalForce = CalculatePacejkaForce(PacejkaCoefficients.Longitudinal, LongSlipInput)
				* WheelLoad * TotalGrip;

			// Lateral force (slip angle in degrees)
			const float LatSlipInput = State.SlipAngleDeg * PacejkaConstants::SlipAngleScale;
			State.LateralForce = CalculatePacejkaForce(PacejkaCoefficients.Lateral, LatSlipInput)
				* WheelLoad * TotalGrip;

			// Combined slip reduction
			const float NormLong = FMath::Abs(State.SlipRatio);
			const float NormLat = FMath::Abs(State.SlipAngleDeg) / 45.0f;
			const float CombinedSlipMagnitude = FMath::Sqrt(NormLong * NormLong + NormLat * NormLat);

			if (CombinedSlipMagnitude > 0.01f)
			{
				const float CombinedReduction = FMath::Lerp(1.0f, PacejkaCoefficients.CombinedSlipFactor,
					FMath::Clamp(CombinedSlipMagnitude, 0.0f, 1.0f));
				State.LongitudinalForce *= CombinedReduction;
				State.LateralForce *= CombinedReduction;
			}
		}
		else
		{
			State.LongitudinalForce = 0.0f;
			State.LateralForce = 0.0f;
		}
	}
}

void UTirePhysicsModel::UpdateThermalModel(float DeltaTime)
{
	for (int32 i = 0; i < TireStates.Num(); i++)
	{
		FTireState& State = TireStates[i];

		// Calculate slip magnitude for heat generation (consistent with combined slip calculation)
		const float NormLong = FMath::Abs(State.SlipRatio);
		const float NormLat = FMath::Abs(State.SlipAngleDeg) / 45.0f;
		const float SlipMagnitude = FMath::Sqrt(NormLong * NormLong + NormLat * NormLat);

		UpdateThermalState(DeltaTime, i, SlipMagnitude, State.WheelLoad);
	}
}

void UTirePhysicsModel::UpdateTireWear(float DeltaTime, int32 WheelIndex, float SlipMagnitude)
{
	if (!TireStates.IsValidIndex(WheelIndex))
	{
		return;
	}

	FTireThermalState& Thermal = TireStates[WheelIndex].Thermal;

	// Base wear rate from preset
	float WearRate = CurrentPreset.WearRate;

	// Increased wear at high temperatures
	if (Thermal.SurfaceTemperature > PacejkaConstants::WearHeatThreshold)
	{
		const float OverheatFactor = (Thermal.SurfaceTemperature - PacejkaConstants::WearHeatThreshold) /
			(Thermal.MaxTemperature - PacejkaConstants::WearHeatThreshold);
		WearRate *= (1.0f + OverheatFactor * 3.0f);
	}

	// Increased wear during high slip (burnouts, lockups)
	WearRate *= (1.0f + SlipMagnitude * 2.0f);

	// Apply wear
	Thermal.WearFactor -= WearRate * DeltaTime;
	Thermal.WearFactor = FMath::Clamp(Thermal.WearFactor, 0.0f, 1.0f);

	// Worn tires have reduced peak grip
	if (Thermal.WearFactor < 0.3f)
	{
		// Severely worn: significant grip reduction
		Thermal.ThermalGripMultiplier *= (0.5f + Thermal.WearFactor * 1.67f);
	}
}
