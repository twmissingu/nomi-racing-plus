// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "UI/ErrorRecoveryWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/Spacer.h"
#include "NomiRacingPlus.h"

UErrorRecoveryWidget::UErrorRecoveryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UErrorRecoveryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildLayout();

	// Start hidden
	HideRecoveryDialog();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void UErrorRecoveryWidget::ShowRecoveryDialog(const FString& ErrorMessage, bool bInCanRestoreBackup)
{
	bIsVisible = true;
	bCanRestoreBackup = bInCanRestoreBackup;

	if (MessageText)
	{
		MessageText->SetText(FText::FromString(ErrorMessage));
	}

	// Show/hide restore button based on backup availability
	if (RestoreBackupButton)
	{
		RestoreBackupButton->SetVisibility(bCanRestoreBackup
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);
	}

	SetVisibility(ESlateVisibility::Visible);

	UE_LOG(LogNomiRacing, Log, TEXT("Error recovery dialog shown: %s"), *ErrorMessage);
}

void UErrorRecoveryWidget::HideRecoveryDialog()
{
	bIsVisible = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

// ---------------------------------------------------------------------------
// Button callbacks
// ---------------------------------------------------------------------------

void UErrorRecoveryWidget::OnRestoreBackupClicked()
{
	UE_LOG(LogNomiRacing, Log, TEXT("User selected: Restore Backup"));
	HideRecoveryDialog();
	OnRecoveryActionSelected.Broadcast(ERecoveryAction::RestoreBackup);
}

void UErrorRecoveryWidget::OnResetDefaultsClicked()
{
	UE_LOG(LogNomiRacing, Log, TEXT("User selected: Reset to Defaults"));
	HideRecoveryDialog();
	OnRecoveryActionSelected.Broadcast(ERecoveryAction::ResetDefaults);
}

void UErrorRecoveryWidget::OnCancelClicked()
{
	UE_LOG(LogNomiRacing, Log, TEXT("User selected: Cancel recovery"));
	HideRecoveryDialog();
	OnRecoveryActionSelected.Broadcast(ERecoveryAction::Cancel);
}

// ---------------------------------------------------------------------------
// Programmatic UI construction
// ---------------------------------------------------------------------------

void UErrorRecoveryWidget::BuildLayout()
{
	// Root canvas — fullscreen overlay
	RootCanvas = NewObject<UCanvasPanel>(this, TEXT("RecoveryRoot"));

	// Semi-transparent dark overlay border
	OverlayBorder = NewObject<UBorder>(this, TEXT("Overlay"));
	OverlayBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f));
	OverlayBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	UCanvasPanelSlot* OverlaySlot = RootCanvas->AddChildToCanvas(OverlayBorder);
	if (OverlaySlot)
	{
		OverlaySlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		OverlaySlot->SetOffsets(FMargin(0.0f));
	}

	// Dialog content — centered vertical box
	DialogContent = NewObject<UVerticalBox>(this, TEXT("DialogContent"));
	DialogContent->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	UCanvasPanelSlot* ContentSlot = RootCanvas->AddChildToCanvas(DialogContent);
	if (ContentSlot)
	{
		ContentSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		ContentSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		ContentSlot->SetAutoSize(true);
		ContentSlot->SetPosition(FVector2D::ZeroVector);
	}

	// --- Title ---
	TitleText = NewObject<UTextBlock>(this, TEXT("Title"));
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("Save Data Problem")));
		FSlateFontInfo TitleFont = TitleText->GetFont();
		TitleFont.Size = 24;
		TitleFont.TypefaceFontName = FName("Bold");
		TitleText->SetFont(TitleFont);
		TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.722f, 0.0f))); // Amber

		UVerticalBoxSlot* TitleSlot = DialogContent->AddChildToVerticalBox(TitleText);
		if (TitleSlot)
		{
			TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 12.0f));
			TitleSlot->SetHorizontalAlignment(HAlign_Center);
		}
	}

	// --- Message ---
	MessageText = NewObject<UTextBlock>(this, TEXT("Message"));
	if (MessageText)
	{
		MessageText->SetText(FText::FromString(TEXT("Your save data could not be loaded.")));
		FSlateFontInfo MsgFont = MessageText->GetFont();
		MsgFont.Size = 16;
		MessageText->SetFont(MsgFont);
		MessageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		MessageText->SetAutoWrapText(true);

		UVerticalBoxSlot* MsgSlot = DialogContent->AddChildToVerticalBox(MessageText);
		if (MsgSlot)
		{
			MsgSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));
			MsgSlot->SetHorizontalAlignment(HAlign_Center);
		}
	}

	// --- Button container ---
	ButtonContainer = NewObject<UVerticalBox>(this, TEXT("Buttons"));
	if (ButtonContainer)
	{
		UVerticalBoxSlot* BtnContainerSlot = DialogContent->AddChildToVerticalBox(ButtonContainer);
		if (BtnContainerSlot)
		{
			BtnContainerSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
			BtnContainerSlot->SetHorizontalAlignment(HAlign_Center);
		}
	}

	// --- Restore Backup button ---
	RestoreBackupButton = CreateDialogButton(
		TEXT("Restore from Backup"),
		FLinearColor(0.0f, 0.6f, 0.3f) // Green
	);
	RestoreBackupText = NewObject<UTextBlock>(this, TEXT("RestoreText"));
	if (RestoreBackupText)
	{
		RestoreBackupText->SetText(FText::FromString(TEXT("Restore from Backup")));
		FSlateFontInfo BtnFont = RestoreBackupText->GetFont();
		BtnFont.Size = 16;
		RestoreBackupText->SetFont(BtnFont);
		RestoreBackupText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}
	if (RestoreBackupButton)
	{
		RestoreBackupButton->AddChild(RestoreBackupText);
		RestoreBackupButton->OnClicked.AddDynamic(this, &UErrorRecoveryWidget::OnRestoreBackupClicked);

		UVerticalBoxSlot* Slot = ButtonContainer->AddChildToVerticalBox(RestoreBackupButton);
		if (Slot)
		{
			Slot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
			Slot->SetHorizontalAlignment(HAlign_Center);
		}
	}

	// --- Reset Defaults button ---
	ResetDefaultsButton = CreateDialogButton(
		TEXT("Reset to Defaults"),
		FLinearColor(0.8f, 0.4f, 0.0f) // Orange
	);
	ResetDefaultsText = NewObject<UTextBlock>(this, TEXT("ResetText"));
	if (ResetDefaultsText)
	{
		ResetDefaultsText->SetText(FText::FromString(TEXT("Reset to Defaults")));
		FSlateFontInfo BtnFont = ResetDefaultsText->GetFont();
		BtnFont.Size = 16;
		ResetDefaultsText->SetFont(BtnFont);
		ResetDefaultsText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}
	if (ResetDefaultsButton)
	{
		ResetDefaultsButton->AddChild(ResetDefaultsText);
		ResetDefaultsButton->OnClicked.AddDynamic(this, &UErrorRecoveryWidget::OnResetDefaultsClicked);

		UVerticalBoxSlot* Slot = ButtonContainer->AddChildToVerticalBox(ResetDefaultsButton);
		if (Slot)
		{
			Slot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
			Slot->SetHorizontalAlignment(HAlign_Center);
		}
	}

	// --- Cancel button ---
	CancelButton = CreateDialogButton(
		TEXT("Cancel"),
		FLinearColor(0.3f, 0.3f, 0.3f) // Gray
	);
	CancelText = NewObject<UTextBlock>(this, TEXT("CancelText"));
	if (CancelText)
	{
		CancelText->SetText(FText::FromString(TEXT("Cancel")));
		FSlateFontInfo BtnFont = CancelText->GetFont();
		BtnFont.Size = 16;
		CancelText->SetFont(BtnFont);
		CancelText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}
	if (CancelButton)
	{
		CancelButton->AddChild(CancelText);
		CancelButton->OnClicked.AddDynamic(this, &UErrorRecoveryWidget::OnCancelClicked);

		UVerticalBoxSlot* Slot = ButtonContainer->AddChildToVerticalBox(CancelButton);
		if (Slot)
		{
			Slot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
			Slot->SetHorizontalAlignment(HAlign_Center);
		}
	}

	// Note: AddToViewport is handled by NomiPlayerController, not here
}

UButton* UErrorRecoveryWidget::CreateDialogButton(const FString& Label, const FLinearColor& ButtonColor)
{
	UButton* Button = NewObject<UButton>(this, *FName(Label + TEXT("Btn")).ToString());
	if (Button)
	{
		// Style the button with the provided color
		FButtonStyle Style;
		Style.Normal.TintColor = FSlateColor(ButtonColor);
		Style.Hovered.TintColor = FSlateColor(ButtonColor * 1.2f);
		Style.Pressed.TintColor = FSlateColor(ButtonColor * 0.8f);
		Button->SetStyle(Style);
	}

	return Button;
}
