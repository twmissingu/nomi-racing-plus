// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetValidator.generated.h"

#if WITH_EDITOR

/**
 * Asset validation result
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAssetValidationResult
{
	GENERATED_BODY()

	// Asset path
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FString AssetPath;

	// Is valid?
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	bool bIsValid = true;

	// Error messages
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FString> Errors;

	// Warning messages
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	TArray<FString> Warnings;

	// Validation timestamp
	UPROPERTY(BlueprintReadOnly, Category = "Validation")
	FDateTime Timestamp;
};

/**
 * Validation rules for different asset types
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FValidationRules
{
	GENERATED_BODY()

	// Mesh validation

	// Maximum polygon count for vehicles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	int32 MaxVehiclePolygons = 1500000;

	// Maximum polygon count for LOD0
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	int32 MaxLOD0Polygons = 600000;

	// Maximum polygon count for LOD1
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	int32 MaxLOD1Polygons = 150000;

	// Maximum polygon count for LOD2
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	int32 MaxLOD2Polygons = 30000;

	// Texture validation

	// Maximum texture size (pixels)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
	int32 MaxTextureSize = 4096;

	// Required texture formats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Texture")
	TArray<FString> AllowedTextureFormats;

	// Material validation

	// Maximum material complexity (instructions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	int32 MaxMaterialInstructions = 256;

	// Naming validation

	// Required prefixes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Naming")
	TMap<FString, FString> RequiredPrefixes;

	// Validation enabled?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	bool bEnabled = true;
};

/**
 * Asset Validation Report
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FAssetValidationReport
{
	GENERATED_BODY()

	// Total assets checked
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	int32 TotalAssets = 0;

	// Assets passed
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	int32 AssetsPassed = 0;

	// Assets failed
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	int32 AssetsFailed = 0;

	// Assets with warnings
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	int32 AssetsWithWarnings = 0;

	// Validation results
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	TArray<FAssetValidationResult> Results;

	// Report timestamp
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	FDateTime Timestamp;

	// Total validation time (seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	float ValidationTime = 0.0f;
};

/**
 * Asset Validator
 * Validates game assets for quality, performance, and naming conventions
 */
UCLASS(BlueprintType)
class NOMIRACINGPLUS_API UAssetValidator : public UObject
{
	GENERATED_BODY()

public:
	UAssetValidator();

	// Validate a single asset
	UFUNCTION(BlueprintCallable, Category = "Validation")
	FAssetValidationResult ValidateAsset(const FString& AssetPath);

	// Validate all assets in a directory
	UFUNCTION(BlueprintCallable, Category = "Validation")
	FAssetValidationReport ValidateDirectory(const FString& DirectoryPath, bool bRecursive = true);

	// Validate vehicle mesh
	UFUNCTION(BlueprintCallable, Category = "Validation|Vehicle")
	FAssetValidationResult ValidateVehicleMesh(const FString& MeshPath);

	// Validate texture
	UFUNCTION(BlueprintCallable, Category = "Validation|Texture")
	FAssetValidationResult ValidateTexture(const FString& TexturePath);

	// Validate material
	UFUNCTION(BlueprintCallable, Category = "Validation|Material")
	FAssetValidationResult ValidateMaterial(const FString& MaterialPath);

	// Validate audio
	UFUNCTION(BlueprintCallable, Category = "Validation|Audio")
	FAssetValidationResult ValidateAudio(const FString& AudioPath);

	// Set validation rules
	UFUNCTION(BlueprintCallable, Category = "Validation|Config")
	void SetValidationRules(const FValidationRules& Rules) { ValidationRules = Rules; }

	// Get validation rules
	UFUNCTION(BlueprintCallable, Category = "Validation|Config")
	const FValidationRules& GetValidationRules() const { return ValidationRules; }

	// Export report to file
	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ExportReport(const FAssetValidationReport& Report, const FString& FilePath);

protected:
	// Validation rules
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	FValidationRules ValidationRules;

private:
	// Check mesh polygon count
	bool CheckPolygonCount(UStaticMesh* Mesh, int32 MaxPolygons, FString& ErrorMessage);

	// Check mesh LOD levels
	bool CheckLODLevels(UStaticMesh* Mesh, FString& ErrorMessage);

	// Check texture size
	bool CheckTextureSize(UTexture* Texture, int32 MaxSize, FString& ErrorMessage);

	// Check texture format
	bool CheckTextureFormat(UTexture* Texture, FString& ErrorMessage);

	// Check material complexity
	bool CheckMaterialComplexity(UMaterialInterface* Material, FString& ErrorMessage);

	// Check naming convention
	bool CheckNamingConvention(const FString& AssetPath, const FString& ExpectedPrefix, FString& ErrorMessage);

	// Get asset type from path
	FString GetAssetType(const FString& AssetPath) const;
};

#endif // WITH_EDITOR
