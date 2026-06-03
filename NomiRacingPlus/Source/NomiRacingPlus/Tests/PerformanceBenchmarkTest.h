// Copyright NomiRacingPlus Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

/**
 * Performance Benchmark Tests
 *
 * Measures execution time of critical systems to detect performance regressions.
 * All benchmarks use FPlatformTime cycles for high-resolution timing.
 *
 * Categories:
 * - Vehicle Physics: Pacejka force calculation, thermal model, slip ratio/angle
 * - AI Calculations: Behavior tree decisions, overtake/defensive evaluation,
 *   rubber band scaling, sensor raycasting
 * - Rendering Pipeline: Camera system update, FOV calculation, replay interpolation,
 *   performance profiler metrics collection
 *
 * Run with: Automation test "NomiRacing.Performance.*"
 */

/**
 * Helper utilities for performance benchmarking
 */
class FPerformanceBenchmarkUtils
{
public:
	// Convert platform cycles to microseconds
	static double CyclesToMicroseconds(uint64 Cycles);

	// Run a callable N times and return average microseconds per iteration
	template <typename Func>
	static double BenchmarkIterations(int32 Iterations, Func&& Callable)
	{
		const uint64 StartCycles = FPlatformTime::Cycles64();
		for (int32 i = 0; i < Iterations; ++i)
		{
			Callable();
		}
		const uint64 EndCycles = FPlatformTime::Cycles64();
		return CyclesToMicroseconds(EndCycles - StartCycles) / static_cast<double>(Iterations);
	}

	// Log benchmark result to test output
	static void LogBenchmarkResult(FAutomationTestBase* Test, const FString& Name,
		double AvgMicroseconds, double TargetMicroseconds);
};
