// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "TirePhysicsModel.h"
#include "NIOVehicleMovementComponent.generated.h"

/**
 * Tire effects state exposed to visual/audio systems
 * Contains aggregated tire data for effects like smoke, screech, sparks
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FTireEffectsState
{
	GENERATED_BODY()

	// Is any tire slipping beyond threshold?
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	bool bAnyTireSlipping = false;

	// Maximum slip ratio across all tires (0-1+)
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	float MaxSlipRatio = 0.0f;

	// Maximum slip angle across all tires (degrees)
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	float MaxSlipAngleDeg = 0.0f;

	// Average tire surface temperature (Celsius)
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	float AverageTireTemperature = 25.0f;

	// Per-wheel ground contact status (FL, FR, RL, RR)
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	TArray<bool> WheelGrounded;

	// Per-wheel slip ratio for granular effects
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	TArray<float> WheelSlipRatios;

	// Per-wheel slip angle for granular effects
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	TArray<float> WheelSlipAngles;

	// Per-wheel tire temperature for heat haze effects
	UPROPERTY(BlueprintReadOnly, Category = "Effects")
	TArray<float> WheelTemperatures;
};

/**
 * NIO Electric Vehicle Movement Component
 * Extends ChaosVehicleMovementComponent with electric vehicle physics:
 * - Instant torque from 0 RPM
 * - Single-speed transmission (no gear shifts)
 * - Regenerative braking
 * - Lower center of gravity (battery pack)
 * - Integrated Pacejka tire physics model
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UNIOVehicleMovementComponent : public UChaosWheeledVehicleMovementComponent
{
	GENERATED_BODY()

public:
	UNIOVehicleMovementComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Electric vehicle specific functions

	// Get motor torque at current RPM (electric: high at 0 RPM, decays at high RPM)
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Electric")
	float GetElectricMotorTorque(float CurrentRPM) const;

	// Get effective motor torque with battery level applied (does not mutate PeakMotorTorque)
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Electric")
	float GetEffectiveMotorTorque() const;

	// Get regenerative braking torque
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Electric")
	float GetRegenerativeBrakingTorque() const;

	// Is motor at overheat limit?
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Electric")
	bool IsMotorOverheating() const { return MotorTemperature > MotorOverheatThreshold; }

	// Get motor temperature (0-100)
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Electric")
	float GetMotorTemperature() const { return MotorTemperature; }

	// Get battery level (0-100%)
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Electric")
	float GetBatteryLevel() const { return BatteryLevel; }

	// Apply downforce based on speed (EP9: 2000kg at max speed)
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Aero")
	void ApplyAerodynamicDownforce(float DeltaTime);

	// Configure for specific NIO vehicle type
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Config")
	void ConfigureForNIOVehicle(ENIOVehicleType VehicleType);

	// Get the integrated tire physics model
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Tire")
	UTirePhysicsModel* GetTirePhysicsModel() const { return TireModel; }

	// Get tire state for a specific wheel
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Tire")
	const FTireState& GetTireState(int32 WheelIndex) const;

	// Is any tire slipping significantly?
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Tire")
	bool IsAnyTireSlipping(float Threshold = 0.15f) const;

	// Get tire effects state for visual/audio systems
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Tire")
	FTireEffectsState GetTireEffectsState() const;

protected:
	// Electric motor parameters

	// Peak motor torque in Nm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Electric")
	float PeakMotorTorque = 850.0f;

	// RPM at which torque starts to decay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Electric")
	float TorqueDecayRPM = 3000.0f;

	// Maximum motor RPM
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Electric")
	float MaxMotorRPM = 12000.0f;

	// Motor efficiency (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Electric")
	float MotorEfficiency = 0.95f;

	// Regenerative braking strength (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Electric")
	float RegenBrakingStrength = 0.3f;

	// Battery parameters

	// Current battery level (0-100%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Battery")
	float BatteryLevel = 100.0f;

	// Battery discharge rate (%/second at full throttle)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Battery")
	float BatteryDischargeRate = 0.05f;

	// Battery regen rate (%/second at full regen)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Battery")
	float BatteryRegenRate = 0.02f;

	// Motor thermal parameters

	// Current motor temperature (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Thermal")
	float MotorTemperature = 20.0f;

	// Temperature rise rate (degrees/second at full power)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Thermal")
	float MotorHeatRate = 5.0f;

	// Temperature cooling rate (degrees/second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Thermal")
	float MotorCoolRate = 2.0f;

	// Overheat threshold
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Thermal")
	float MotorOverheatThreshold = 80.0f;

	// Aerodynamic parameters

	// Custom downforce coefficient (overrides parent class)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Aero")
	float NIODownforceCoefficient = 0.0f;

	// Custom drag coefficient (overrides parent class)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Aero")
	float NIODragCoefficient = 0.35f;

	// Frontal area in m^2
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Aero")
	float FrontalArea = 2.2f;

private:
	// Calculate torque curve for electric motor
	float CalculateElectricTorqueCurve(float RPM) const;

	// Apply regenerative braking
	void ApplyRegenerativeBraking(float DeltaTime);

	// Update motor temperature
	void UpdateMotorTemperature(float DeltaTime, float ThrottleInput);

	// Update battery level
	void UpdateBatteryLevel(float DeltaTime, float ThrottleInput, float RegenInput);

	// Initialize tire physics model
	void InitializeTireModel();

	// Apply tire forces to the vehicle simulation
	void ApplyTireForces(float DeltaTime);

	// Tire physics model component (auto-created)
	UPROPERTY()
	TObjectPtr<UTirePhysicsModel> TireModel;

public:
	// Front tire preset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Tire")
	FTireModelPreset FrontTirePreset;

	// Rear tire preset (typically wider, different compound)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Tire")
	FTireModelPreset RearTirePreset;
};
