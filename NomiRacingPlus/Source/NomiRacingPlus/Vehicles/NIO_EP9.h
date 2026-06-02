// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Vehicles/NIOVehicleBase.h"
#include "NIO_EP9.generated.h"

/**
 * NIO EP9 - Hypercar
 * - 1000kW (1360 HP) quad motor AWD
 * - 0-100 km/h: 2.7 seconds
 * - Top speed: 313 km/h
 * - 2000kg downforce at max speed
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API ANIO_EP9 : public ANIOVehicleBase
{
	GENERATED_BODY()

public:
	ANIO_EP9();
};
