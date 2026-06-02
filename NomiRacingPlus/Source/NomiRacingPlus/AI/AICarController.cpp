// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "AI/AICarController.h"
#include "AI/AIBehaviorTree.h"
#include "AI/AISensorSystem.h"
#include "AI/AIOvertakeEvaluator.h"
#include "AI/AIDefensiveEvaluator.h"
#include "AI/AISlipstreamSystem.h"
#include "AI/AIRubberBandScaler.h"
#include "Vehicles/VehicleStateManager.h"
#include "Vehicles/NIOVehicleMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NomiRacingPlus.h"

AAICarController::AAICarController()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize difficulty settings
	FAIDifficultySettings EasySettings;
	EasySettings.SpeedMultiplier = 0.85f;
	EasySettings.ThrottleSmoothness = 0.9f;
	EasySettings.BrakeSmoothness = 0.8f;
	EasySettings.SteeringSmoothness = 0.7f;
	EasySettings.BrakingDistanceMultiplier = 1.3f;
	EasySettings.OvertakeAggressiveness = 0.3f;
	EasySettings.CorneringSpeedFactor = 0.7f;
	EasySettings.PathDeviation = 0.15f;
	EasySettings.ReactionTime = 0.5f;
	EasySettings.RubberBandStrength = 0.4f;
	DifficultySettings.Add(EAIDifficulty::Easy, EasySettings);

	FAIDifficultySettings NormalSettings;
	NormalSettings.SpeedMultiplier = 0.95f;
	NormalSettings.ThrottleSmoothness = 0.8f;
	NormalSettings.BrakeSmoothness = 0.7f;
	NormalSettings.SteeringSmoothness = 0.6f;
	NormalSettings.BrakingDistanceMultiplier = 1.1f;
	NormalSettings.OvertakeAggressiveness = 0.5f;
	NormalSettings.CorneringSpeedFactor = 0.8f;
	NormalSettings.PathDeviation = 0.1f;
	NormalSettings.ReactionTime = 0.3f;
	NormalSettings.RubberBandStrength = 0.3f;
	DifficultySettings.Add(EAIDifficulty::Normal, NormalSettings);

	FAIDifficultySettings HardSettings;
	HardSettings.SpeedMultiplier = 1.0f;
	HardSettings.ThrottleSmoothness = 0.7f;
	HardSettings.BrakeSmoothness = 0.6f;
	HardSettings.SteeringSmoothness = 0.5f;
	HardSettings.BrakingDistanceMultiplier = 0.9f;
	HardSettings.OvertakeAggressiveness = 0.7f;
	HardSettings.CorneringSpeedFactor = 0.9f;
	HardSettings.PathDeviation = 0.05f;
	HardSettings.ReactionTime = 0.15f;
	HardSettings.RubberBandStrength = 0.2f;
	DifficultySettings.Add(EAIDifficulty::Hard, HardSettings);

	FAIDifficultySettings ExpertSettings;
	ExpertSettings.SpeedMultiplier = 1.0f;
	ExpertSettings.ThrottleSmoothness = 0.6f;
	ExpertSettings.BrakeSmoothness = 0.5f;
	ExpertSettings.SteeringSmoothness = 0.4f;
	ExpertSettings.BrakingDistanceMultiplier = 0.85f;
	ExpertSettings.OvertakeAggressiveness = 0.85f;
	ExpertSettings.CorneringSpeedFactor = 0.95f;
	ExpertSettings.PathDeviation = 0.02f;
	ExpertSettings.ReactionTime = 0.08f;
	ExpertSettings.RubberBandStrength = 0.0f;
	DifficultySettings.Add(EAIDifficulty::Expert, ExpertSettings);
}

void AAICarController::BeginPlay()
{
	Super::BeginPlay();

	ControlledVehicle = GetPawn();
	CachedWorld = GetWorld();

	// Cache component references to avoid FindComponentByClass every frame
	if (ControlledVehicle)
	{
		CachedStateManager = ControlledVehicle->FindComponentByClass<UVehicleStateManager>();
		CachedMovementComponent = ControlledVehicle->FindComponentByClass<UNIOVehicleMovementComponent>();

		// Create behavior tree component
		BehaviorTree = NewObject<UAIBehaviorTree>(ControlledVehicle, TEXT("AIBehaviorTree"));
		BehaviorTree->RegisterComponent();

		// Configure behavior tree based on difficulty
		BehaviorTree->SetDifficulty(FMath::Clamp(GetCurrentSettings().SpeedMultiplier, 0.0f, 1.0f));

		// Configure overtake evaluator personality based on difficulty
		if (UAIOvertakeEvaluator* OvertakeEval = BehaviorTree->GetOvertakeEvaluator())
		{
			FOvertakePersonality OvertakePersonality;
			OvertakePersonality.FrequencyMultiplier = GetCurrentSettings().OvertakeAggressiveness * 2.0f;
			OvertakePersonality.RiskTolerance = GetCurrentSettings().OvertakeAggressiveness;
			OvertakePersonality.Patience = 1.0f - GetCurrentSettings().OvertakeAggressiveness;
			OvertakeEval->SetPersonality(OvertakePersonality);
		}

		// Configure defensive evaluator personality based on difficulty
		if (UAIDefensiveEvaluator* DefendEval = BehaviorTree->GetDefensiveEvaluator())
		{
			FAIDefensivePersonality DefendPersonality;
			DefendPersonality.Aggressiveness = GetCurrentSettings().OvertakeAggressiveness;
			DefendPersonality.Awareness = 1.0f - GetCurrentSettings().ReactionTime;
			DefendEval->SetPersonality(DefendPersonality);
		}

		// Configure rubber band scaler based on difficulty settings
		if (UAIRubberBandScaler* RBScaler = BehaviorTree->GetRubberBandScaler())
		{
			FAIRubberBandConfig RBConfig;
			RBConfig.CatchUpSpeedBoost = GetCurrentSettings().RubberBandStrength * 0.5f;
			RBConfig.SlowDownWhenAhead = GetCurrentSettings().RubberBandStrength * 0.2f;
			RBConfig.MaxBoost = GetCurrentSettings().RubberBandStrength * 0.8f;
			RBConfig.MaxReduction = GetCurrentSettings().RubberBandStrength * 0.4f;
			RBScaler->SetConfig(RBConfig);
		}

		UE_LOG(LogNomiAI, Log, TEXT("AI Behavior Tree initialized for %s"), *ControlledVehicle->GetName());
	}
}

void AAICarController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsRacing && AIState != EAIState::Idle && AIState != EAIState::Finished)
	{
		UpdateAIDecision(DeltaTime);
	}
}

void AAICarController::UpdateRubberBandContext()
{
	if (!BehaviorTree || !ControlledVehicle)
	{
		return;
	}

	UAIRubberBandScaler* RBScaler = BehaviorTree->GetRubberBandScaler();
	if (!RBScaler)
	{
		return;
	}

	// Get race manager to determine positions
	// For now, use a simplified approach with the target speed multiplier
	// The rubber band scaler will be updated with proper race context from the race manager

	// Calculate distance to player (would come from race manager)
	float DistanceToPlayer = 0.0f;
	int32 AIPosition = 0;
	int32 PlayerPosition = 0;
	float RaceProgress = 0.0f;

	// Update rubber band scaler
	RBScaler->UpdateState(DistanceToPlayer, AIPosition, PlayerPosition, RaceProgress);
}

void AAICarController::SetDifficulty(EAIDifficulty InDifficulty)
{
	Difficulty = InDifficulty;
	UE_LOG(LogNomiAI, Log, TEXT("AI difficulty set to: %d"), (int32)InDifficulty);
}

void AAICarController::SetWaypoints(const TArray<FAIWaypoint>& InWaypoints)
{
	Waypoints = InWaypoints;
	CurrentWaypointIndex = 0;

	UE_LOG(LogNomiAI, Log, TEXT("AI waypoints set: %d waypoints"), Waypoints.Num());
}

void AAICarController::StartRacing()
{
	if (Waypoints.Num() == 0)
	{
		UE_LOG(LogNomiAI, Warning, TEXT("Cannot start racing: no waypoints set"));
		return;
	}

	bIsRacing = true;
	AIState = EAIState::Racing;
	CurrentWaypointIndex = 0;

	UE_LOG(LogNomiAI, Log, TEXT("AI started racing"));
}

void AAICarController::StopRacing()
{
	bIsRacing = false;
	AIState = EAIState::Idle;

	UE_LOG(LogNomiAI, Log, TEXT("AI stopped racing"));
}

void AAICarController::SetTargetSpeedMultiplier(float Multiplier)
{
	TargetSpeedMultiplier = FMath::Clamp(Multiplier, 0.5f, 1.5f);
}

void AAICarController::UpdateAIDecision(float DeltaTime)
{
	if (!ControlledVehicle || Waypoints.Num() == 0)
	{
		return;
	}

	// Update reaction timer
	if (ReactionTimer > 0.0f)
	{
		ReactionTimer -= DeltaTime;
		return;
	}

	// Get current waypoint
	const FAIWaypoint& CurrentWaypoint = Waypoints[CurrentWaypointIndex];

	// Calculate distance to waypoint
	FVector VehicleLocation = ControlledVehicle->GetActorLocation();
	float DistanceToWaypoint = FVector::Distance(VehicleLocation, CurrentWaypoint.Location);

	// Check if we should move to next waypoint
	if (DistanceToWaypoint < 500.0f) // 5 meters threshold
	{
		CurrentWaypointIndex = (CurrentWaypointIndex + 1) % Waypoints.Num();

		// Reset reaction timer
		const FAIDifficultySettings& Settings = GetCurrentSettings();
		ReactionTimer = Settings.ReactionTime;
	}

	// Calculate steering
	float Steering = CalculateSteering(CurrentWaypoint.Location);

	// Calculate throttle and brake
	float Throttle = 0.0f;
	float Brake = 0.0f;
	CalculateThrottleBrake(Throttle, Brake, DeltaTime);

	// Check for obstacles
	FVector AvoidanceDirection;
	if (CheckForObstacles(AvoidanceDirection))
	{
		// Adjust steering for avoidance
		float AvoidanceSteering = FVector::DotProduct(ControlledVehicle->GetActorRightVector(), AvoidanceDirection);
		Steering += AvoidanceSteering * 0.5f;
	}

	// Update rubber band context
	UpdateRubberBandContext();

	// If behavior tree is available, use it for advanced decisions
	if (BehaviorTree)
	{
		// Build decision factors for behavior tree
		FAIDecisionFactors Factors;
		Factors.CurrentSpeed = 0.0f;
		Factors.RecommendedSpeed = CurrentWaypoint.RecommendedSpeed;
		Factors.bIsInCorner = CurrentWaypoint.bIsCorner;
		Factors.CornerSharpness = CurrentWaypoint.CornerSharpness;
		Factors.bIsOnStraight = !CurrentWaypoint.bIsCorner;

		// Get speed from cached state manager
		if (CachedStateManager)
		{
			Factors.CurrentSpeed = CachedStateManager->GetVehicleState().SpeedKmh;
		}

		// Update behavior tree
		BehaviorTree->UpdateDecisions(Factors);

		// Apply behavior tree inputs
		float BTThrottle = BehaviorTree->GetThrottleInput();
		float BTBrake = BehaviorTree->GetBrakeInput();
		float BTSteering = BehaviorTree->GetSteeringInput();

		// Blend behavior tree decisions with waypoint following
		const FAIDifficultySettings& Settings = GetCurrentSettings();
		float BTWeight = Settings.OvertakeAggressiveness;

		Throttle = FMath::Lerp(Throttle, BTThrottle, BTWeight);
		Brake = FMath::Lerp(Brake, BTBrake, BTWeight);
		Steering = FMath::Lerp(Steering, BTSteering, BTWeight * 0.5f);

		// Apply rubber band speed multiplier
		if (UAIRubberBandScaler* RBScaler = BehaviorTree->GetRubberBandScaler())
		{
			float RBMultiplier = RBScaler->GetSpeedMultiplier();
			TargetSpeedMultiplier *= RBMultiplier;
		}
	}
	else
	{
		// Fallback to legacy overtake logic
		UpdateOvertakeLogic(DeltaTime);
	}

	// Apply inputs
	ApplyAIInputs(Throttle, Brake, Steering, DeltaTime);
}

float AAICarController::CalculateSteering(const FVector& TargetLocation)
{
	if (!ControlledVehicle)
	{
		return 0.0f;
	}

	FVector VehicleLocation = ControlledVehicle->GetActorLocation();
	FVector VehicleForward = ControlledVehicle->GetActorForwardVector();

	// Direction to target
	FVector DirectionToTarget = (TargetLocation - VehicleLocation).GetSafeNormal();

	// Calculate angle difference
	float DotProduct = FVector::DotProduct(VehicleForward, DirectionToTarget);
	float CrossProductZ = FVector::CrossProduct(VehicleForward, DirectionToTarget).Z;

	// Convert to steering value (-1 to 1)
	float AngleDegrees = FMath::Atan2(CrossProductZ, DotProduct) * (180.0f / PI);
	float Steering = FMath::Clamp(AngleDegrees / 45.0f, -1.0f, 1.0f);

	// Apply path deviation
	const FAIDifficultySettings& Settings = GetCurrentSettings();
	Steering += FMath::RandRange(-Settings.PathDeviation, Settings.PathDeviation);

	return Steering;
}

void AAICarController::CalculateThrottleBrake(float& OutThrottle, float& OutBrake, float DeltaTime)
{
	if (!ControlledVehicle || Waypoints.Num() == 0)
	{
		return;
	}

	const FAIDifficultySettings& Settings = GetCurrentSettings();
	const FAIWaypoint& CurrentWaypoint = Waypoints[CurrentWaypointIndex];

	// Get current speed from cached state manager
	float CurrentSpeed = 0.0f;
	if (CachedStateManager)
	{
		CurrentSpeed = CachedStateManager->GetVehicleState().SpeedKmh;
	}

	// Calculate target speed
	float TargetSpeed = CurrentWaypoint.RecommendedSpeed * Settings.SpeedMultiplier * TargetSpeedMultiplier;

	// Adjust for corners
	if (CurrentWaypoint.bIsCorner)
	{
		TargetSpeed *= Settings.CorneringSpeedFactor * (1.0f - CurrentWaypoint.CornerSharpness * 0.5f);
	}

	// Rubber band effect
	if (Settings.RubberBandStrength > 0.0f)
	{
		// This would need race position data to implement properly
		// For now, just use base target speed
	}

	// Calculate throttle/brake based on speed difference
	float SpeedDifference = TargetSpeed - CurrentSpeed;

	if (SpeedDifference > 10.0f)
	{
		// Need to accelerate
		OutThrottle = FMath::Clamp(SpeedDifference / 100.0f, 0.3f, 1.0f);
		OutBrake = 0.0f;
	}
	else if (SpeedDifference < -10.0f)
	{
		// Need to brake
		OutThrottle = 0.0f;
		OutBrake = FMath::Clamp(-SpeedDifference / 100.0f, 0.3f, 1.0f);
	}
	else
	{
		// Maintain speed
		OutThrottle = 0.3f;
		OutBrake = 0.0f;
	}

	// Apply braking distance multiplier for corners
	if (CurrentWaypoint.bIsCorner)
	{
		float DistanceToCorner = FVector::Distance(ControlledVehicle->GetActorLocation(), CurrentWaypoint.Location);
		float BrakingDistance = CurrentSpeed * Settings.BrakingDistanceMultiplier;

		if (DistanceToCorner < BrakingDistance)
		{
			OutBrake = FMath::Clamp((BrakingDistance - DistanceToCorner) / BrakingDistance, 0.3f, 1.0f);
			OutThrottle = 0.0f;
		}
	}
}

bool AAICarController::CheckForObstacles(FVector& OutAvoidanceDirection)
{
	if (!ControlledVehicle || !CachedWorld)
	{
		return false;
	}

	FVector Start = ControlledVehicle->GetActorLocation();
	FVector Forward = ControlledVehicle->GetActorForwardVector();
	FVector Right = ControlledVehicle->GetActorRightVector();

	// Cast rays forward and to sides
	float RayLength = 1000.0f; // 10 meters
	FHitResult HitResult;

	// Forward ray
	if (CachedWorld->LineTraceSingleByChannel(HitResult, Start, Start + Forward * RayLength, ECC_Visibility))
	{
		OutAvoidanceDirection = -Forward;
		return true;
	}

	// Right ray
	if (CachedWorld->LineTraceSingleByChannel(HitResult, Start, Start + Right * RayLength * 0.5f, ECC_Visibility))
	{
		OutAvoidanceDirection = -Right;
		return true;
	}

	// Left ray
	if (CachedWorld->LineTraceSingleByChannel(HitResult, Start, Start - Right * RayLength * 0.5f, ECC_Visibility))
	{
		OutAvoidanceDirection = Right;
		return true;
	}

	return false;
}

void AAICarController::UpdateOvertakeLogic(float DeltaTime)
{
	if (!ControlledVehicle || !CachedWorld)
	{
		return;
	}

	const FAIDifficultySettings& Settings = GetCurrentSettings();

	// Simple overtake logic: if there's a vehicle ahead and to the side, try to overtake
	FVector Start = ControlledVehicle->GetActorLocation();
	FVector Forward = ControlledVehicle->GetActorForwardVector();
	FVector Right = ControlledVehicle->GetActorRightVector();

	// Check for vehicles ahead
	FHitResult HitResult;
	float CheckDistance = 2000.0f; // 20 meters

	if (CachedWorld->LineTraceSingleByChannel(HitResult, Start, Start + Forward * CheckDistance, ECC_Pawn))
	{
		if (APawn* OtherVehicle = Cast<APawn>(HitResult.GetActor()))
		{
			// Check if we should overtake
			if (OvertakeTimer <= 0.0f && FMath::FRand() < Settings.OvertakeAggressiveness)
			{
				// Determine overtake direction
				FVector ToOther = OtherVehicle->GetActorLocation() - Start;
				float Side = FVector::DotProduct(Right, ToOther);

				if (Side > 0)
				{
					// Other vehicle is to the right, go left
					AIState = EAIState::Overtaking;
					OvertakeTimer = 2.0f; // Overtake for 2 seconds
				}
				else
				{
					// Other vehicle is to the left, go right
					AIState = EAIState::Overtaking;
					OvertakeTimer = 2.0f;
				}
			}
		}
	}

	// Update overtake timer
	if (OvertakeTimer > 0.0f)
	{
		OvertakeTimer -= DeltaTime;
		if (OvertakeTimer <= 0.0f)
		{
			AIState = EAIState::Racing;
		}
	}
}

void AAICarController::ApplyAIInputs(float Throttle, float Brake, float Steering, float DeltaTime)
{
	const FAIDifficultySettings& Settings = GetCurrentSettings();

	// Smooth inputs
	SmoothThrottle = FMath::FInterpTo(SmoothThrottle, Throttle, DeltaTime, Settings.ThrottleSmoothness * 10.0f);
	SmoothBrake = FMath::FInterpTo(SmoothBrake, Brake, DeltaTime, Settings.BrakeSmoothness * 10.0f);
	SmoothSteering = FMath::FInterpTo(SmoothSteering, Steering, DeltaTime, Settings.SteeringSmoothness * 10.0f);

	// Apply to cached vehicle movement component
	if (CachedMovementComponent)
	{
		CachedMovementComponent->SetThrottleInput(SmoothThrottle);
		CachedMovementComponent->SetBrakeInput(SmoothBrake);
		CachedMovementComponent->SetSteeringInput(SmoothSteering);
	}
}

const FAIDifficultySettings& AAICarController::GetCurrentSettings() const
{
	const FAIDifficultySettings* Settings = DifficultySettings.Find(Difficulty);
	if (Settings)
	{
		return *Settings;
	}

	// Default to normal
	static FAIDifficultySettings DefaultSettings;
	return DefaultSettings;
}
