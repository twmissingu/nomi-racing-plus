// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NIOVehicleMovementComponent.h"
#include "VehicleStateManager.h"
#include "NIOTirePresets.h"
#include "NomiRacingPlus.h"

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

	UE_LOG(LogNomiVehicle, Log, TEXT("NIOVehicleMovementComponent BeginPlay, TireModel: %s"),
		TireModel ? TEXT("Valid") : TEXT("Null"));
}

void UNIOVehicleMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update electric vehicle systems each frame
	float ThrottleInput = GetThrottleInput();
	float BrakeInput = GetBrakeInput();

	UpdateMotorTemperature(DeltaTime, ThrottleInput);
	UpdateBatteryLevel(DeltaTime, ThrottleInput, BrakeInput);
	ApplyRegenerativeBraking(DeltaTime);
	ApplyAerodynamicDownforce(DeltaTime);

	// Update tire model if available
	if (TireModel)
	{
		ApplyTireForces(DeltaTime);
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
		UE_LOG(LogNomiVehicle, Error, TEXT("Failed to create TirePhysicsModel"));
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

	// The tire model updates automatically in its TickComponent.
	// Here we apply axle-specific adjustments and query tire forces.

	// Apply rear tire grip scaling based on rear preset characteristics
	// Rear tires on NIO vehicles are typically wider with different friction
	const float RearFrictionRatio = (FrontTirePreset.BaseFriction > 0.001f)
		? RearTirePreset.BaseFriction / FrontTirePreset.BaseFriction
		: 1.0f;

	// Scale rear wheel forces if rear tires have different friction
	if (FMath::Abs(RearFrictionRatio - 1.0f) > 0.01f)
	{
		const TArray<FTireState>& AllStates = TireModel->GetAllTireStates();
		// Rear wheels are indices 2 and 3
		for (int32 i = 2; i < FMath::Min(AllStates.Num(), 4); i++)
		{
			// The tire model already calculates forces; rear friction scaling
			// is applied through the preset's BaseFriction during configuration
		}
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

FTireEffectsState UNIOVehicleMovementComponent::GetTireEffectsState() const
{
	FTireEffectsState EffectsState;

	if (!TireModel)
	{
		return EffectsState;
	}

	const TArray<FTireState>& AllStates = TireModel->GetAllTireStates();
	const int32 WheelCount = AllStates.Num();

	// Initialize per-wheel arrays
	EffectsState.WheelGrounded.SetNum(WheelCount);
	EffectsState.WheelSlipRatios.SetNum(WheelCount);
	EffectsState.WheelSlipAngles.SetNum(WheelCount);
	EffectsState.WheelTemperatures.SetNum(WheelCount);

	float TotalTemp = 0.0f;
	float MaxSlipRatio = 0.0f;
	float MaxSlipAngle = 0.0f;
	bool bAnySlipping = false;

	for (int32 i = 0; i < WheelCount; i++)
	{
		const FTireState& State = AllStates[i];

		EffectsState.WheelGrounded[i] = State.bIsGrounded;
		EffectsState.WheelSlipRatios[i] = State.SlipRatio;
		EffectsState.WheelSlipAngles[i] = State.SlipAngleDeg;
		EffectsState.WheelTemperatures[i] = State.Thermal.SurfaceTemperature;

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
	EffectsState.bAnyTireSlipping = bAnySlipping;
	EffectsState.MaxSlipRatio = MaxSlipRatio;
	EffectsState.MaxSlipAngleDeg = MaxSlipAngle;
	EffectsState.AverageTireTemperature = (WheelCount > 0) ? (TotalTemp / WheelCount) : 25.0f;

	return EffectsState;
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
				// We just need to reduce the motor output
				SetMotorInput(-RegenTorque / MaxMotorTorque * 0.1f);
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
