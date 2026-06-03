// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PerformanceProfiler.generated.h"

/**
 * Performance metrics
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FPerformanceMetrics
{
	GENERATED_BODY()

	// Frames per second
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float FPS = 0.0f;

	// Frame time (ms)
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float FrameTime = 0.0f;

	// Game thread time (ms)
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float GameThreadTime = 0.0f;

	// Render thread time (ms)
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float RenderThreadTime = 0.0f;

	// GPU time (ms)
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float GPUTime = 0.0f;

	// Draw calls
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 DrawCalls = 0;

	// Triangles rendered
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 TrianglesRendered = 0;

	// Memory usage (MB)
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float MemoryUsageMB = 0.0f;

	// Texture memory (MB)
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float TextureMemoryMB = 0.0f;

	// Mesh memory (MB)
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float MeshMemoryMB = 0.0f;

	// Timestamp
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	FDateTime Timestamp;
};

/**
 * Performance budget
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FPerformanceBudget
{
	GENERATED_BODY()

	// Target FPS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	float TargetFPS = 60.0f;

	// Minimum acceptable FPS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	float MinAcceptableFPS = 30.0f;

	// Maximum frame time (ms)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	float MaxFrameTime = 16.67f;

	// Maximum draw calls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	int32 MaxDrawCalls = 2000;

	// Maximum triangles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	int32 MaxTriangles = 5000000;

	// Maximum memory (MB)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	float MaxMemoryMB = 4096.0f;

	// Maximum texture memory (MB)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	float MaxTextureMemoryMB = 2048.0f;
};

/**
 * Performance issue
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FPerformanceIssue
{
	GENERATED_BODY()

	// Issue type
	UPROPERTY(BlueprintReadOnly, Category = "Issue")
	FString Type;

	// Description
	UPROPERTY(BlueprintReadOnly, Category = "Issue")
	FString Description;

	// Severity (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Issue")
	float Severity = 0.0f;

	// Location (if applicable)
	UPROPERTY(BlueprintReadOnly, Category = "Issue")
	FString Location;

	// Recommendation
	UPROPERTY(BlueprintReadOnly, Category = "Issue")
	FString Recommendation;
};

/**
 * Performance report
 */
USTRUCT(BlueprintType)
struct NOMIRACINGPLUS_API FPerformanceReport
{
	GENERATED_BODY()

	// Average metrics
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	FPerformanceMetrics AverageMetrics;

	// Min metrics
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	FPerformanceMetrics MinMetrics;

	// Max metrics
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	FPerformanceMetrics MaxMetrics;

	// 1% low FPS
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	float FPS1PercentLow = 0.0f;

	// Issues found
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	TArray<FPerformanceIssue> Issues;

	// Overall score (0-100)
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	float OverallScore = 0.0f;

	// Meets budget?
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	bool bMeetsBudget = true;

	// Report timestamp
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	FDateTime Timestamp;

	// Duration (seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Report")
	float Duration = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceSample, const FPerformanceMetrics&, Metrics);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceIssue, const FPerformanceIssue&, Issue);

/**
 * Performance Profiler
 * Tracks and analyzes game performance metrics
 */
UCLASS(ClassGroup = (NomiRacing), meta = (BlueprintSpawnableComponent))
class NOMIRACINGPLUS_API UPerformanceProfiler : public UActorComponent
{
	GENERATED_BODY()

public:
	UPerformanceProfiler();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Profiling Control

	// Start profiling
	UFUNCTION(BlueprintCallable, Category = "Profiler")
	void StartProfiling();

	// Stop profiling
	UFUNCTION(BlueprintCallable, Category = "Profiler")
	void StopProfiling();

	// Is profiling?
	UFUNCTION(BlueprintCallable, Category = "Profiler")
	bool IsProfiling() const { return bIsProfiling; }

	// Reset profiling data
	UFUNCTION(BlueprintCallable, Category = "Profiler")
	void ResetProfiling();

	// Get current metrics
	UFUNCTION(BlueprintCallable, Category = "Profiler")
	const FPerformanceMetrics& GetCurrentMetrics() const { return CurrentMetrics; }

	// Get performance report
	UFUNCTION(BlueprintCallable, Category = "Profiler")
	FPerformanceReport GenerateReport() const;

	// Budget

	// Set performance budget
	UFUNCTION(BlueprintCallable, Category = "Profiler|Budget")
	void SetBudget(const FPerformanceBudget& InBudget) { Budget = InBudget; }

	// Get performance budget
	UFUNCTION(BlueprintCallable, Category = "Profiler|Budget")
	const FPerformanceBudget& GetBudget() const { return Budget; }

	// Check if meets budget
	UFUNCTION(BlueprintCallable, Category = "Profiler|Budget")
	bool MeetsBudget() const;

	// Analysis

	// Get FPS histogram
	UFUNCTION(BlueprintCallable, Category = "Profiler|Analysis")
	TMap<int32, int32> GetFPSHistogram() const;

	// Get frame time percentile
	UFUNCTION(BlueprintCallable, Category = "Profiler|Analysis")
	float GetFrameTimePercentile(float Percentile) const;

	// Get issues
	UFUNCTION(BlueprintCallable, Category = "Profiler|Analysis")
	const TArray<FPerformanceIssue>& GetIssues() const { return Issues; }

	// Events

	// Performance sample delegate
	UPROPERTY(BlueprintAssignable, Category = "Profiler|Events")
	FOnPerformanceSample OnPerformanceSample;

	// Performance issue delegate
	UPROPERTY(BlueprintAssignable, Category = "Profiler|Events")
	FOnPerformanceIssue OnPerformanceIssue;

protected:
	// Current metrics
	UPROPERTY(BlueprintReadOnly, Category = "Profiler")
	FPerformanceMetrics CurrentMetrics;

	// Budget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiler|Budget")
	FPerformanceBudget Budget;

	// Is profiling?
	UPROPERTY(BlueprintReadOnly, Category = "Profiler")
	bool bIsProfiling = false;

	// Collected metrics
	UPROPERTY(BlueprintReadOnly, Category = "Profiler")
	TArray<FPerformanceMetrics> MetricsHistory;

	// Issues
	UPROPERTY(BlueprintReadOnly, Category = "Profiler")
	TArray<FPerformanceIssue> Issues;

	// Sampling interval (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiler")
	float SamplingInterval = 0.1f;

	// Maximum history size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiler")
	int32 MaxHistorySize = 10000;

private:
	// Collect metrics
	void CollectMetrics(float DeltaTime);

	// Collect memory-specific metrics
	void CollectMemoryMetrics(FPerformanceMetrics& M);

	// Analyze metrics
	void AnalyzeMetrics();

	// Check for issues
	void CheckForIssues(const FPerformanceMetrics& Metrics);

	// Add issue
	void AddIssue(const FString& Type, const FString& Description, float Severity, const FString& Recommendation);

	// Sampling timer
	float SamplingTimer = 0.0f;

	// Smoothed FPS (exponential moving average)
	float SmoothedFPS = 0.0f;

	// Smoothed GPU time (exponential moving average)
	float SmoothedGPUTime = 0.0f;

	// Frame time accumulator
	float FrameTimeAccumulator = 0.0f;
	int32 FrameCount = 0;

	// Frame time range tracking
	float FrameTimeMin = TNumericLimits<float>::Max();
	float FrameTimeMax = 0.0f;

	// FPS history for percentile calculation
	TArray<float> FPSHistory;
};
