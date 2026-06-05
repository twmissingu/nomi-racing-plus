// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "Race/RaceManager.h"
#include "Vehicles/VehicleStateManager.h"
#include "AI/AICarController.h"
#include "Kismet/GameplayStatics.h"
#include "NomiRacingPlus.h"

ARaceManager::ARaceManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARaceManager::BeginPlay()
{
	Super::BeginPlay();
}

void ARaceManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (RaceState)
	{
	case ERaceState::Countdown:
		UpdateCountdown(DeltaTime);
		break;

	case ERaceState::Racing:
		UpdateRaceTimer(DeltaTime);
		UpdatePositions();
		break;

	default:
		break;
	}
}

void ARaceManager::StartRace(const FRaceConfig& Config)
{
	if (RaceState != ERaceState::Idle && RaceState != ERaceState::PostRace)
	{
		UE_LOG(LogNomiRace, Warning, TEXT("Cannot start race: current state is %d"), (int32)RaceState);
		return;
	}

	RaceConfig = Config;
	RaceState = ERaceState::Countdown;
	CountdownTimer = Config.CountdownDuration;
	RaceTimer = 0.0f;

	// Detect Baja mode
	if (RaceConfig.RaceMode == TEXT("Baja"))
	{
		bIsBajaMode = true;
		RaceConfig.bIsPointToPoint = true;
	}
	else
	{
		bIsBajaMode = false;
	}

	// Reset all racers
	for (FRacerData& Racer : Racers)
	{
		Racer.CurrentLap = 0;
		Racer.CurrentCheckpoint = 0;
		Racer.TotalCheckpointsPassed = 0;
		Racer.Position = 0;
		Racer.LapTimes.Empty();
		Racer.BestLapTime = -1.0f;
		Racer.TotalRaceTime = 0.0f;
		Racer.bFinished = false;
		Racer.TrackProgress = 0.0f;
	}

	UE_LOG(LogNomiRace, Log, TEXT("Race starting: %s, %d laps, %d racers, Baja=%d"),
		*Config.TrackName, Config.NumLaps, Racers.Num(), bIsBajaMode);

	BroadcastRaceEvent(ERaceEvent::CountdownStart, FRacerData());
}

void ARaceManager::PauseRace()
{
	if (RaceState == ERaceState::Racing)
	{
		RaceState = ERaceState::Paused;
		UE_LOG(LogNomiRace, Log, TEXT("Race paused"));
	}
}

void ARaceManager::ResumeRace()
{
	if (RaceState == ERaceState::Paused)
	{
		RaceState = ERaceState::Racing;
		UE_LOG(LogNomiRace, Log, TEXT("Race resumed"));
	}
}

void ARaceManager::EndRace()
{
	RaceState = ERaceState::PostRace;
	UE_LOG(LogNomiRace, Log, TEXT("Race finished!"));

	// Stop all AI controllers
	for (const FRacerData& Racer : Racers)
	{
		if (!Racer.bIsPlayer && Racer.VehiclePawn)
		{
			if (AAICarController* AICtrl = Cast<AAICarController>(Racer.VehiclePawn->GetController()))
			{
				AICtrl->StopRacing();
			}
		}
	}

	// Find player and broadcast finish event
	for (const FRacerData& Racer : Racers)
	{
		if (Racer.bIsPlayer)
		{
			BroadcastRaceEvent(ERaceEvent::RaceFinish, Racer);
			break;
		}
	}
}

bool ARaceManager::AreAllRacersFinished() const
{
	for (const FRacerData& Racer : Racers)
	{
		if (!Racer.bFinished)
		{
			return false;
		}
	}
	return Racers.Num() > 0;
}

void ARaceManager::ResetRace()
{
	RaceState = ERaceState::Idle;
	Racers.Empty();
	RaceTimer = 0.0f;
	CountdownTimer = 0.0f;
}

void ARaceManager::RegisterRacer(APawn* VehiclePawn, const FString& DisplayName, bool bIsPlayer)
{
	if (!VehiclePawn)
	{
		return;
	}

	// Check if already registered
	if (FindRacerIndex(VehiclePawn) != INDEX_NONE)
	{
		UE_LOG(LogNomiRace, Warning, TEXT("Racer already registered: %s"), *DisplayName);
		return;
	}

	FRacerData NewRacer;
	NewRacer.RacerID = Racers.Num();
	NewRacer.DisplayName = DisplayName;
	NewRacer.bIsPlayer = bIsPlayer;
	NewRacer.VehiclePawn = VehiclePawn;

	Racers.Add(NewRacer);

	UE_LOG(LogNomiRace, Log, TEXT("Racer registered: %s (ID: %d, Player: %d)"),
		*DisplayName, NewRacer.RacerID, bIsPlayer);
}

void ARaceManager::UnregisterRacer(APawn* VehiclePawn)
{
	int32 Index = FindRacerIndex(VehiclePawn);
	if (Index != INDEX_NONE)
	{
		// Mark as inactive instead of removing to preserve array indices
		// and RacerID consistency for position tracking
		Racers[Index].bFinished = true;
		Racers[Index].VehiclePawn = nullptr;
		UE_LOG(LogNomiRace, Log, TEXT("Racer unregistered: %s"), *Racers[Index].DisplayName);
	}
}

bool ARaceManager::GetRacerData(APawn* VehiclePawn, FRacerData& OutData) const
{
	int32 Index = FindRacerIndex(VehiclePawn);
	if (Index != INDEX_NONE)
	{
		OutData = Racers[Index];
		return true;
	}
	return false;
}

void ARaceManager::RacerPassCheckpoint(APawn* VehiclePawn, int32 CheckpointIndex)
{
	int32 Index = FindRacerIndex(VehiclePawn);
	if (Index == INDEX_NONE)
	{
		return;
	}

	FRacerData& Racer = Racers[Index];

	if (Racer.bFinished)
	{
		return;
	}

	// Validate checkpoint sequence
	if (CheckpointIndex != Racer.CurrentCheckpoint)
	{
		UE_LOG(LogNomiRace, Warning, TEXT("Racer %s: checkpoint %d out of sequence (expected %d)"),
			*Racer.DisplayName, CheckpointIndex, Racer.CurrentCheckpoint);
		return;
	}

	// Update checkpoint
	Racer.CurrentCheckpoint = (CheckpointIndex + 1) % CheckpointsPerLap;
	Racer.TotalCheckpointsPassed++;

	// Baja mode: finish when all checkpoints passed (point-to-point)
	if (bIsBajaMode && Racer.CurrentCheckpoint == 0 && Racer.TotalCheckpointsPassed >= CheckpointsPerLap)
	{
		Racer.bFinished = true;
		Racer.TotalRaceTime = RaceTimer;
		Racer.CurrentLap = 1; // Mark as completed

		BroadcastRaceEvent(ERaceEvent::RaceFinish, Racer);

		if (Racer.bIsPlayer)
		{
			EndRace();
		}
		else if (AreAllRacersFinished())
		{
			EndRace();
		}
	}
	// Standard mode: check for lap completion
	else if (Racer.CurrentCheckpoint == 0)
	{
		Racer.CurrentLap++;

		// Calculate lap time
		float PreviousLapsTotal = 0.0f;
		for (float T : Racer.LapTimes) { PreviousLapsTotal += T; }
		float LapTime = RaceTimer - PreviousLapsTotal;
		Racer.LapTimes.Add(LapTime);

		// Update best lap
		if (Racer.BestLapTime < 0.0f || LapTime < Racer.BestLapTime)
		{
			Racer.BestLapTime = LapTime;
			BroadcastRaceEvent(ERaceEvent::FastestLap, Racer);
		}

		BroadcastRaceEvent(ERaceEvent::LapComplete, Racer);

		// Check if racer finished
		if (Racer.CurrentLap >= RaceConfig.NumLaps)
		{
			Racer.bFinished = true;
			Racer.TotalRaceTime = RaceTimer;

			// End race when the player finishes
			if (Racer.bIsPlayer)
			{
				EndRace();
			}
			// Also end race if ALL racers have finished
			else if (AreAllRacersFinished())
			{
				EndRace();
			}
		}
	}

	BroadcastRaceEvent(ERaceEvent::CheckpointPassed, Racer);

	// Update progress
	Racer.TrackProgress = CalculateRacerProgress(Racer);
}

void ARaceManager::RacerCrossFinishLine(APawn* VehiclePawn)
{
	int32 Index = FindRacerIndex(VehiclePawn);
	if (Index == INDEX_NONE)
	{
		return;
	}

	FRacerData& Racer = Racers[Index];

	// Process finish line crossing.
	//
	// Race flow:
	//   Lap=0, CP=0  → first crossing:   RacerPassCheckpoint(0)  → CP=1         (race start)
	//   Lap=N, CP=0  → subsequent start: RacerPassCheckpoint(9)  → CP=0, Lap++  (new lap)
	//   Lap=N, CP=9  → after CP8:        RacerPassCheckpoint(9)  → CP=0, Lap++  (lap completion)
	//
	// Cases 1 & 2 occur when the finish line trigger overlaps the start line
	// and the racer's CP has wrapped to 0.  Case 3 occurs when the finish
	// line is the *last* checkpoint (index CheckpointsPerLap-1) and the
	// racer has just passed checkpoint 8.
	if (Racer.CurrentCheckpoint == 0)
	{
		if (Racer.CurrentLap == 0)
		{
			RacerPassCheckpoint(VehiclePawn, 0);
		}
		else
		{
			RacerPassCheckpoint(VehiclePawn, CheckpointsPerLap - 1);
		}
	}
	else if (Racer.CurrentCheckpoint == CheckpointsPerLap - 1)
	{
		// Crossing the finish line after passing all intermediate checkpoints.
		// This calls RacerPassCheckpoint(lastIndex) which wraps CP to 0 and
		// increments the lap via the existing lap-completion logic.
		RacerPassCheckpoint(VehiclePawn, CheckpointsPerLap - 1);
	}
}

int32 ARaceManager::GetPlayerPosition() const
{
	for (const FRacerData& Racer : Racers)
	{
		if (Racer.bIsPlayer)
		{
			return Racer.Position;
		}
	}
	return 0;
}

int32 ARaceManager::GetPlayerCurrentLap() const
{
	for (const FRacerData& Racer : Racers)
	{
		if (Racer.bIsPlayer)
		{
			return Racer.CurrentLap;
		}
	}
	return 0;
}

bool ARaceManager::IsRaceFinished() const
{
	return RaceState == ERaceState::Finished;
}

void ARaceManager::UpdateCountdown(float DeltaTime)
{
	CountdownTimer -= DeltaTime;

	if (CountdownTimer <= 0.0f)
	{
		CountdownTimer = 0.0f;
		RaceState = ERaceState::Racing;

		UE_LOG(LogNomiRace, Log, TEXT("Race started!"));
		BroadcastRaceEvent(ERaceEvent::RaceStart, FRacerData());
	}
}

void ARaceManager::UpdateRaceTimer(float DeltaTime)
{
	RaceTimer += DeltaTime;

	// Update total race time for all active racers
	for (FRacerData& Racer : Racers)
	{
		if (!Racer.bFinished)
		{
			Racer.TotalRaceTime = RaceTimer;
		}
	}
}

void ARaceManager::UpdatePositions()
{
	// Sort racers by progress
	TArray<int32> SortedIndices;
	for (int32 i = 0; i < Racers.Num(); i++)
	{
		SortedIndices.Add(i);
	}

	SortedIndices.Sort([this](int32 A, int32 B) {
		const FRacerData& RacerA = Racers[A];
		const FRacerData& RacerB = Racers[B];

		// Finished racers are ahead
		if (RacerA.bFinished != RacerB.bFinished)
		{
			return RacerA.bFinished;
		}

		// Baja mode: sort by progress percent (distance-based)
		if (bIsBajaMode)
		{
			return GetProgressPercent(A) > GetProgressPercent(B);
		}

		// Compare by lap
		if (RacerA.CurrentLap != RacerB.CurrentLap)
		{
			return RacerA.CurrentLap > RacerB.CurrentLap;
		}

		// Compare by checkpoint
		if (RacerA.TotalCheckpointsPassed != RacerB.TotalCheckpointsPassed)
		{
			return RacerA.TotalCheckpointsPassed > RacerB.TotalCheckpointsPassed;
		}

		// Compare by track progress
		return RacerA.TrackProgress > RacerB.TrackProgress;
	});

	// Update positions
	for (int32 i = 0; i < SortedIndices.Num(); i++)
	{
		int32 RacerIndex = SortedIndices[i];
		int32 NewPosition = i + 1;

		// Check for position changes (overtake events)
		if (Racers[RacerIndex].Position != 0 && Racers[RacerIndex].Position != NewPosition)
		{
			if (Racers[RacerIndex].bIsPlayer)
			{
				if (NewPosition < Racers[RacerIndex].Position)
				{
					BroadcastRaceEvent(ERaceEvent::Overtake, Racers[RacerIndex]);
				}
				else
				{
					BroadcastRaceEvent(ERaceEvent::Overtaken, Racers[RacerIndex]);
				}
			}
		}

		// Check for first/last place events (only when position changes)
		if (Racers[RacerIndex].bIsPlayer && Racers[RacerIndex].Position != NewPosition)
		{
			if (NewPosition == 1 && Racers[RacerIndex].Position != 1)
			{
				BroadcastRaceEvent(ERaceEvent::FirstPlace, Racers[RacerIndex]);
			}
			else if (NewPosition == Racers.Num() && Racers[RacerIndex].Position != Racers.Num())
			{
				BroadcastRaceEvent(ERaceEvent::LastPlace, Racers[RacerIndex]);
			}
		}

		Racers[RacerIndex].Position = NewPosition;
	}
}

float ARaceManager::GetDistanceToFinish(int32 RacerIndex) const
{
	if (!Racers.IsValidIndex(RacerIndex))
	{
		return TotalTrackDistance;
	}

	const FRacerData& Racer = Racers[RacerIndex];

	// If racer is finished, no distance remaining
	if (Racer.bFinished)
	{
		return 0.0f;
	}

	// If we have checkpoint positions, use actual distances
	if (CheckpointPositions.Num() >= 2)
	{
		const int32 NumCheckpoints = CheckpointPositions.Num();
		const int32 NextCheckpoint = Racer.CurrentCheckpoint % NumCheckpoints;

		float RemainingDistance = 0.0f;

		// Distance from racer's current position to the next checkpoint
		if (Racer.VehiclePawn)
		{
			const FVector RacerLocation = Racer.VehiclePawn->GetActorLocation();
			RemainingDistance += FVector::Dist(RacerLocation, CheckpointPositions[NextCheckpoint]) / 100.0f; // cm to meters
		}

		// Sum distances between remaining checkpoints (NextCheckpoint -> ... -> 0 = finish)
		int32 Current = NextCheckpoint;
		int32 SafetyCounter = 0;
		while (Current != 0 && SafetyCounter < NumCheckpoints)
		{
			int32 Next = (Current + 1) % NumCheckpoints;
			RemainingDistance += FVector::Dist(CheckpointPositions[Current], CheckpointPositions[Next]) / 100.0f;
			Current = Next;
			SafetyCounter++;
		}

		return FMath::Max(RemainingDistance, 0.0f);
	}

	// Fallback: estimate based on checkpoint count
	const int32 SafeCheckpoints = FMath::Max(CheckpointsPerLap, 1);
	const float DistancePerCheckpoint = TotalTrackDistance / SafeCheckpoints;
	const int32 RemainingCheckpoints = SafeCheckpoints - Racer.CurrentCheckpoint;
	return RemainingCheckpoints * DistancePerCheckpoint;
}

float ARaceManager::GetProgressPercent(int32 RacerIndex) const
{
	float DistanceRemaining = GetDistanceToFinish(RacerIndex);
	float Progress = 1.0f - (DistanceRemaining / FMath::Max(TotalTrackDistance, 1.0f));
	return FMath::Clamp(Progress * 100.0f, 0.0f, 100.0f);
}

float ARaceManager::CalculateRacerProgress(const FRacerData& Racer) const
{
	// Calculate progress as 0-1 value
	const int32 SafeNumLaps = FMath::Max(RaceConfig.NumLaps, 1);
	const int32 SafeCheckpointsPerLap = FMath::Max(CheckpointsPerLap, 1);

	float LapsProgress = (float)Racer.CurrentLap / (float)SafeNumLaps;
	float CheckpointProgress = (float)Racer.CurrentCheckpoint / (float)SafeCheckpointsPerLap / (float)SafeNumLaps;

	return FMath::Clamp(LapsProgress + CheckpointProgress, 0.0f, 1.0f);
}

int32 ARaceManager::FindRacerIndex(APawn* VehiclePawn) const
{
	for (int32 i = 0; i < Racers.Num(); i++)
	{
		if (Racers[i].VehiclePawn == VehiclePawn)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void ARaceManager::BroadcastRaceEvent(ERaceEvent Event, const FRacerData& RacerData)
{
	OnRaceEvent.Broadcast(Event, RacerData);
}
