// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

/**
 * Shared JSON serialization helpers for deterministic checksum computation.
 * Used by NomiGameInstance and ProgressionSerializer to avoid code duplication.
 */
namespace JsonSerializationHelpers
{
	/** Calculate CRC32 checksum of a UTF-8 string. */
	inline uint32 CalculateCRC32(const FString& Content)
	{
		FTCHARToUTF8 Converter(*Content);
		return FCrc::MemCrc32(Converter.Get(), Converter.Length());
	}

	/** Serialize a JSON value to string. */
	inline FString SerializeJsonValue(const TSharedPtr<FJsonValue>& Val)
	{
		switch (Val->Type)
		{
		case EJson::String: return FString::Printf(TEXT("\"%s\""), *Val->AsString());
		case EJson::Number: return FString::Printf(TEXT("%g"), Val->AsNumber());
		case EJson::Boolean: return Val->AsBool() ? TEXT("true") : TEXT("false");
		case EJson::Null: return TEXT("null");
		case EJson::Object: return SerializeSorted(Val->AsObject());
		case EJson::Array:
		{
			FString Result = TEXT("[");
			const TArray<TSharedPtr<FJsonValue>>& Arr = Val->AsArray();
			for (int32 j = 0; j < Arr.Num(); ++j)
			{
				if (j > 0) Result += TEXT(",");
				Result += SerializeJsonValue(Arr[j]);
			}
			Result += TEXT("]");
			return Result;
		}
		default: return TEXT("null");
		}
	}

	/** Serialize a JSON object with sorted keys for deterministic checksum. */
	inline FString SerializeSorted(const TSharedPtr<FJsonObject>& Obj)
	{
		FString Result;
		Result += TEXT("{");

		TArray<FString> Keys;
		Obj->Values.GetKeys(Keys);
		Keys.Sort();

		for (int32 i = 0; i < Keys.Num(); ++i)
		{
			if (i > 0) Result += TEXT(",");
			Result += FString::Printf(TEXT("\"%s\":"), *Keys[i]);

			const TSharedPtr<FJsonValue>& Val = Obj->Values[Keys[i]];
			switch (Val->Type)
			{
			case EJson::String:
				Result += FString::Printf(TEXT("\"%s\""), *Val->AsString());
				break;
			case EJson::Number:
				Result += FString::Printf(TEXT("%g"), Val->AsNumber());
				break;
			case EJson::Boolean:
				Result += Val->AsBool() ? TEXT("true") : TEXT("false");
				break;
			case EJson::Object:
				Result += SerializeSorted(Val->AsObject());
				break;
			case EJson::Array:
				Result += SerializeJsonValue(Val);
				break;
			default:
				Result += TEXT("null");
				break;
			}
		}

		Result += TEXT("}");
		return Result;
	}
} // namespace JsonSerializationHelpers
