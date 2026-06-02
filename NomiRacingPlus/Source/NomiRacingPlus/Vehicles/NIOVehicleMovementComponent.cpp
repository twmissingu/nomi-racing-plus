// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Vehicles/NIOVehicleMovementComponent.h"
#include "Vehicles/VehicleStateManager.h"
#include "Vehicles/NIOTirePresets.h"
#include "NomiRacingPlus.h"

UNIOVehicleMovementComponent::UNIOVehicleMovementComponent()
{
	// Electric vehicles have instant response
	MaxMotorRPM = 12000.0f;
	TorqueDecayRPM = 3000.0f;
}

void UNIOVehicleMovementComponent::SetupVehicleMovement()
{
	Super::SetupVehicleMovement();

	// Configure for electric vehicle
	// Override engine setup for electric motor behavior
	if (EngineSetup)
	{
		// Electric motors have max torque at 0 RPM
		// The actual torque curve will be applied in UpdateSimulation
		UE_LOG(LogNomiVehicle, Log, TEXT("NIO Vehicle Movement Component initialized"));
	}

	// Initialize tire physics model
	InitializeTireModel();
}

void UNIOVehicleMovementComponent::UpdateSimulation(float DeltaTime)
{
	// Apply electric motor torque
	float CurrentRPM = GetEngineRotationSpeed();
	float ElectricTorque = GetElectricMotorTorque(CurrentRPM);

	// Get input states
	float ThrottleInput = GetThrottleInput();
	float BrakeInput = GetBrakeInput();

	// Update battery and thermal systems
	UpdateBatteryLevel(DeltaTime, ThrottleInput, BrakeInput * RegenBrakingStrength);
	UpdateMotorTemperature(DeltaTime, ThrottleInput);

	// Apply regenerative braking
	ApplyRegenerativeBraking(DeltaTime);

	// Apply aerodynamic effects
	ApplyAerodynamicDownforce(DeltaTime);

	// Apply tire physics forces (after base simulation)
	if (TireModel)
	{
		ApplyTireForces(DeltaTime);
	}

	// Call parent simulation
	Super::UpdateSimulation(DeltaTime);
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
	TireModel->RegisterComponent();

	// Apply appropriate preset based on vehicle type
	// Default to EP9 preset, can be overridden via ConfigureForNIOVehicle
	FTireModelPreset FrontPreset = UNIOTirePresets::CreateEP9FrontPreset();
	FTireModelPreset RearPreset = UNIOTirePresets::CreateEP9RearPreset();

	// For now, apply the front preset as default
	// In a full implementation, front/rear would be handled separately
	TireModel->ApplyPreset(FrontPreset);

	UE_LOG(LogNomiVehicle, Log, TEXT("Tire physics model initialized"));
}

void UNIOVehicleMovementComponent::ApplyTireForces(float DeltaTime)
{
	if (!TireModel)
	{
		return;
	}

	// The tire model updates automatically in its TickComponent
	// Here we can query tire forces for additional effects (e.g., tire smoke, audio)
	// The forces are calculated but applied through the Pacejka model integration

	// Example: Check for tire slip to trigger effects
	if (TireModel->IsAnyTireSlipping(0.15f))
	{
		// Could trigger tire smoke particle system
		// Could trigger tire screech audio
		// These would be handled by the visual/audio systems querying tire state
	}
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

		// Apply regen as engine braking
		// This creates a deceleration effect when lifting off throttle
		if (GetThrottleInput() < 0.01f)
		{
			// Simulate engine braking effect
			// The actual force application happens through Chaos physics
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
	if (DownforceCoefficient <= 0.0f)
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
	float DownforceNewtons = DynamicPressure * FrontalArea * DownforceCoefficient;

	// Convert to UE force units (assumes cm-based coordinate system)
	float DownforceKg = DownforceNewtons / AeroConstants::GravityCmPerSec2;

	// Apply downforce as additional gravity
	if (UPrimitiveComponent* VehicleBody = GetUpdatedPrimitive())
	{
		FVector DownforceVector = FVector(0.0f, 0.0f, -DownforceKg * 100.0f);
		VehicleBody->AddForce(DownforceVector);
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
		DownforceCoefficient = 3.5f;  // High downforce
		DragCoefficient = 0.30f;
		FrontalArea = 2.0f;
		BatteryDischargeRate = 0.08f;  // Higher drain for more power

		// Apply EP9 tire presets (semi-slick performance tires)
		if (TireModel)
		{
			FTireModelPreset FrontPreset = UNIOTirePresets::CreateEP9FrontPreset();
			TireModel->ApplyPreset(FrontPreset);
		}
		break;

	case ENIOVehicleType::ET7:
		// Luxury sedan configuration
		PeakMotorTorque = 850.0f;
		TorqueDecayRPM = 3500.0f;
		MaxMotorRPM = 12000.0f;
		RegenBrakingStrength = 0.3f;
		DownforceCoefficient = 0.5f;
		DragCoefficient = 0.23f;  // Very aerodynamic
		FrontalArea = 2.3f;
		BatteryDischargeRate = 0.04f;

		// Apply ET7 tire presets (touring performance tires)
		if (TireModel)
		{
			FTireModelPreset FrontPreset = UNIOTirePresets::CreateET7FrontPreset();
			TireModel->ApplyPreset(FrontPreset);
		}
		break;

	case ENIOVehicleType::ES7:
		// SUV configuration
		PeakMotorTorque = 850.0f;
		TorqueDecayRPM = 3000.0f;
		MaxMotorRPM = 12000.0f;
		RegenBrakingStrength = 0.25f;
		DownforceCoefficient = 0.3f;
		DragCoefficient = 0.32f;  // Higher drag (SUV shape)
		FrontalArea = 2.8f;
		BatteryDischargeRate = 0.05f;

		// Apply ES7 tire presets (SUV performance tires)
		if (TireModel)
		{
			FTireModelPreset FrontPreset = UNIOTirePresets::CreateES7FrontPreset();
			TireModel->ApplyPreset(FrontPreset);
		}
		break;

	case ENIOVehicleType::ET5:
		// Sport sedan configuration
		PeakMotorTorque = 700.0f;
		TorqueDecayRPM = 3500.0f;
		MaxMotorRPM = 12000.0f;
		RegenBrakingStrength = 0.3f;
		DownforceCoefficient = 0.4f;
		DragCoefficient = 0.24f;
		FrontalArea = 2.2f;
		BatteryDischargeRate = 0.04f;

		// Apply ET5 tire presets (sport sedan tires)
		if (TireModel)
		{
			FTireModelPreset FrontPreset = UNIOTirePresets::CreateET5FrontPreset();
			TireModel->ApplyPreset(FrontPreset);
		}
		break;

	default:
		break;
	}

	UE_LOG(LogNomiVehicle, Log, TEXT("Configured for NIO vehicle type: %d"), (int32)VehicleType);
}
