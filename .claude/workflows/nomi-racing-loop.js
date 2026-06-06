export const meta = {
  name: 'nomi-racing-autonomous-loop',
  description: '10-cycle autonomous dev loop: audit→develop→test→review→iterate for NIO Racing Plus',
  phases: [
    { title: 'Audit', detail: 'Doc↔Code consistency check' },
    { title: 'Develop', detail: 'Feature completion + bug fixes' },
    { title: 'Test', detail: 'Test coverage + validation' },
    { title: 'Review', detail: 'Multi-role code review' },
    { title: 'Iterate', detail: 'Senior Player + PM UX critique' },
  ],
}

const ROOT = '/Users/twzhan/Documents/dev/nomi-racing-plus'
const SRC = `${ROOT}/NomiRacingPlus/Source/NomiRacingPlus`
const CONTENT = `${ROOT}/NomiRacingPlus/Content`
const DOCS = `${ROOT}/NomiRacingPlus/Docs`

// Structured output schemas
const AUDIT_SCHEMA = {
  type: 'object',
  properties: {
    inconsistencies: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          docFile: { type: 'string' },
          codeFile: { type: 'string' },
          issue: { type: 'string' },
          severity: { type: 'string', enum: ['critical', 'high', 'medium', 'low'] },
          fix: { type: 'string' },
        },
      },
    },
    staleCode: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          file: { type: 'string' },
          line: { type: 'number' },
          issue: { type: 'string' },
        },
      },
    },
    summary: { type: 'string' },
  },
}

const REVIEW_SCHEMA = {
  type: 'object',
  properties: {
    findings: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          file: { type: 'string' },
          line: { type: 'number' },
          severity: { type: 'string', enum: ['critical', 'high', 'medium', 'low'] },
          category: { type: 'string' },
          issue: { type: 'string' },
          fix: { type: 'string' },
        },
      },
    },
    summary: { type: 'string' },
  },
}

const UX_SCHEMA = {
  type: 'object',
  properties: {
    insights: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          area: { type: 'string' },
          currentState: { type: 'string' },
          proposedChange: { type: 'string' },
          expectedImpact: { type: 'string' },
          impactPercent: { type: 'number' },
          complexity: { type: 'string', enum: ['low', 'medium', 'high'] },
          files: { type: 'array', items: { type: 'string' } },
        },
      },
    },
    topPriority: { type: 'string' },
    summary: { type: 'string' },
  },
}

// Cycle-specific focus areas
const CYCLE_FOCUS = {
  1: { dev: 'baseline wiring audit', desc: 'Establish baseline — fix critical compilation and wiring issues' },
  2: { dev: 'menu→race flow', desc: 'Main menu to race start flow — vehicle selection, track loading, game mode setup' },
  3: { dev: 'race lifecycle', desc: 'Race start → countdown → racing → finish — checkpoint tracking, position calculation' },
  4: { dev: 'camera + HUD', desc: 'Camera mode transitions, race HUD real-time updates, speed/position/lap display' },
  5: { dev: 'NOMI + audio', desc: 'NOMI commentary triggers during race events, motor sound integration' },
  6: { dev: 'results + progression', desc: 'Post-race results, save/load progression, achievements' },
  7: { dev: 'championship mode', desc: 'Multi-race championship series, standings, series completion' },
  8: { dev: 'accessibility + settings', desc: 'Settings menu implementation, input remapping, color themes, accessibility' },
  9: { dev: 'performance optimization', desc: 'Memory leaks, frame rate, particle budget, component caching' },
  10: { dev: 'final polish', desc: 'Edge cases, error recovery, loading states, final doc sync' },
}

function auditPrompt(cycle) {
  return `You are a documentation-code consistency auditor for the NIO Racing Plus UE5 racing game project.

**Your task:** Cross-reference ALL documentation against the actual source code and find inconsistencies.

**Documentation files to check:**
- ${ROOT}/CLAUDE.md
- ${ROOT}/README.md
- ${ROOT}/README_zh.md
- ${ROOT}/CHANGELOG.md
- ${ROOT}/PLAN.md
- ${DOCS}/API_Reference.md
- ${DOCS}/Asset_Pipeline_Guide.md
- ${DOCS}/Build_System.md
- ${DOCS}/Custom_Vehicle_Guide.md
- ${DOCS}/Development_Guide.md
- ${DOCS}/Gameplay_Guide.md
- ${DOCS}/Setup_Guide.md
- ${SRC}/Tests/README.md

**Source code to verify against:**
- All .h and .cpp files under ${SRC}/
- All JSON configs under ${CONTENT}/
- All INI configs under ${ROOT}/NomiRacingPlus/Config/

**Check for:**
1. Documented classes/functions/enums that don't exist in code
2. Implemented features not documented
3. API signatures that changed without doc updates
4. Config values in docs that don't match JSON files
5. Stale TODOs/FIXMEs/HACKs in source code
6. Test coverage gaps vs documented test categories
7. Build instructions that are incorrect or outdated
8. Cross-platform notes that are wrong

**Cycle ${cycle} focus:** ${CYCLE_FOCUS[cycle].desc}

Return structured findings with file paths, line numbers, severity, and suggested fixes.`
}

function developPrompt(cycle, auditSummary) {
  return `You are a senior C++ game developer working on NIO Racing Plus, a UE5 racing game.

**Your task:** Fix issues and complete features for Cycle ${cycle}.

**Focus area:** ${CYCLE_FOCUS[cycle].dev} — ${CYCLE_FOCUS[cycle].desc}

**Audit findings to address:**
${auditSummary}

**Source code location:** ${SRC}/

**Key system files:**
- Vehicles: ${SRC}/Vehicles/NIOVehicleMovementComponent.cpp, NIOVehicleBase.cpp, VehicleStateManager.cpp
- AI: ${SRC}/AI/AIBehaviorTree.cpp, AICarController.cpp, AISensorSystem.cpp
- Race: ${SRC}/Race/RaceManager.cpp, CheckpointSystem.cpp, RaceProgression.cpp
- Camera: ${SRC}/Core/CameraSystem.cpp, CameraSystem_Modes.cpp
- NOMI: ${SRC}/NOMI/CommentaryEngine.cpp, NOMIController.cpp
- UI: ${SRC}/UI/MainMenuWidget.cpp, RaceHUD.cpp, MenuManager.cpp, GarageWidget.cpp, TrackSelectWidget.cpp
- Audio: ${SRC}/Core/AudioManager.cpp
- Game: ${SRC}/Core/NomiGameMode.cpp, NomiGameInstance.cpp, NomiPlayerController.cpp

**Rules:**
1. Read each file before modifying it
2. Make minimal, surgical changes — don't refactor what isn't broken
3. Follow UE5 coding conventions (UCLASS/UPROPERTY/UFUNCTION macros)
4. Always null-check GetWorld(), GetOwner(), component pointers
5. Cache FindComponentByClass() results in BeginPlay()
6. Fix compilation issues, missing includes, type mismatches
7. Ensure systems wire together correctly
8. After making changes, list every file modified and what was changed

Return a summary of all changes made with file paths and descriptions.`
}

function testPrompt(cycle, changesSummary) {
  return `You are a test engineer for NIO Racing Plus, a UE5 racing game.

**Your task:** Verify test coverage for Cycle ${cycle} changes and add missing tests.

**Recent changes:**
${changesSummary}

**Test files location:** ${SRC}/Tests/

**Existing test files:**
- VehicleSystemTest.h/.cpp — vehicle state, physics config, drift, torque
- TirePhysicsTest.h/.cpp — Pacejka formula, thermal model, tire wear
- AITest.h/.cpp — behavior tree, overtake, defensive evaluator
- NOMISystemTest.h/.cpp — commentary engine, NOMI controller, comments
- RaceSystemTest.h/.cpp — race manager, checkpoint, position, timer
- CameraSystemTest.h/.cpp — camera modes
- ProgressionTest.h/.cpp — progression, achievements, championship
- VehicleRaceIntegrationTest.h/.cpp — vehicle-race interaction
- AIRaceManagerIntegrationTest.h/.cpp — AI-race interaction
- NOMIEventIntegrationTest.h/.cpp — NOMI-race events
- PerformanceBenchmarkTest.h/.cpp — perf benchmarks
- TestUtilities.h/.cpp — mocks, helpers

**Check:**
1. Read each test file to understand current coverage
2. Identify which changed code paths are NOT covered by existing tests
3. Add new test cases for uncovered code (follow existing test patterns)
4. Verify test utilities (mocks, helpers) are still correct
5. Look for test anti-patterns: tests that always pass, missing edge cases
6. Ensure integration tests cover the system interactions modified this cycle

**Rules:**
- Follow AAA pattern (Arrange-Act-Assert)
- Use descriptive test names explaining the behavior
- Add to existing test files, don't create new ones unless needed
- Mock external dependencies, don't require actual UE5 runtime

Return a summary of test coverage analysis and any new tests added.`
}

function cppReviewPrompt(changesSummary) {
  return `You are a C++ expert reviewing NIO Racing Plus code changes.

**Your task:** Review all Cycle changes for C++ correctness, UE5 conventions, and memory safety.

**Changes to review:**
${changesSummary}

**Review checklist:**
1. Memory safety: null pointer dereferences, use-after-free, dangling references
2. RAII: proper resource cleanup, smart pointer usage
3. UE5 conventions: UCLASS/UPROPERTY/UFUNCTION macro correctness, proper prefixes (I/E/F)
4. Header hygiene: include guards, forward declarations, include order
5. Const correctness: const methods, const references, constexpr where appropriate
6. Move semantics: unnecessary copies, missing std::move
7. Thread safety: game thread vs render thread access patterns
8. Error handling: null checks on GetWorld(), GetOwner(), component lookups
9. Performance: per-frame allocations, unnecessary FindComponentByClass calls, missing caching

Return structured findings with file, line, severity, and fix suggestions.`
}

function securityReviewPrompt(changesSummary) {
  return `You are a security reviewer for NIO Racing Plus, a UE5 racing game.

**Your task:** Security audit of Cycle changes.

**Changes to review:**
${changesSummary}

**Security checklist:**
1. Save file tampering: are save/load operations robust against corrupted data?
2. Input validation: are player inputs (steering, throttle) bounds-checked?
3. Buffer overflows: array access, string operations, JSON parsing
4. Integer overflow: race timer, score calculations, lap counters
5. Denial of service: unbounded allocations, infinite loops in AI
6. Information leakage: error messages that expose internal state
7. Config injection: can malicious JSON configs cause crashes?
8. Memory safety: no use-after-free, no double-free, no null deref

This is a single-player game with no network component, so focus on local attack surface.

Return structured findings with file, line, severity, and fix suggestions.`
}

function archReviewPrompt(changesSummary) {
  return `You are a software architect reviewing NIO Racing Plus code changes.

**Your task:** Architecture and design review of Cycle changes.

**Changes to review:**
${changesSummary}

**Architecture checklist:**
1. SOLID principles: single responsibility, open/closed, dependency inversion
2. Coupling: are systems properly decoupled? Can vehicles be tested without race manager?
3. Cohesion: are related functions grouped in the right classes?
4. UE5 patterns: proper use of GameMode, PlayerController, GameState, Actor hierarchy
5. Data flow: do events flow through proper channels (delegates, interfaces)?
6. Configuration: is behavior data-driven (JSON configs) vs hardcoded?
7. Extensibility: can new vehicles/tracks/modes be added without modifying existing code?
8. Error propagation: do errors bubble up with context, or get swallowed?

Return structured findings with file, line, severity, and fix suggestions.`
}

function seniorPlayerPrompt(cycle, changesSummary) {
  return `You are a **senior racing game player** with 20+ years of experience playing Gran Turismo, Forza, Assetto Corsa, and Need for Speed. You've just been asked to evaluate NIO Racing Plus.

**Your task:** From a PLAYER's perspective, identify the top improvements that would make this game 10%+ more enjoyable.

**Current game state (Cycle ${cycle} changes):**
${changesSummary}

**Game features:**
- 4 vehicles: NIO EP9 (hypercar), ET7 (sedan), ES7 (SUV), Xiaomi SU7 Ultra
- 5 tracks: NIO City Circuit, Shanghai Pudong, Speedway Oval, Mountain Pass, Desert Rally
- 3 modes: Street GT, NIO Championship, Baja Rally
- 7 camera modes: Chase, Hood, Cockpit, Bumper, Free, Cinematic, Replay
- AI opponents with 4 difficulty levels and rubber banding
- NOMI companion with 500+ Chinese commentary lines
- Pacejka tire physics with EV drivetrain simulation

**Think like a player:**
- What would make the FIRST 5 MINUTES impressive?
- What would make you want to play ANOTHER race?
- What would make you show this to a FRIEND?
- What's the "feel" — is the driving satisfying? Is the AI fun to race against?
- What are racing game deal-breakers? (bad camera, unfair AI, no feedback, boring progression)

**Propose 3-5 concrete improvements** that would each deliver 10%+ better user experience. Be specific — which files, which systems, what exactly changes. Don't suggest cosmetic tweaks; suggest gameplay-impacting improvements.

For each improvement:
- What's the current state (bad)
- What should it be (good)
- Why does it matter (impact)
- Which files need to change`
}

function productManagerPrompt(cycle, changesSummary) {
  return `You are a **product manager** for NIO Racing Plus, a UE5 racing game featuring NIO electric vehicles.

**Your task:** From a PRODUCT perspective, identify the highest-impact improvements to make this game feel complete and polished.

**Current game state (Cycle ${cycle} changes):**
${changesSummary}

**Product context:**
- Target audience: NIO fans, EV enthusiasts, casual racing game players
- Differentiator: Real NIO vehicles with EV physics, NOMI AI companion
- Platform: Windows + macOS
- Stage: Pre-alpha, needs to reach "playable demo" quality
- Competitive landscape: Need for Speed, Forza Horizon, Gran Turismo

**Product analysis framework:**
1. **Core Loop:** Is Menu→Race→Results→Replay satisfying?
2. **First-Time Experience:** Can a new player figure out how to play in 60 seconds?
3. **Retention:** What makes someone come back for another session?
4. **Unique Value:** What does this game have that others don't?
5. **Polish:** What small details make the difference between "prototype" and "game"?

**Propose 3-5 product improvements** that would:
- Increase perceived quality by 10%+
- Be achievable in code changes (not requiring new 3D assets)
- Have clear before/after states
- Prioritize by impact × feasibility

For each improvement:
- Product rationale (why this matters)
- Expected user impact (measurable if possible)
- Implementation scope (which files, estimated complexity)
- Priority vs other improvements`
}

function updateDocsPrompt(cycle, audit, tests, reviews, ux) {
  return `You are the documentation maintainer for NIO Racing Plus.

**Your task:** Update all documentation to reflect Cycle ${cycle} changes.

**Files to update:**
1. ${ROOT}/ITERATION_LOG.md — Add a new cycle entry with:
   - Audit findings and severity distribution
   - Changes made (files + descriptions)
   - Review findings and resolution status
   - UX improvements proposed with impact estimates
   - Test coverage changes

2. ${ROOT}/WORKFLOW_STATUS.md — Update the cycle row to ✅

3. ${ROOT}/CHANGELOG.md — Add cycle changes under [Unreleased]

4. ${ROOT}/CLAUDE.md — Update if project structure, key files, or common pitfalls changed

5. ${ROOT}/PLAN.md — Add UX improvement proposals from the Iterate phase

6. ${ROOT}/README.md / ${ROOT}/README_zh.md — Update if features or capabilities changed

**Review data:**
${reviews}

**UX insights:**
${ux}

**Test results:**
${tests}

**Audit results:**
${audit}

Be thorough but concise. Each ITERATION_LOG entry should be a clear record of what was done and why.`
}

function commitPrompt(cycle) {
  return `You are the release manager for NIO Racing Plus.

**Your task:** Create a git commit for Cycle ${cycle} changes.

**Steps:**
1. Run \`git status\` to see all modified files
2. Run \`git diff --stat\` to see change summary
3. Stage all relevant changes: \`git add\` the modified source, test, and documentation files
4. Create a commit with format: \`fix: cycle ${cycle} — ${CYCLE_FOCUS[cycle].dev}\`
5. Use the commit body to list key changes

**Rules:**
- Only commit source, test, config, and documentation files
- Never commit build artifacts, binaries, or IDE files
- Keep commits atomic — one commit for the entire cycle
- Commit message should summarize the cycle's work clearly

After committing, report the commit hash and summary.`
}

// Main workflow
for (let cycle = 1; cycle <= 10; cycle++) {
  log(`\n${'='.repeat(60)}`)
  log(`Starting Cycle ${cycle}/10: ${CYCLE_FOCUS[cycle].dev}`)
  log(`${'='.repeat(60)}\n`)

  // Phase 1: AUDIT
  phase('Audit')
  log(`Cycle ${cycle}: Auditing doc↔code consistency...`)
  const audit = await agent(auditPrompt(cycle), {
    label: `audit:c${cycle}`,
    phase: 'Audit',
    schema: AUDIT_SCHEMA,
  })

  const auditSummary = audit
    ? `Found ${audit.inconsistencies?.length || 0} inconsistencies, ${audit.staleCode?.length || 0} stale code issues. ${audit.summary || ''}`
    : 'Audit skipped.'

  // Phase 2: DEVELOP
  phase('Develop')
  log(`Cycle ${cycle}: Developing — ${CYCLE_FOCUS[cycle].dev}...`)
  const developResult = await agent(developPrompt(cycle, auditSummary), {
    label: `develop:c${cycle}`,
    phase: 'Develop',
  })

  // Phase 3: TEST
  phase('Test')
  log(`Cycle ${cycle}: Testing and verifying...`)
  const testResult = await agent(testPrompt(cycle, developResult || 'No changes made.'), {
    label: `test:c${cycle}`,
    phase: 'Test',
  })

  // Phase 4: REVIEW (3 parallel reviewers)
  phase('Review')
  log(`Cycle ${cycle}: Multi-role code review (3 reviewers)...`)
  const [cppReview, secReview, archReview] = await parallel([
    () => agent(cppReviewPrompt(developResult || 'No changes.'), {
      label: `review-cpp:c${cycle}`,
      phase: 'Review',
      schema: REVIEW_SCHEMA,
    }),
    () => agent(securityReviewPrompt(developResult || 'No changes.'), {
      label: `review-sec:c${cycle}`,
      phase: 'Review',
      schema: REVIEW_SCHEMA,
    }),
    () => agent(archReviewPrompt(developResult || 'No changes.'), {
      label: `review-arch:c${cycle}`,
      phase: 'Review',
      schema: REVIEW_SCHEMA,
    }),
  ])

  // Fix critical/high review findings
  const allFindings = [
    ...(cppReview?.findings || []),
    ...(secReview?.findings || []),
    ...(archReview?.findings || []),
  ]
  const criticalFindings = allFindings.filter(f => f.severity === 'critical' || f.severity === 'high')

  if (criticalFindings.length > 0) {
    log(`Cycle ${cycle}: ${criticalFindings.length} critical/high findings — fixing...`)
    const fixPrompt = `Fix these CRITICAL/HIGH code review findings:\n\n${criticalFindings.map(f =>
      `- [${f.severity.toUpperCase()}] ${f.file}:${f.line} — ${f.issue}\n  Fix: ${f.fix}`
    ).join('\n\n')}\n\nApply the fixes. Read each file before modifying.`
    await agent(fixPrompt, { label: `fix-findings:c${cycle}`, phase: 'Review' })
  }

  // Phase 5: ITERATE (2 parallel UX perspectives)
  phase('Iterate')
  log(`Cycle ${cycle}: Senior Player + PM UX critique...`)
  const [playerUX, pmUX] = await parallel([
    () => agent(seniorPlayerPrompt(cycle, developResult || 'No changes.'), {
      label: `ux-player:c${cycle}`,
      phase: 'Iterate',
      schema: UX_SCHEMA,
    }),
    () => agent(productManagerPrompt(cycle, developResult || 'No changes.'), {
      label: `ux-pm:c${cycle}`,
      phase: 'Iterate',
      schema: UX_SCHEMA,
    }),
  ])

  // Update docs
  log(`Cycle ${cycle}: Updating documentation...`)
  const reviewSummary = [
    cppReview?.summary || 'No C++ review.',
    secReview?.summary || 'No security review.',
    archReview?.summary || 'No architecture review.',
  ].join('\n')

  const uxSummary = [
    playerUX?.summary || 'No player feedback.',
    pmUX?.summary || 'No PM feedback.',
  ].join('\n')

  await agent(updateDocsPrompt(cycle, auditSummary, testResult || '', reviewSummary, uxSummary), {
    label: `docs:c${cycle}`,
  })

  // Commit
  log(`Cycle ${cycle}: Committing changes...`)
  await agent(commitPrompt(cycle), { label: `commit:c${cycle}` })

  log(`\n✅ Cycle ${cycle}/10 complete.\n`)
}

log('\n' + '='.repeat(60))
log('🎉 All 10 cycles complete!')
log('='.repeat(60))
log('\nFinal deliverables:')
log('- ITERATION_LOG.md: Complete development journal')
log('- WORKFLOW_STATUS.md: Final status')
log('- CHANGELOG.md: All changes documented')
log('- PLAN.md: UX improvement roadmap')
log('- Code: All systems wired, tested, reviewed')
