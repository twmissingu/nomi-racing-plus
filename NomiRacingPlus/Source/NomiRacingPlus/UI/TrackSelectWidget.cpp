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
	NIOCityCircuit.Description = TEXT("A high-speed urban circuit through the heart of the city with sweeping corners and long straights.");
	NIOCityCircuit.Mode = TEXT("All");
	AllTracks.Add(NIOCityCircuit);

	FTrackInfo ShanghaiPudong;
	ShanghaiPudong.Name = FName("ShanghaiPudong");
	ShanghaiPudong.DisplayName = TEXT("Shanghai Pudong");
	ShanghaiPudong.Description = TEXT("Race through the iconic Pudong skyline with tight chicanes and fast flowing sections.");
	ShanghaiPudong.Mode = TEXT("All");
	AllTracks.Add(ShanghaiPudong);

	FTrackInfo SpeedwayOval;
	SpeedwayOval.Name = FName("SpeedwayOval");
	SpeedwayOval.DisplayName = TEXT("Speedway Oval");
	SpeedwayOval.Description = TEXT("A classic oval speedway for pure high-speed racing. Flat out from start to finish.");
	SpeedwayOval.Mode = TEXT("GT");
	AllTracks.Add(SpeedwayOval);

	FTrackInfo MountainPass;
	MountainPass.Name = FName("MountainPass");
	MountainPass.DisplayName = TEXT("Mountain Pass");
	MountainPass.Description = TEXT("A challenging mountain road with elevation changes, tight hairpins, and stunning views.");
	MountainPass.Mode = TEXT("GT");
	AllTracks.Add(MountainPass);

	FTrackInfo DesertRally;
	DesertRally.Name = FName("DesertRally");
	DesertRally.DisplayName = TEXT("Desert Rally");
	DesertRally.Description = TEXT("An off-road desert course with sand dunes, rocky terrain, and unpredictable conditions.");
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
	UpdateTrackDisplay();
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

	CurrentTrackIndex = 0;
	UpdateTrackDisplay();
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
		const FTrackInfo& SelectedTrack = AvailableTracks[CurrentTrackIndex];
		MenuManager->SetTrack(SelectedTrack.Name.ToString());
		MenuManager->ShowRaceSettings();
	}
}

void UTrackSelectWidget::OnBackClicked()
{
	if (MenuManager)
	{
		MenuManager->ReturnToPrevious();
	}
}
