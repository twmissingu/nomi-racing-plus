// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "AssetValidator.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundWave.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Json.h"
#include "UObject/UObjectGlobals.h"

UAssetValidator::UAssetValidator()
{
	// Initialize allowed texture formats
	if (ValidationRules.AllowedTextureFormats.Num() == 0)
	{
		ValidationRules.AllowedTextureFormats = {
			TEXT("DXT1"), TEXT("DXT5"), TEXT("DXT3"),
			TEXT("BC7"), TEXT("BC4"), TEXT("BC5"),
			TEXT("ETC2"), TEXT("ASTC")
		};
	}
	if (ValidationRules.RequiredPrefixes.Num() == 0)
	{
		ValidationRules.RequiredPrefixes.Add(TEXT("Mesh"), TEXT("SM_"));
		ValidationRules.RequiredPrefixes.Add(TEXT("Texture"), TEXT("T_"));
		ValidationRules.RequiredPrefixes.Add(TEXT("Material"), TEXT("MI_"));
		ValidationRules.RequiredPrefixes.Add(TEXT("Audio"), TEXT("A_"));
		ValidationRules.RequiredPrefixes.Add(TEXT("Blueprint"), TEXT("BP_"));
	}
}

FAssetValidationResult UAssetValidator::ValidateAsset(const FString& AssetPath)
{
	FAssetValidationResult Result;
	Result.AssetPath = AssetPath;
	Result.Timestamp = FDateTime::Now();

	if (!ValidationRules.bEnabled)
	{
		Result.bIsValid = true;
		return Result;
	}

	const FString AssetType = GetAssetType(AssetPath);

	if (AssetType == TEXT("Mesh"))
	{
		return ValidateVehicleMesh(AssetPath);
	}
	else if (AssetType == TEXT("Texture"))
	{
		return ValidateTexture(AssetPath);
	}
	else if (AssetType == TEXT("Material"))
	{
		return ValidateMaterial(AssetPath);
	}
	else if (AssetType == TEXT("Audio"))
	{
		return ValidateAudio(AssetPath);
	}

	// Unknown type: mark as warning but not an error
	Result.bIsValid = true;
	Result.Warnings.Add(FString::Printf(TEXT("Unknown asset type for path: %s"), *AssetPath));
	return Result;
}

FAssetValidationReport UAssetValidator::ValidateDirectory(const FString& DirectoryPath, bool bRecursive)
{
	FAssetValidationReport Report;
	Report.Timestamp = FDateTime::Now();

	if (DirectoryPath.IsEmpty())
	{
		return Report;
	}

	// Scan for asset files in directory
	TArray<FString> FoundAssets;
	const FString FullPath = FPaths::ProjectContentDir() / DirectoryPath;
	IFileManager::Get().FindFilesRecursive(FoundAssets, *FullPath, TEXT("*.*"), true, false);

	for (const FString& AssetFile : FoundAssets)
	{
		const FString RelativePath = AssetFile;
		FAssetValidationResult AssetResult = ValidateAsset(RelativePath);
		Report.Results.Add(AssetResult);
		Report.TotalAssets++;

		if (AssetResult.bIsValid)
		{
			if (AssetResult.Warnings.Num() > 0)
			{
				Report.AssetsWithWarnings++;
			}
			else
			{
				Report.AssetsPassed++;
			}
		}
		else
		{
			Report.AssetsFailed++;
		}
	}

	Report.ValidationTime = (FDateTime::Now() - Report.Timestamp).GetTotalSeconds();
	return Report;
}

FAssetValidationResult UAssetValidator::ValidateVehicleMesh(const FString& MeshPath)
{
	FAssetValidationResult Result;
	Result.AssetPath = MeshPath;
	Result.Timestamp = FDateTime::Now();

	if (!ValidationRules.bEnabled)
	{
		Result.bIsValid = true;
		return Result;
	}

	// Try to load the mesh
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
	if (!Mesh)
	{
		// Not a critical error in non-editor builds — asset may not be loadable by path
		Result.bIsValid = true;
		Result.Warnings.Add(FString::Printf(TEXT("Could not load mesh at path: %s (expected in editor only)"), *MeshPath));
		return Result;
	}

	FString ErrorMsg;

	// Check polygon count
	if (!CheckPolygonCount(Mesh, ValidationRules.MaxVehiclePolygons, ErrorMsg))
	{
		Result.bIsValid = false;
		Result.Errors.Add(ErrorMsg);
	}

	// Check LOD levels
	if (!CheckLODLevels(Mesh, ErrorMsg))
	{
		Result.Warnings.Add(ErrorMsg);
	}

	// Check naming convention
	const FString* Prefix = ValidationRules.RequiredPrefixes.Find(TEXT("Mesh"));
	if (Prefix && !CheckNamingConvention(MeshPath, *Prefix, ErrorMsg))
	{
		Result.Warnings.Add(ErrorMsg);
	}

	return Result;
}

FAssetValidationResult UAssetValidator::ValidateTexture(const FString& TexturePath)
{
	FAssetValidationResult Result;
	Result.AssetPath = TexturePath;
	Result.Timestamp = FDateTime::Now();

	if (!ValidationRules.bEnabled)
	{
		Result.bIsValid = true;
		return Result;
	}

	UTexture* Texture = LoadObject<UTexture>(nullptr, *TexturePath);
	if (!Texture)
	{
		Result.bIsValid = true;
		return Result;
	}

	FString ErrorMsg;

	// Check texture size
	if (!CheckTextureSize(Texture, ValidationRules.MaxTextureSize, ErrorMsg))
	{
		Result.bIsValid = false;
		Result.Errors.Add(ErrorMsg);
	}

	// Check texture format
	if (!CheckTextureFormat(Texture, ErrorMsg))
	{
		Result.Warnings.Add(ErrorMsg);
	}

	// Check naming convention
	const FString* Prefix = ValidationRules.RequiredPrefixes.Find(TEXT("Texture"));
	if (Prefix && !CheckNamingConvention(TexturePath, *Prefix, ErrorMsg))
	{
		Result.Warnings.Add(ErrorMsg);
	}

	return Result;
}

FAssetValidationResult UAssetValidator::ValidateMaterial(const FString& MaterialPath)
{
	FAssetValidationResult Result;
	Result.AssetPath = MaterialPath;
	Result.Timestamp = FDateTime::Now();

	if (!ValidationRules.bEnabled)
	{
		Result.bIsValid = true;
		return Result;
	}

	UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
	if (!Material)
	{
		Result.bIsValid = true;
		return Result;
	}

	FString ErrorMsg;

	// Check material complexity
	if (!CheckMaterialComplexity(Material, ErrorMsg))
	{
		Result.Warnings.Add(ErrorMsg);
	}

	// Check naming convention
	const FString* Prefix = ValidationRules.RequiredPrefixes.Find(TEXT("Material"));
	if (Prefix && !CheckNamingConvention(MaterialPath, *Prefix, ErrorMsg))
	{
		Result.Warnings.Add(ErrorMsg);
	}

	return Result;
}

FAssetValidationResult UAssetValidator::ValidateAudio(const FString& AudioPath)
{
	FAssetValidationResult Result;
	Result.AssetPath = AudioPath;
	Result.Timestamp = FDateTime::Now();

	if (!ValidationRules.bEnabled)
	{
		Result.bIsValid = true;
		return Result;
	}

	USoundWave* Sound = LoadObject<USoundWave>(nullptr, *AudioPath);
	if (!Sound)
	{
		Result.bIsValid = true;
		return Result;
	}

	// Audio validation: check duration is reasonable
	if (Sound->Duration <= 0.0f)
	{
		Result.Warnings.Add(FString::Printf(TEXT("Audio asset has zero or negative duration: %s"), *AudioPath));
	}

	// Check naming convention
	{
		FString NamingError;
		const FString* Prefix = ValidationRules.RequiredPrefixes.Find(TEXT("Audio"));
		if (Prefix && !CheckNamingConvention(AudioPath, *Prefix, NamingError))
		{
			Result.Warnings.Add(NamingError);
		}
	}

	return Result;
}

bool UAssetValidator::ExportReport(const FAssetValidationReport& Report, const FString& FilePath)
{
	if (FilePath.IsEmpty())
	{
		return false;
	}

	TSharedPtr<FJsonObject> ReportJson = MakeShareable(new FJsonObject);
	ReportJson->SetNumberField(TEXT("total_assets"), Report.TotalAssets);
	ReportJson->SetNumberField(TEXT("assets_passed"), Report.AssetsPassed);
	ReportJson->SetNumberField(TEXT("assets_failed"), Report.AssetsFailed);
	ReportJson->SetNumberField(TEXT("assets_with_warnings"), Report.AssetsWithWarnings);
	ReportJson->SetNumberField(TEXT("validation_time_seconds"), Report.ValidationTime);

	TArray<TSharedPtr<FJsonValue>> ResultsArray;
	for (const FAssetValidationResult& AssetResult : Report.Results)
	{
		TSharedPtr<FJsonObject> EntryJson = MakeShareable(new FJsonObject);
		EntryJson->SetStringField(TEXT("path"), AssetResult.AssetPath);
		EntryJson->SetBoolField(TEXT("is_valid"), AssetResult.bIsValid);

		TArray<TSharedPtr<FJsonValue>> ErrorsArray;
		for (const FString& Err : AssetResult.Errors)
		{
			ErrorsArray.Add(MakeShareable(new FJsonValueString(Err)));
		}
		EntryJson->SetArrayField(TEXT("errors"), ErrorsArray);

		TArray<TSharedPtr<FJsonValue>> WarningsArray;
		for (const FString& Warn : AssetResult.Warnings)
		{
			WarningsArray.Add(MakeShareable(new FJsonValueString(Warn)));
		}
		EntryJson->SetArrayField(TEXT("warnings"), WarningsArray);

		ResultsArray.Add(MakeShareable(new FJsonValueObject(EntryJson)));
	}
	ReportJson->SetArrayField(TEXT("results"), ResultsArray);

	FString JsonOutput;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonOutput);
	if (FJsonSerializer::Serialize(ReportJson.ToSharedRef(), Writer))
	{
		return FFileHelper::SaveStringToFile(JsonOutput, *FilePath);
	}

	return false;
}

bool UAssetValidator::CheckPolygonCount(UStaticMesh* Mesh, int32 MaxPolygons, FString& ErrorMessage)
{
	if (!Mesh)
	{
		ErrorMessage = TEXT("Null mesh reference");
		return false;
	}

	// Get triangle count from LOD 0 (simplest API, works in all builds)
	int32 TotalTriangles = Mesh->GetNumTriangles(0);

	if (TotalTriangles > MaxPolygons)
	{
		ErrorMessage = FString::Printf(
			TEXT("Mesh %s has %d triangles in LOD0 (max allowed: %d)"),
			*Mesh->GetName(), TotalTriangles, MaxPolygons);
		return false;
	}

	return true;
}

bool UAssetValidator::CheckLODLevels(UStaticMesh* Mesh, FString& ErrorMessage)
{
	if (!Mesh)
	{
		ErrorMessage = TEXT("Null mesh reference");
		return false;
	}

	const int32 LODCount = Mesh->GetNumLODs();

	if (LODCount < 2)
	{
		ErrorMessage = FString::Printf(
			TEXT("Mesh %s has only %d LOD level(s); expected at least 2"),
			*Mesh->GetName(), LODCount);
		return false;
	}

	return true;
}

bool UAssetValidator::CheckTextureSize(UTexture* Texture, int32 MaxSize, FString& ErrorMessage)
{
	if (!Texture)
	{
		ErrorMessage = TEXT("Null texture reference");
		return false;
	}

	const int32 SizeX = Texture->GetSurfaceWidth();
	const int32 SizeY = Texture->GetSurfaceHeight();

	if (SizeX > MaxSize || SizeY > MaxSize)
	{
		ErrorMessage = FString::Printf(
			TEXT("Texture %s is %dx%d (max allowed: %dx%d)"),
			*Texture->GetName(), SizeX, SizeY, MaxSize, MaxSize);
		return false;
	}

	return true;
}

bool UAssetValidator::CheckTextureFormat(UTexture* Texture, FString& ErrorMessage)
{
	if (!Texture)
	{
		ErrorMessage = TEXT("Null texture reference");
		return false;
	}

	// Get pixel format as string
	const EPixelFormat Format = Texture->GetPixelFormat();
	FString FormatName;

	switch (Format)
	{
	case PF_DXT1: FormatName = TEXT("DXT1"); break;
	case PF_DXT3: FormatName = TEXT("DXT3"); break;
	case PF_DXT5: FormatName = TEXT("DXT5"); break;
	case PF_BC4:  FormatName = TEXT("BC4"); break;
	case PF_BC5:  FormatName = TEXT("BC5"); break;
	case PF_BC6H: FormatName = TEXT("BC6H"); break;
	case PF_BC7:  FormatName = TEXT("BC7"); break;
	case PF_ETC1: FormatName = TEXT("ETC1"); break;
	case PF_ETC2_RGB: FormatName = TEXT("ETC2"); break;
	case PF_ASTC_4x4:   FormatName = TEXT("ASTC"); break;
	case PF_ASTC_6x6:   FormatName = TEXT("ASTC"); break;
	case PF_ASTC_8x8:   FormatName = TEXT("ASTC"); break;
	default: FormatName = FString::Printf(TEXT("Unknown(%d)"), static_cast<int32>(Format)); break;
	}

	if (ValidationRules.AllowedTextureFormats.Num() > 0)
	{
		bool bFormatAllowed = false;
		for (const FString& AllowedFormat : ValidationRules.AllowedTextureFormats)
		{
			if (FormatName.Contains(AllowedFormat))
			{
				bFormatAllowed = true;
				break;
			}
		}

		if (!bFormatAllowed)
		{
			ErrorMessage = FString::Printf(
				TEXT("Texture %s uses format %s, which is not in allowed list"),
				*Texture->GetName(), *FormatName);
			return false;
		}
	}

	return true;
}

bool UAssetValidator::CheckMaterialComplexity(UMaterialInterface* Material, FString& ErrorMessage)
{
	if (!Material)
	{
		ErrorMessage = TEXT("Null material reference");
		return false;
	}

	// Get approximate instruction count
	const uint32 InstructionCount = Material->GetNumInstructions();

	if (InstructionCount > static_cast<uint32>(ValidationRules.MaxMaterialInstructions))
	{
		ErrorMessage = FString::Printf(
			TEXT("Material %s has %u instructions (max allowed: %d)"),
			*Material->GetName(), InstructionCount, ValidationRules.MaxMaterialInstructions);
		return false;
	}

	return true;
}

bool UAssetValidator::CheckNamingConvention(const FString& AssetPath, const FString& ExpectedPrefix, FString& ErrorMessage)
{
	if (AssetPath.IsEmpty() || ExpectedPrefix.IsEmpty())
	{
		return true;
	}

	// Extract just the asset name (last part of path)
	FString AssetName = FPaths::GetBaseFilename(AssetPath);
	if (AssetName.Contains(TEXT(".")))
	{
		// Handle package paths like /Game/Mesh.mesh
		AssetName = AssetName.Left(AssetName.Find(TEXT(".")));
	}

	if (!AssetName.StartsWith(ExpectedPrefix))
	{
		ErrorMessage = FString::Printf(
			TEXT("Asset '%s' should start with prefix '%s'"),
			*AssetName, *ExpectedPrefix);
		return false;
	}

	return true;
}

FString UAssetValidator::GetAssetType(const FString& AssetPath) const
{
	if (AssetPath.IsEmpty())
	{
		return TEXT("unknown");
	}

	const FString LowerPath = AssetPath.ToLower();

	// Detect by path patterns
	if (LowerPath.Contains(TEXT("meshes")) || LowerPath.Contains(TEXT("/sm_")) ||
		LowerPath.EndsWith(TEXT(".mesh")) || LowerPath.EndsWith(TEXT(".staticmesh")))
	{
		return TEXT("Mesh");
	}

	if (LowerPath.Contains(TEXT("textures")) || LowerPath.Contains(TEXT("/t_")) ||
		LowerPath.EndsWith(TEXT(".texture2d")) || LowerPath.EndsWith(TEXT(".texture")) ||
		LowerPath.EndsWith(TEXT(".png")) || LowerPath.EndsWith(TEXT(".jpg")) ||
		LowerPath.EndsWith(TEXT(".tga")))
	{
		return TEXT("Texture");
	}

	if (LowerPath.Contains(TEXT("materials")) || LowerPath.Contains(TEXT("/mi_")) ||
		LowerPath.EndsWith(TEXT(".material")) || LowerPath.EndsWith(TEXT(".materialinstance")))
	{
		return TEXT("Material");
	}

	if (LowerPath.Contains(TEXT("audio")) || LowerPath.Contains(TEXT("sound")) ||
		LowerPath.Contains(TEXT("/a_")) ||
		LowerPath.EndsWith(TEXT(".wav")) || LowerPath.EndsWith(TEXT(".ogg")) ||
		LowerPath.EndsWith(TEXT(".soundwave")))
	{
		return TEXT("Audio");
	}

	return TEXT("unknown");
}
