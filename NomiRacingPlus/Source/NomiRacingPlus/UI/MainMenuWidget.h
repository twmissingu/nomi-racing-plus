// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UMenuManager;

UCLASS(Abstract, Blueprintable)
class NOMIRACINGPLUS_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GTModeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NIOModeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BajaModeButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY()
	TObjectPtr<UMenuManager> MenuManager;

	UFUNCTION()
	void SetMenuManager(UMenuManager* Manager);

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnGTModeClicked();

	UFUNCTION()
	void OnNIOModeClicked();

	UFUNCTION()
	void OnBajaModeClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnExitClicked();
};
