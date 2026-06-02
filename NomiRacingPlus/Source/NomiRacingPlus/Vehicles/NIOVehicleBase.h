// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Vehicles/VehicleStateManager.h"
#include "Vehicles/NIOVehicleMovementComponent.h"
#include "NIOVehicleBase.generated.h"

/**
 * Base class for all NIO vehicles
 * Integrates VehicleStateManager and NIOVehicleMovementComponent
 */
UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API ANIOVehicleBase : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	ANIOVehicleBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Vehicle State Access

	// Get current vehicle state
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle")
	const FVehicleState& GetVehicleState() const;

	// Get vehicle type
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle")
	ENIOVehicleType GetNIOVehicleType() const { return VehicleType; }

	// Get vehicle display name
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle")
	FString GetVehicleDisplayName() const;

	// Is this a NIO vehicle?
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle")
	bool IsNIOVehicle() const;

	// Performance Configuration

	// Get performance config
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Performance")
	const FNIOPerformanceConfig& GetPerformanceConfig() const;

	// Set performance config
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Performance")
	void SetPerformanceConfig(const FNIOPerformanceConfig& InConfig);

	// Vehicle Actions

	// Play horn sound
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Actions")
	void PlayHorn();

	// Toggle headlights
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Actions")
	void ToggleHeadlights();

	// Set headlight brightness (0-1)
	UFUNCTION(BlueprintCallable, Category = "NIO Vehicle|Actions")
	void SetHeadlightBrightness(float Brightness);

protected:
	// Vehicle type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle")
	ENIOVehicleType VehicleType = ENIOVehicleType::EP9;

	// State manager component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NIO Vehicle")
	TObjectPtr<UVehicleStateManager> StateManager;

	// NIO movement component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NIO Vehicle")
	TObjectPtr<UNIOVehicleMovementComponent> NIOMovement;

	// Headlight components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NIO Vehicle|Lights")
	TObjectPtr<USpotLightComponent> LeftHeadlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NIO Vehicle|Lights")
	TObjectPtr<USpotLightComponent> RightHeadlight;

	// Tail light components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NIO Vehicle|Lights")
	TObjectPtr<UPointLightComponent> LeftTaillight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NIO Vehicle|Lights")
	TObjectPtr<UPointLightComponent> RightTaillight;

	// Horn sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Audio")
	TObjectPtr<USoundBase> HornSound;

	// Motor sound (electric whine)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NIO Vehicle|Audio")
	TObjectPtr<USoundBase> MotorSound;

	// Headlight state
	UPROPERTY(BlueprintReadOnly, Category = "NIO Vehicle|Lights")
	bool bHeadlightsOn = false;

	UPROPERTY(BlueprintReadOnly, Category = "NIO Vehicle|Lights")
	float HeadlightBrightness = 1.0f;

private:
	// Initialize components
	void InitializeComponents();

	// Update lights based on state
	void UpdateLights(float DeltaTime);

	// Update audio based on state
	void UpdateAudio(float DeltaTime);
};
