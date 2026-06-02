// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Vehicles/NIOVehicleBase.h"
#include "NIO_ES7.generated.h"

/**
 * NIO ES7 - Performance SUV
 * - 480kW dual motor AWD
 * - 0-100 km/h: 3.9 seconds
 * - Top speed: 200 km/h
 * - Higher ride height, SUV characteristics
 */
UCLASS(Blueprintable)
class NOMIRACINGPLUS_API ANIO_ES7 : public ANIOVehicleBase
{
	GENERATED_BODY()

public:
	ANIO_ES7();
};
