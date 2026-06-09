// Copyright NomiRacingPlus Project. All Rights Reserved.

#include "PerformanceProfiler.h"
#include "NomiRacingPlus.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTime.h"
#include "Misc/App.h"
#include "Engine/Engine.h"

#if STATS
#include "GPUProfiler.h"
#endif

// ============================================================================
// Static helpers for safely reading render-thread stats
// ============================================================================

static int32 SafeGetDrawCallCount()
{
#if STATS
	return static_cast<int32>(GNumDrawCallsRHI[0]);
#else
	return 0;
#endif
}

static int32 SafeGetTriangleCount()
{
#if STATS
	return static_cast<int32>(GNumPrimitivesDrawnRHI[0]);
#else
	return 0;
#endif
}

static float SafeGetGPUTimeMs()
{
#if STATS
	const uint32 GPUCycles = RHIGetGPUFrameCycles();
	if (GPUCycles > 0)
	{
		return static_cast<float>(FPlatformTime::ToMilliseconds(static_cast<uint64>(GPUCycles)));
	}
#endif
	return 0.0f;
}

// ============================================================================
// UPerformanceProfiler
// ============================================================================

UPerformanceProfiler::UPerformanceProfiler()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UPerformanceProfiler::BeginPlay()
{
	Super::BeginPlay();
	StartProfiling();
}

void UPerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsProfiling)
	{
		return;
	}

	SamplingTimer += DeltaTime;
	if (SamplingTimer >= SamplingInterval)
	{
		CollectMetrics(SamplingTimer);
		SamplingTimer = 0.0f;
	}
}

// ============================================================================
// Profiling Control
// ============================================================================

void UPerformanceProfiler::StartProfiling()
{
	ResetProfiling();
	bIsProfiling = true;
	UE_LOG(LogNomiPerf, Log, TEXT("Performance profiling started (interval=%.3fs)"), SamplingInterval);
}

void UPerformanceProfiler::StopProfiling()
{
	bIsProfiling = false;
	AnalyzeMetrics();
	UE_LOG(LogNomiPerf, Log, TEXT("Performance profiling stopped (%d samples collected)"), MetricsHistory.Num());
}

void UPerformanceProfiler::ResetProfiling()
{
	MetricsHistory.Empty(MaxHistorySize);
	FPSHistory.Empty(MaxHistorySize);
	Issues.Empty();
	FrameTimeMin = TNumericLimits<float>::Max();
	FrameTimeMax = 0.0f;
}

// ============================================================================
// Metrics Collection
// ============================================================================

void UPerformanceProfiler::CollectMetrics(float DeltaTime)
{
	FPerformanceMetrics M;
	M.Timestamp = FDateTime::Now();

	// ── FPS & frame time ──────────────────────────────────────────────
	const float InstantFPS = (DeltaTime > SMALL_NUMBER) ? (1.0f / DeltaTime) : 0.0f;
	const float SmoothAlpha = FMath::Clamp(DeltaTime * 10.0f, 0.1f, 0.5f);
	SmoothedFPS = FMath::Lerp(SmoothedFPS, InstantFPS, SmoothAlpha);
	M.FPS = SmoothedFPS;

	M.FrameTime = DeltaTime * 1000.0f; // seconds -> ms
	FrameTimeAccumulator += DeltaTime;
	++FrameCount;

	FrameTimeMin = FMath::Min(FrameTimeMin, M.FrameTime);
	FrameTimeMax = FMath::Max(FrameTimeMax, M.FrameTime);

	// ── CPU thread times ──────────────────────────────────────────────
	M.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
	M.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);

	// ── GPU time ──────────────────────────────────────────────────────
	const float RawGPU = SafeGetGPUTimeMs();
	const float GPUAlpha = FMath::Clamp(DeltaTime * 8.0f, 0.1f, 0.4f);
	SmoothedGPUTime = FMath::Lerp(SmoothedGPUTime, RawGPU, GPUAlpha);
	M.GPUTime = SmoothedGPUTime;

	// ── Draw calls & triangles ────────────────────────────────────────
	M.DrawCalls = SafeGetDrawCallCount();
	M.TrianglesRendered = SafeGetTriangleCount();

	// ── Memory ────────────────────────────────────────────────────────
	CollectMemoryMetrics(M);

	// ── Store ─────────────────────────────────────────────────────────
	CurrentMetrics = M;

	if (MetricsHistory.Num() >= MaxHistorySize)
	{
		MetricsHistory.RemoveAt(0);
	}
	MetricsHistory.Add(M);

	FPSHistory.Add(M.FPS);

	CheckForIssues(M);
	OnPerformanceSample.Broadcast(M);
}

void UPerformanceProfiler::CollectMemoryMetrics(FPerformanceMetrics& M)
{
	const FPlatformMemoryStats Stats = FPlatformMemory::GetStats();

	M.MemoryUsageMB = static_cast<float>(Stats.UsedPhysical) / (1024.0f * 1024.0f);

	// Texture/mesh memory breakdowns require RHI-specific queries per platform.
	// These remain zero unless populated externally via a custom tracking hook.
	// For GPU-side memory totals, GRHIVideoMemorySize provides the VRAM budget.
	M.TextureMemoryMB = 0.0f;
	M.MeshMemoryMB = 0.0f;
}

// ============================================================================
// Issue Detection
// ============================================================================

void UPerformanceProfiler::CheckForIssues(const FPerformanceMetrics& Metrics)
{
	// Skip the first few samples (warm-up)
	if (FrameCount < 10)
	{
		return;
	}

	const float FrameTimeBudget = 1000.0f / Budget.TargetFPS;

	if (Metrics.FPS < Budget.MinAcceptableFPS)
	{
		AddIssue(TEXT("FPS"), FString::Printf(TEXT("FPS dropped to %.1f (minimum: %.1f)"),
			Metrics.FPS, Budget.MinAcceptableFPS), 0.8f,
			TEXT("Reduce draw distance, lower shadow quality, or enable more aggressive LOD."));
	}
	else if (Metrics.FrameTime > FrameTimeBudget * 1.5f)
	{
		AddIssue(TEXT("FrameTime"), FString::Printf(TEXT("Frame time %.2fms exceeds 150%% of budget (%.2fms)"),
			Metrics.FrameTime, FrameTimeBudget), 0.5f,
			TEXT("Profile game thread and render thread to find hotspots."));
	}

	if (Budget.MaxDrawCalls > 0 && Metrics.DrawCalls > Budget.MaxDrawCalls)
	{
		AddIssue(TEXT("DrawCalls"), FString::Printf(TEXT("Draw calls %d exceed budget %d"),
			Metrics.DrawCalls, Budget.MaxDrawCalls), 0.6f,
			TEXT("Use instanced rendering, merge meshes, or reduce visible actors."));
	}

	if (Budget.MaxTriangles > 0 && Metrics.TrianglesRendered > Budget.MaxTriangles)
	{
		AddIssue(TEXT("Triangles"), FString::Printf(TEXT("Triangle count %d exceeds budget %d"),
			Metrics.TrianglesRendered, Budget.MaxTriangles), 0.5f,
			TEXT("Enable LOD, use Nanite (Windows), or simplify high-poly meshes."));
	}

	if (Budget.MaxMemoryMB > 0.0f && Metrics.MemoryUsageMB > Budget.MaxMemoryMB)
	{
		AddIssue(TEXT("Memory"), FString::Printf(TEXT("Memory usage %.0f MB exceeds budget %.0f MB"),
			Metrics.MemoryUsageMB, Budget.MaxMemoryMB), 0.7f,
			TEXT("Reduce texture resolution, stream assets, or unload unused levels."));
	}

	if (Budget.MaxTextureMemoryMB > 0.0f && Metrics.TextureMemoryMB > Budget.MaxTextureMemoryMB)
	{
		AddIssue(TEXT("TextureMemory"), FString::Printf(TEXT("Texture memory %.0f MB exceeds budget %.0f MB"),
			Metrics.TextureMemoryMB, Budget.MaxTextureMemoryMB), 0.5f,
			TEXT("Lower texture streaming pool size or reduce texture resolutions."));
	}

	// Stutter detection: frame time spike > 3x the smoothed average
	if (MetricsHistory.Num() > 10)
	{
		float AvgFrameTime = 0.0f;
		const int32 SampleCount = FMath::Min(30, MetricsHistory.Num());
		for (int32 i = MetricsHistory.Num() - SampleCount; i < MetricsHistory.Num(); ++i)
		{
			AvgFrameTime += MetricsHistory[i].FrameTime;
		}
		AvgFrameTime /= static_cast<float>(SampleCount);

		if (Metrics.FrameTime > AvgFrameTime * 3.0f && Metrics.FrameTime > 50.0f)
		{
			AddIssue(TEXT("Stutter"), FString::Printf(TEXT("Frame spike: %.1fms (avg: %.1fms)"),
				Metrics.FrameTime, AvgFrameTime), 0.6f,
				TEXT("Check for asset streaming hitches, GC pauses, or shader compilation."));
		}
	}
}

void UPerformanceProfiler::AddIssue(const FString& Type, const FString& Description,
	float Severity, const FString& Recommendation)
{
	// De-duplicate: only flag each issue type once per profiling session
	for (const FPerformanceIssue& Existing : Issues)
	{
		if (Existing.Type == Type)
		{
			return; // Already flagged
		}
	}

	FPerformanceIssue Issue;
	Issue.Type = Type;
	Issue.Description = Description;
	Issue.Severity = Severity;
	Issue.Recommendation = Recommendation;

	Issues.Add(Issue);
	OnPerformanceIssue.Broadcast(Issue);

	UE_LOG(LogNomiPerf, Warning, TEXT("[Perf] %s: %s"), *Type, *Description);
}

void UPerformanceProfiler::AnalyzeMetrics()
{
	if (MetricsHistory.Num() < 10)
	{
		return;
	}

	const float P1 = GetFrameTimePercentile(0.99f);
	const float FPS1Pct = (P1 > SMALL_NUMBER) ? (1000.0f / P1) : 0.0f;

	UE_LOG(LogNomiPerf, Log, TEXT("Performance summary: %d samples, 1%% Low FPS = %.1f, issues = %d"),
		MetricsHistory.Num(), FPS1Pct, Issues.Num());
}

// ============================================================================
// Analysis & Queries
// ============================================================================

TMap<int32, int32> UPerformanceProfiler::GetFPSHistogram() const
{
	TMap<int32, int32> Histogram;
	for (const float FPSVal : FPSHistory)
	{
		const int32 Bin = FMath::FloorToInt32(FPSVal / 10.0f) * 10;
		Histogram.FindOrAdd(Bin)++;
	}
	return Histogram;
}

float UPerformanceProfiler::GetFrameTimePercentile(float Percentile) const
{
	if (MetricsHistory.Num() == 0)
	{
		return 0.0f;
	}

	TArray<float> Sorted;
	Sorted.Reserve(MetricsHistory.Num());
	for (const FPerformanceMetrics& M : MetricsHistory)
	{
		Sorted.Add(M.FrameTime);
	}
	Sorted.Sort();

	const int32 Index = FMath::Clamp(
		FMath::FloorToInt32(Percentile * Sorted.Num()),
		0,
		Sorted.Num() - 1
	);
	return Sorted[Index];
}

bool UPerformanceProfiler::MeetsBudget() const
{
	if (MetricsHistory.Num() == 0)
	{
		return true;
	}

	// Use the most recent sample
	const FPerformanceMetrics& Latest = MetricsHistory.Last();

	if (Latest.FPS < Budget.MinAcceptableFPS)
	{
		return false;
	}
	if (Budget.MaxDrawCalls > 0 && Latest.DrawCalls > Budget.MaxDrawCalls)
	{
		return false;
	}
	if (Budget.MaxTriangles > 0 && Latest.TrianglesRendered > Budget.MaxTriangles)
	{
		return false;
	}
	if (Budget.MaxMemoryMB > 0.0f && Latest.MemoryUsageMB > Budget.MaxMemoryMB)
	{
		return false;
	}
	if (Budget.MaxTextureMemoryMB > 0.0f && Latest.TextureMemoryMB > Budget.MaxTextureMemoryMB)
	{
		return false;
	}
	return true;
}

// ============================================================================
// Report Generation
// ============================================================================

FPerformanceReport UPerformanceProfiler::GenerateReport() const
{
	FPerformanceReport Report;
	Report.Timestamp = FDateTime::Now();

	if (MetricsHistory.Num() == 0)
	{
		return Report;
	}

	const int32 Count = MetricsHistory.Num();
	Report.Duration = FrameTimeAccumulator;

	// ── Aggregate: average, min, max ──────────────────────────────────
	FPerformanceMetrics& Avg = Report.AverageMetrics;
	FPerformanceMetrics& Min = Report.MinMetrics;
	FPerformanceMetrics& Max = Report.MaxMetrics;

	// Initialize min with extreme values
	Min.FPS = TNumericLimits<float>::Max();
	Min.FrameTime = TNumericLimits<float>::Max();
	Min.GameThreadTime = TNumericLimits<float>::Max();
	Min.RenderThreadTime = TNumericLimits<float>::Max();
	Min.GPUTime = TNumericLimits<float>::Max();
	Min.DrawCalls = TNumericLimits<int32>::Max();
	Min.TrianglesRendered = TNumericLimits<int32>::Max();
	Min.MemoryUsageMB = TNumericLimits<float>::Max();
	Min.TextureMemoryMB = TNumericLimits<float>::Max();
	Min.MeshMemoryMB = TNumericLimits<float>::Max();

	for (const FPerformanceMetrics& M : MetricsHistory)
	{
		Avg.FPS += M.FPS;
		Avg.FrameTime += M.FrameTime;
		Avg.GameThreadTime += M.GameThreadTime;
		Avg.RenderThreadTime += M.RenderThreadTime;
		Avg.GPUTime += M.GPUTime;
		Avg.DrawCalls += M.DrawCalls;
		Avg.TrianglesRendered += M.TrianglesRendered;
		Avg.MemoryUsageMB += M.MemoryUsageMB;
		Avg.TextureMemoryMB += M.TextureMemoryMB;
		Avg.MeshMemoryMB += M.MeshMemoryMB;

		Min.FPS = FMath::Min(Min.FPS, M.FPS);
		Min.FrameTime = FMath::Min(Min.FrameTime, M.FrameTime);
		Min.GameThreadTime = FMath::Min(Min.GameThreadTime, M.GameThreadTime);
		Min.RenderThreadTime = FMath::Min(Min.RenderThreadTime, M.RenderThreadTime);
		Min.GPUTime = FMath::Min(Min.GPUTime, M.GPUTime);
		Min.DrawCalls = FMath::Min(Min.DrawCalls, M.DrawCalls);
		Min.TrianglesRendered = FMath::Min(Min.TrianglesRendered, M.TrianglesRendered);
		Min.MemoryUsageMB = FMath::Min(Min.MemoryUsageMB, M.MemoryUsageMB);
		Min.TextureMemoryMB = FMath::Min(Min.TextureMemoryMB, M.TextureMemoryMB);
		Min.MeshMemoryMB = FMath::Min(Min.MeshMemoryMB, M.MeshMemoryMB);

		Max.FPS = FMath::Max(Max.FPS, M.FPS);
		Max.FrameTime = FMath::Max(Max.FrameTime, M.FrameTime);
		Max.GameThreadTime = FMath::Max(Max.GameThreadTime, M.GameThreadTime);
		Max.RenderThreadTime = FMath::Max(Max.RenderThreadTime, M.RenderThreadTime);
		Max.GPUTime = FMath::Max(Max.GPUTime, M.GPUTime);
		Max.DrawCalls = FMath::Max(Max.DrawCalls, M.DrawCalls);
		Max.TrianglesRendered = FMath::Max(Max.TrianglesRendered, M.TrianglesRendered);
		Max.MemoryUsageMB = FMath::Max(Max.MemoryUsageMB, M.MemoryUsageMB);
		Max.TextureMemoryMB = FMath::Max(Max.TextureMemoryMB, M.TextureMemoryMB);
		Max.MeshMemoryMB = FMath::Max(Max.MeshMemoryMB, M.MeshMemoryMB);
	}

	const float InvCount = 1.0f / static_cast<float>(Count);
	Avg.FPS *= InvCount;
	Avg.FrameTime *= InvCount;
	Avg.GameThreadTime *= InvCount;
	Avg.RenderThreadTime *= InvCount;
	Avg.GPUTime *= InvCount;
	Avg.DrawCalls = FMath::RoundToInt32(static_cast<float>(Avg.DrawCalls) * InvCount);
	Avg.TrianglesRendered = FMath::RoundToInt32(static_cast<float>(Avg.TrianglesRendered) * InvCount);
	Avg.MemoryUsageMB *= InvCount;
	Avg.TextureMemoryMB *= InvCount;
	Avg.MeshMemoryMB *= InvCount;

	// ── 1% low FPS (derived from 99th-percentile frame time) ─────────
	const float P99FrameTime = GetFrameTimePercentile(0.99f);
	Report.FPS1PercentLow = (P99FrameTime > SMALL_NUMBER) ? (1000.0f / P99FrameTime) : 0.0f;

	// ── Issues ────────────────────────────────────────────────────────
	Report.Issues = Issues;

	// ── Overall score (0-100) ─────────────────────────────────────────
	float Score = 100.0f;

	// FPS component: 40 points
	const float FPSTarget = Budget.TargetFPS;
	if (FPSTarget > 0.0f)
	{
		const float FPSRatio = FMath::Clamp(Avg.FPS / FPSTarget, 0.0f, 1.0f);
		Score -= (1.0f - FPSRatio) * 40.0f;
	}

	// 1% low component: 20 points
	if (FPSTarget > 0.0f)
	{
		const float LowRatio = FMath::Clamp(Report.FPS1PercentLow / FPSTarget, 0.0f, 1.0f);
		Score -= (1.0f - LowRatio) * 20.0f;
	}

	// Frame time consistency: 15 points (penalize high variance)
	if (FrameTimeMax > 0.0f && FrameTimeMin < TNumericLimits<float>::Max())
	{
		const float VarianceRatio = (FrameTimeMax - FrameTimeMin) / FMath::Max(Avg.FrameTime, 1.0f);
		Score -= FMath::Clamp(VarianceRatio * 5.0f, 0.0f, 15.0f);
	}

	// Draw calls: 10 points
	if (Budget.MaxDrawCalls > 0)
	{
		const float DCRatio = FMath::Clamp(
			static_cast<float>(Avg.DrawCalls) / static_cast<float>(Budget.MaxDrawCalls),
			0.0f, 1.0f
		);
		Score -= DCRatio * 10.0f;
	}

	// Memory: 10 points
	if (Budget.MaxMemoryMB > 0.0f)
	{
		const float MemRatio = FMath::Clamp(Avg.MemoryUsageMB / Budget.MaxMemoryMB, 0.0f, 1.0f);
		Score -= MemRatio * 10.0f;
	}

	// Issue penalties
	for (const FPerformanceIssue& Issue : Issues)
	{
		Score -= Issue.Severity * 5.0f;
	}

	// Budget compliance bonus
	Report.bMeetsBudget = MeetsBudget();
	if (Report.bMeetsBudget)
	{
		Score += 5.0f;
	}

	Report.OverallScore = FMath::Clamp(Score, 0.0f, 100.0f);

	// Sort issues by severity (highest first)
	Report.Issues.Sort([](const FPerformanceIssue& A, const FPerformanceIssue& B)
	{
		return A.Severity > B.Severity;
	});

	return Report;
}
