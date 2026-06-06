// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NomiErrorHandler.generated.h"

/**
 * Error severity levels for structured logging and UI display
 */
UENUM(BlueprintType)
enum class ENomiErrorSeverity : uint8
{
	Info     UMETA(DisplayName = "Info"),
	Warning  UMETA(DisplayName = "Warning"),
	Error    UMETA(DisplayName = "Error"),
	Critical UMETA(DisplayName = "Critical")
};

/**
 * Lightweight result type for consistent error handling.
 * Use FNomiResultVoid for operations that return no value.
 *
 * Usage:
 *   FNomiResult<int32> Result = NomiError::SafeDivide(10, 0);
 *   if (!Result) { UE_LOG(..., *Result.ErrorMessage); }
 */
template<typename T>
struct FNomiResult
{
	bool bSuccess;
	T Value;
	FString ErrorMessage;
	ENomiErrorSeverity Severity;

	FNomiResult()
		: bSuccess(false)
		, Value(T())
		, Severity(ENomiErrorSeverity::Error)
	{
	}

	explicit FNomiResult(const T& InValue)
		: bSuccess(true)
		, Value(InValue)
		, Severity(ENomiErrorSeverity::Info)
	{
	}

	FNomiResult(const FString& InError, ENomiErrorSeverity InSeverity = ENomiErrorSeverity::Error)
		: bSuccess(false)
		, Value(T())
		, ErrorMessage(InError)
		, Severity(InSeverity)
	{
	}

	operator bool() const { return bSuccess; }
	const T& GetValue() const { return Value; }
};

/**
 * Specialization for void-returning operations
 */
template<>
struct FNomiResult<void>
{
	bool bSuccess;
	FString ErrorMessage;
	ENomiErrorSeverity Severity;

	FNomiResult()
		: bSuccess(false)
		, Severity(ENomiErrorSeverity::Error)
	{
	}

	static FNomiResult<void> Success()
	{
		FNomiResult<void> R;
		R.bSuccess = true;
		R.Severity = ENomiErrorSeverity::Info;
		return R;
	}

	static FNomiResult<void> Failure(const FString& InError, ENomiErrorSeverity InSeverity = ENomiErrorSeverity::Error)
	{
		FNomiResult<void> R;
		R.bSuccess = false;
		R.ErrorMessage = InError;
		R.Severity = InSeverity;
		return R;
	}

	operator bool() const { return bSuccess; }
};

using FNomiResultVoid = FNomiResult<void>;

/**
 * Delegate for error toast notifications.
 * Broadcast when NomiError::Log() is called with Warning+ severity.
 *
 * Usage:
 *   NomiError::OnError.AddDynamic(this, &UMyWidget::HandleError);
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnErrorLogged, ENomiErrorSeverity, Severity, const FString&, Message);

/**
 * Centralized error handling utility.
 * Provides structured logging, null-check helpers, and validation wrappers
 * that replace the current mix of bool/nullptr/log-and-continue patterns.
 */
class NOMIRACINGPLUS_API NomiError
{
public:
	/** Broadcast when an error is logged — ErrorToastWidget binds to this */
	static FOnErrorLogged OnError;

	/** Log an error with severity and optional context.
	 *  Broadcasts OnError delegate for Warning+ severity so UI can show toasts.
	 */
	static void Log(ENomiErrorSeverity Severity, const TCHAR* Category, const FString& Message);

	/** Check a pointer and return a Result — replaces scattered null-check-and-log patterns */
	template<typename T>
	static FNomiResult<T*> CheckPointer(T* Ptr, const FString& Name)
	{
		if (!Ptr)
		{
			return FNomiResult<T*>(FString::Printf(TEXT("%s is null"), *Name), ENomiErrorSeverity::Error);
		}
		return FNomiResult<T*>(Ptr);
	}

	/** Validate a condition and return a ResultVoid */
	static FNomiResultVoid Validate(bool bCondition, const FString& Context);

	/** Validate a file exists */
	static FNomiResultVoid ValidateFileExists(const FString& FilePath);

	/** Safe divide that returns a Result instead of producing NaN/Inf */
	static FNomiResult<float> SafeDivide(float Numerator, float Denominator);
};
