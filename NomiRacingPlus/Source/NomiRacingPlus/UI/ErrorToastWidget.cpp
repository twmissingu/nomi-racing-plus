// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "UI/ErrorToastWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "NomiRacingPlus.h"

UErrorToastWidget::UErrorToastWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UErrorToastWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// If no UMG-bound container exists, create the layout programmatically
	if (!ToastContainer)
	{
		UCanvasPanel* Root = NewObject<UCanvasPanel>(this, TEXT("ToastRoot"));
		ToastContainer = NewObject<UVerticalBox>(this, TEXT("ToastContainer"));

		UCanvasPanelSlot* ContainerSlot = Root->AddChildToCanvas(ToastContainer);
		if (ContainerSlot)
		{
			ContainerSlot->SetAnchors(FAnchors(1.0f, 0.0f, 1.0f, 0.0f));
			ContainerSlot->SetAlignment(FVector2D(1.0f, 0.0f));
			ContainerSlot->SetPosition(FVector2D(-20.0f, 20.0f));
			ContainerSlot->SetAutoSize(true);
		}

		AddChild(Root);
	}

	// Auto-bind to error handler
	BindToErrorHandler();
}

void UErrorToastWidget::NativeDestruct()
{
	// Unbind from error handler to prevent dangling delegates
	NomiError::OnError.RemoveDynamic(this, &UErrorToastWidget::OnErrorHandlerLog);
	Super::NativeDestruct();
}

void UErrorToastWidget::BindToErrorHandler()
{
	// Add binding — UE5 handles duplicate bindings gracefully
	NomiError::OnError.AddDynamic(this, &UErrorToastWidget::OnErrorHandlerLog);
}

void UErrorToastWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	bool bNeedsRebuild = false;

	// Tick down active toasts
	for (int32 i = ActiveToasts.Num() - 1; i >= 0; --i)
	{
		ActiveToasts[i].RemainingTime -= InDeltaTime;
		if (ActiveToasts[i].RemainingTime <= 0.0f)
		{
			ActiveToasts.RemoveAt(i);
			bNeedsRebuild = true;
		}
	}

	if (bNeedsRebuild)
	{
		RebuildDisplay();
	}
}

void UErrorToastWidget::ShowToast(const FString& Message, ENomiErrorSeverity Severity)
{
	if (Message.IsEmpty())
	{
		return;
	}

	// Also log the message through the structured error handler
	NomiError::Log(Severity, TEXT("Toast"), Message);

	FToastEntry NewEntry;
	NewEntry.Message = Message;
	NewEntry.Severity = Severity;
	NewEntry.RemainingTime = DismissDuration;
	NewEntry.bDismissed = false;

	ActiveToasts.Add(NewEntry);

	// Remove oldest if exceeding max
	while (ActiveToasts.Num() > MaxVisibleToasts)
	{
		ActiveToasts.RemoveAt(0);
	}

	RebuildDisplay();
}

void UErrorToastWidget::DismissAll()
{
	ActiveToasts.Empty();
	RebuildDisplay();
}

void UErrorToastWidget::RebuildDisplay()
{
	if (!ToastContainer)
	{
		return;
	}

	ToastContainer->ClearChildren();

	for (const FToastEntry& Entry : ActiveToasts)
	{
		UTextBlock* Row = CreateToastRow(Entry);
		if (Row)
		{
			UVerticalBoxSlot* Slot = ToastContainer->AddChildToVerticalBox(Row);
			if (Slot)
			{
				Slot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 4.0f));
			}
		}
	}
}

UTextBlock* UErrorToastWidget::CreateToastRow(const FToastEntry& Entry) const
{
	UTextBlock* TextBlock = NewObject<UTextBlock>(const_cast<UErrorToastWidget*>(this));
	if (!TextBlock)
	{
		return nullptr;
	}

	FString DisplayText = GetSeverityPrefix(Entry.Severity) + TEXT(" ") + Entry.Message;
	TextBlock->SetText(FText::FromString(DisplayText));

	FSlateFontInfo FontInfo = TextBlock->GetFont();
	FontInfo.Size = 16;
	TextBlock->SetFont(FontInfo);

	TextBlock->SetColorAndOpacity(FSlateColor(GetSeverityColor(Entry.Severity)));

	return TextBlock;
}

FLinearColor UErrorToastWidget::GetSeverityColor(ENomiErrorSeverity Severity) const
{
	switch (Severity)
	{
	case ENomiErrorSeverity::Info:
		return FLinearColor(0.0f, 0.831f, 1.0f, 1.0f);   // NIO Cyan
	case ENomiErrorSeverity::Warning:
		return FLinearColor(1.0f, 0.722f, 0.0f, 1.0f);    // Amber
	case ENomiErrorSeverity::Error:
		return FLinearColor(1.0f, 0.133f, 0.267f, 1.0f);  // Red
	case ENomiErrorSeverity::Critical:
		return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);      // Bright red
	default:
		return FLinearColor::White;
	}
}

FString UErrorToastWidget::GetSeverityPrefix(ENomiErrorSeverity Severity) const
{
	switch (Severity)
	{
	case ENomiErrorSeverity::Info:
		return TEXT("[INFO]");
	case ENomiErrorSeverity::Warning:
		return TEXT("[WARN]");
	case ENomiErrorSeverity::Error:
		return TEXT("[ERROR]");
	case ENomiErrorSeverity::Critical:
		return TEXT("[CRITICAL]");
	default:
		return TEXT("");
	}
}

void UErrorToastWidget::OnErrorHandlerLog(ENomiErrorSeverity Severity, const FString& Message)
{
	ShowToast(Message, Severity);
}
