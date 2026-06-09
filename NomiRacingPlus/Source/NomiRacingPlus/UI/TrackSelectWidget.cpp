// NomiRacingPlus - Track Select Widget

#include "UI/TrackSelectWidget.h"
#include "UI/MenuManager.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UTrackSelectWidget::PopulateAllTracks()
{
	AllTracks.Empty();

	FTrackInfo NIOCityCircuit;
	NIOCityCircuit.Name = FName("NIOCityCircuit");
	NIOCityCircuit.DisplayName = TEXT("NIO City Circuit");
	NIOCityCircuit.Description = TEXT("4.2 km urban circuit through the heart of the city. Features sweeping corners, long straights, and neon-lit night racing. Medium difficulty — perfect for all vehicle types. Watch for wet surfaces near the harbor.");
	NIOCityCircuit.Mode = TEXT("All");
	AllTracks.Add(NIOCityCircuit);

	FTrackInfo ShanghaiPudong;
	ShanghaiPudong.Name = FName("ShanghaiPudong");
	ShanghaiPudong.DisplayName = TEXT("Shanghai Pudong");
	ShanghaiPudong.Description = TEXT("5.1 km street circuit with iconic Pudong skyline views. Tight chicanes, tunnel sections, and elevated highway sections demand precision. Hard difficulty — rewards skilled braking and late apexes.");
	ShanghaiPudong.Mode = TEXT("All");
	AllTracks.Add(ShanghaiPudong);

	FTrackInfo SpeedwayOval;
	SpeedwayOval.Name = FName("SpeedwayOval");
	SpeedwayOval.DisplayName = TEXT("Speedway Oval");
	SpeedwayOval.Description = TEXT("3.0 km classic oval speedway with banked corners. Pure high-speed racing — flat out from start to finish. Easy difficulty but demands courage at 300+ km/h. Best for GT mode.");
	SpeedwayOval.Mode = TEXT("GT");
	AllTracks.Add(SpeedwayOval);

	FTrackInfo MountainPass;
	MountainPass.Name = FName("MountainPass");
	MountainPass.DisplayName = TEXT("Mountain Pass");
	MountainPass.Description = TEXT("8.5 km mountain road with dramatic elevation changes, tight hairpins, and stunning cliff views. Hard difficulty — narrow roads punish mistakes. One lap point-to-point challenge.");
	MountainPass.Mode = TEXT("GT");
	AllTracks.Add(MountainPass);

	FTrackInfo DesertRally;
	DesertRally.Name = FName("DesertRally");
	DesertRally.DisplayName = TEXT("Desert Rally");
	DesertRally.Description = TEXT("12.0 km off-road desert course through sand dunes and canyons. Extreme difficulty — unpredictable terrain and dust visibility. Baja mode only. Requires off-road capable vehicles.");
	DesertRally.Mode = TEXT("Baja");
	AllTracks.Add(DesertRally);
}

void UTrackSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PopulateAllTracks();

	if (PrevTrackButton)
	{
		PrevTrackButton->OnClicked.AddDynamic(this, &UTrackSelectWidget::OnPrevClicked);
	}

	if (NextTrackButton)
	{
		NextTrackButton->OnClicked.AddDynamic(this, &UTrackSelectWidget::OnNextClicked);
	}

	if (SelectButton)
	{
		SelectButton->OnClicked.AddDynamic(this, &UTrackSelectWidget::OnSelectClicked);
	}

	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &UTrackSelectWidget::OnBackClicked);
	}

	CurrentTrackIndex = 0;
	AvailableTracks = AllTracks;
	if (AvailableTracks.Num() > 0)
	{
		UpdateTrackDisplay();
		ApplyEmptyState(false);
	}
	else
	{
		ApplyEmptyState(true);
	}
}

void UTrackSelectWidget::SetMenuManager(UMenuManager* Manager)
{
	MenuManager = Manager;
}

void UTrackSelectWidget::SetModeFilter(const FString& Mode)
{
	ActiveModeFilter = Mode;
	AvailableTracks.Empty();

	if (Mode.IsEmpty())
	{
		AvailableTracks = AllTracks;
	}
	else
	{
		for (const FTrackInfo& Track : AllTracks)
		{
			if (Track.Mode == Mode || Track.Mode == TEXT("All"))
			{
				AvailableTracks.Add(Track);
			}
		}
	}

	// Handle empty state after filtering
	if (AvailableTracks.Num() == 0)
	{
		ApplyEmptyState(true);
		return;
	}

	// Restore saved selection index from MenuContext
	int32 SavedIndex = 0;
	if (MenuManager)
	{
		SavedIndex = MenuManager->GetMenuContext().TrackIndex;
	}
	CurrentTrackIndex = FMath::Clamp(SavedIndex, 0, AvailableTracks.Num() - 1);
	ApplyEmptyState(false);
	UpdateTrackDisplay();
}

void UTrackSelectWidget::ApplyEmptyState(bool bEmpty)
{
	if (bEmpty)
	{
		if (TrackNameText)
		{
			TrackNameText->SetText(FText::FromString(TEXT("No Tracks Available")));
		}
		if (TrackDescText)
		{
			TrackDescText->SetText(FText::FromString(TEXT("")));
		}

		if (PrevTrackButton) PrevTrackButton->SetIsEnabled(false);
		if (NextTrackButton) NextTrackButton->SetIsEnabled(false);
		if (SelectButton) SelectButton->SetIsEnabled(false);
	}
	else
	{
		if (PrevTrackButton) PrevTrackButton->SetIsEnabled(true);
		if (NextTrackButton) NextTrackButton->SetIsEnabled(true);
		if (SelectButton) SelectButton->SetIsEnabled(true);
	}
}

void UTrackSelectWidget::UpdateTrackDisplay()
{
	if (AvailableTracks.Num() == 0)
	{
		if (TrackNameText)
		{
			TrackNameText->SetText(FText::FromString(TEXT("No Tracks Available")));
		}
		if (TrackDescText)
		{
			TrackDescText->SetText(FText::FromString(TEXT("")));
		}
		return;
	}

	const FTrackInfo& CurrentTrack = AvailableTracks[CurrentTrackIndex];

	if (TrackNameText)
	{
		TrackNameText->SetText(FText::FromString(CurrentTrack.DisplayName));
	}

	if (TrackDescText)
	{
		TrackDescText->SetText(FText::FromString(CurrentTrack.Description));
	}
}

void UTrackSelectWidget::OnPrevClicked()
{
	if (AvailableTracks.Num() == 0)
	{
		return;
	}

	CurrentTrackIndex--;
	if (CurrentTrackIndex < 0)
	{
		CurrentTrackIndex = AvailableTracks.Num() - 1;
	}

	UpdateTrackDisplay();
}

void UTrackSelectWidget::OnNextClicked()
{
	if (AvailableTracks.Num() == 0)
	{
		return;
	}

	CurrentTrackIndex++;
	if (CurrentTrackIndex >= AvailableTracks.Num())
	{
		CurrentTrackIndex = 0;
	}

	UpdateTrackDisplay();
}

void UTrackSelectWidget::OnSelectClicked()
{
	if (AvailableTracks.Num() == 0)
	{
		return;
	}

	if (MenuManager)
	{
		// Save current selection to MenuContext for state preservation
		FMenuContext Ctx = MenuManager->GetMenuContext();
		Ctx.TrackIndex = CurrentTrackIndex;
		MenuManager->SetMenuContext(Ctx);

		const FTrackInfo& SelectedTrack = AvailableTracks[CurrentTrackIndex];
		MenuManager->SetTrack(SelectedTrack.Name.ToString());
		MenuManager->ShowRaceSettings();
	}
}

void UTrackSelectWidget::OnBackClicked()
{
	if (MenuManager)
	{
		// Save current selection to MenuContext before navigating back
		FMenuContext Ctx = MenuManager->GetMenuContext();
		Ctx.TrackIndex = CurrentTrackIndex;
		MenuManager->SetMenuContext(Ctx);

		MenuManager->ReturnToPrevious();
	}
}
