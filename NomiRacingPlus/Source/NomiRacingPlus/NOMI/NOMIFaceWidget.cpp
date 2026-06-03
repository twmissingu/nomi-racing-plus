// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "NOMIFaceWidget.h"
#include "NomiRacingPlus.h"

UNOMIFaceWidget::UNOMIFaceWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Default eye states
	LeftEyeState.Openness = 1.0f;
	LeftEyeState.ShapeType = 0;
	LeftEyeState.PupilSize = 0.5f;
	LeftEyeState.Color = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);

	RightEyeState = LeftEyeState;

	// Default mouth state
	MouthState.OpenAmount = 0.0f;
	MouthState.ShapeType = 0;
	MouthState.WidthScale = 1.0f;
}

void UNOMIFaceWidget::UpdateExpression(ENOMIExpression Expression)
{
	if (Expression != CurrentExpression)
	{
		CurrentExpression = Expression;
		TargetExpression = Expression;
		ExpressionBlendAlpha = 0.0f;

		ApplyExpressionSettings(Expression);

		UE_LOG(LogNomiNOMI, Verbose, TEXT("NOMI face expression updated: %d"), (int32)Expression);
	}
}

void UNOMIFaceWidget::SetEyeOpenness(float LeftEye, float RightEye)
{
	LeftEyeState.Openness = FMath::Clamp(LeftEye, 0.0f, 1.0f);
	RightEyeState.Openness = FMath::Clamp(RightEye, 0.0f, 1.0f);
}

void UNOMIFaceWidget::SetMouthOpen(float Amount)
{
	MouthState.OpenAmount = FMath::Clamp(Amount, 0.0f, 1.0f);

	// Switch to speaking shape when mouth is open
	if (Amount > 0.1f)
	{
		MouthState.ShapeType = 4; // Speaking
	}
	else if (MouthState.ShapeType == 4)
	{
		// Return to expression default
		ApplyExpressionSettings(CurrentExpression);
	}
}

void UNOMIFaceWidget::SetMouthShape(int32 ShapeType)
{
	MouthState.ShapeType = FMath::Clamp(ShapeType, 0, 4);
}

void UNOMIFaceWidget::ApplyExpressionSettings(ENOMIExpression Expression)
{
	switch (Expression)
	{
	case ENOMIExpression::Neutral:
		// Default neutral face
		LeftEyeState.ShapeType = 0;  // Normal
		RightEyeState.ShapeType = 0;
		LeftEyeState.PupilSize = 0.5f;
		RightEyeState.PupilSize = 0.5f;
		MouthState.ShapeType = 0;    // Neutral
		MouthState.WidthScale = 1.0f;
		MouthState.OpenAmount = 0.0f;
		break;

	case ENOMIExpression::Happy:
		// Happy: slightly closed eyes, smile
		LeftEyeState.ShapeType = 1;  // Happy (slightly closed)
		RightEyeState.ShapeType = 1;
		LeftEyeState.Openness = 0.7f;
		RightEyeState.Openness = 0.7f;
		LeftEyeState.PupilSize = 0.6f;
		RightEyeState.PupilSize = 0.6f;
		MouthState.ShapeType = 1;    // Smile
		MouthState.WidthScale = 1.2f;
		MouthState.OpenAmount = 0.1f;
		break;

	case ENOMIExpression::Sad:
		// Sad: droopy eyes, frown
		LeftEyeState.ShapeType = 2;  // Sad
		RightEyeState.ShapeType = 2;
		LeftEyeState.Openness = 0.6f;
		RightEyeState.Openness = 0.6f;
		LeftEyeState.PupilSize = 0.4f;
		RightEyeState.PupilSize = 0.4f;
		MouthState.ShapeType = 2;    // Frown
		MouthState.WidthScale = 0.8f;
		MouthState.OpenAmount = 0.0f;
		break;

	case ENOMIExpression::Sleepy:
		// Sleepy: half-closed eyes, small mouth
		LeftEyeState.ShapeType = 0;  // Normal but half closed
		RightEyeState.ShapeType = 0;
		LeftEyeState.Openness = 0.3f;
		RightEyeState.Openness = 0.3f;
		LeftEyeState.PupilSize = 0.3f;
		RightEyeState.PupilSize = 0.3f;
		MouthState.ShapeType = 0;    // Neutral, small
		MouthState.WidthScale = 0.6f;
		MouthState.OpenAmount = 0.05f;
		break;

	case ENOMIExpression::Curious:
		// Curious: wide eyes, small smile
		LeftEyeState.ShapeType = 3;  // Surprised/wide
		RightEyeState.ShapeType = 3;
		LeftEyeState.Openness = 1.0f;
		RightEyeState.Openness = 1.0f;
		LeftEyeState.PupilSize = 0.7f;
		RightEyeState.PupilSize = 0.7f;
		MouthState.ShapeType = 0;    // Neutral
		MouthState.WidthScale = 0.9f;
		MouthState.OpenAmount = 0.05f;
		break;

	case ENOMIExpression::Surprised:
		// Surprised: very wide eyes, open mouth
		LeftEyeState.ShapeType = 3;  // Surprised
		RightEyeState.ShapeType = 3;
		LeftEyeState.Openness = 1.0f;
		RightEyeState.Openness = 1.0f;
		LeftEyeState.PupilSize = 0.2f; // Small pupils = surprised
		RightEyeState.PupilSize = 0.2f;
		MouthState.ShapeType = 3;    // Surprised (O shape)
		MouthState.WidthScale = 0.7f;
		MouthState.OpenAmount = 0.6f;
		break;

	case ENOMIExpression::Confused:
		// Confused: asymmetric eyes, wavy mouth
		LeftEyeState.ShapeType = 0;  // Normal
		RightEyeState.ShapeType = 2; // Sad (different from left = confused)
		LeftEyeState.Openness = 0.8f;
		RightEyeState.Openness = 0.6f;
		LeftEyeState.PupilSize = 0.5f;
		RightEyeState.PupilSize = 0.4f;
		MouthState.ShapeType = 0;    // Neutral, slightly open
		MouthState.WidthScale = 0.9f;
		MouthState.OpenAmount = 0.1f;
		break;

	case ENOMIExpression::Excited:
		// Excited: wide eyes, big smile
		LeftEyeState.ShapeType = 3;  // Wide
		RightEyeState.ShapeType = 3;
		LeftEyeState.Openness = 1.0f;
		RightEyeState.Openness = 1.0f;
		LeftEyeState.PupilSize = 0.8f; // Large pupils = excited
		RightEyeState.PupilSize = 0.8f;
		MouthState.ShapeType = 1;    // Big smile
		MouthState.WidthScale = 1.4f;
		MouthState.OpenAmount = 0.3f;
		break;
	}
}
