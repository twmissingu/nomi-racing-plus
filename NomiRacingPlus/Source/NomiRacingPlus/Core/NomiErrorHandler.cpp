// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Core/NomiErrorHandler.h"
#include "NomiRacingPlus.h"

// Static delegate definition
FOnErrorLogged NomiError::OnError;

void NomiError::Log(ENomiErrorSeverity Severity, const TCHAR* Category, const FString& Message)
{
	switch (Severity)
	{
	case ENomiErrorSeverity::Info:
		UE_LOG(LogNomiRacing, Log, TEXT("[%s] %s"), Category, *Message);
		break;
	case ENomiErrorSeverity::Warning:
		UE_LOG(LogNomiRacing, Warning, TEXT("[%s] %s"), Category, *Message);
		break;
	case ENomiErrorSeverity::Error:
		UE_LOG(LogNomiRacing, Error, TEXT("[%s] %s"), Category, *Message);
		break;
	case ENomiErrorSeverity::Critical:
		UE_LOG(LogNomiRacing, Error, TEXT("[%s] CRITICAL: %s"), Category, *Message);
		break;
	}

	// Broadcast to UI listeners (Warning+ only — Info is too noisy for toasts)
	if (Severity >= ENomiErrorSeverity::Warning)
	{
		FString FullMessage = FString::Printf(TEXT("[%s] %s"), Category, *Message);
		OnError.Broadcast(Severity, FullMessage);
	}
}

FNomiResultVoid NomiError::Validate(bool bCondition, const FString& Context)
{
	if (!bCondition)
	{
		return FNomiResultVoid::Failure(Context);
	}
	return FNomiResultVoid::Success();
}

FNomiResultVoid NomiError::ValidateFileExists(const FString& FilePath)
{
	if (!FPaths::FileExists(FilePath))
	{
		return FNomiResultVoid::Failure(
			FString::Printf(TEXT("File not found: %s"), *FilePath));
	}
	return FNomiResultVoid::Success();
}

FNomiResult<float> NomiError::SafeDivide(float Numerator, float Denominator)
{
	if (FMath::IsNearlyZero(Denominator))
	{
		return FNomiResult<float>(TEXT("Division by zero"), ENomiErrorSeverity::Warning);
	}
	return FNomiResult<float>(Numerator / Denominator);
}
