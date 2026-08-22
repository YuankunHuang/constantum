# Plumbline — Design

> A deterministic simulation evaluation harness: prove a result is *trustworthy*, not just "looks right".

## What it is

A **simulation-agnostic evaluation platform** that answers one question, over and over: *when a
simulation produces a result, how do we know that result is true?*

Not a game, not a toy. A platform: a minimal simulation interface, and a CI-enforced loop of
**determinism → coverage → provenance → verdict** around whatever simulation plugs in.

## Architecture

```
┌─────────────────────────────────────────────┐
│            Evaluation Harness (platform)    │
│  scenario runner · determinism gate ·        │
│  coverage · provenance · metrics · oracle    │
└────────────────────┬────────────────────────┘
                     │  Simulation interface
                     │  (reset / step / tick / checksum / serialize)
          ┌──────────┴──────────┐
          ▼                     ▼
  Reference sim 1        Reference sim 2
  deterministic           minimal toy sim
  multi-agent world       (proves reusability)
  (the real one)
```

The harness is simulation-agnostic: it does not know or care which simulation is plugged in. That is
what makes it a *platform*, and it is proven by having ≥2 simulations plugged in (reusability), not by
a fancy plugin architecture.

## Simulation interface

A simulation is a deterministic state machine stepped in fixed-size ticks. The contract is deliberately
minimal — anything more, and adding a new simulation stops being "an hour of work".

```cpp
template <typename S>
concept Simulation = requires(S& s, std::uint64_t seed) {
    { s.reset(seed) } -> std::same_as<void>;
    { s.step() } -> std::same_as<void>;
    { s.tick() } -> std::same_as<std::uint64_t>;
    { s.checksum() } -> std::same_as<std::uint64_t>;
};
```

## Why determinism first

A simulation that is not deterministic cannot be evaluated: a "regression" might just be noise.
Plumbline treats determinism as a **first-class invariant, proven by CI** (run the same scenario twice,
compare bit-identical per-tick checksums), not claimed. Foundations:

- **Fixed-point math** (Q16.16) for simulation state — floating point is not bit-stable across
  platforms/compilers.
- **Explicitly seeded RNG** (SplitMix64) with no global state.
- **No unordered iteration** over hash containers in simulation logic.
- **Deterministic, order-sensitive state checksums** (FNV-1a mixing).

## Components and their excellence bars

| Component | Responsibility | Bar |
|---|---|---|
| 1. Determinism kernel | fixed-point + seeded RNG + no globals + no unordered iteration | determinism is a first-class invariant, proven by CI |
| 2. Simulation interface | reset / step / tick / checksum / serialize | minimal enough that a new sim takes ~1 hour to add |
| 3. Scenario engine | declarative scenarios (YAML/small DSL) + generic runner | scenarios are data, not code |
| 4. Determinism gate | record/replay + per-tick checksum | runs in CI on every push; non-determinism = red |
| 5. Coverage | input-space fuzzing + coverage report | "coverage" = state-space regions, not line coverage |
| 6. Provenance | commit + seed + scenario version + oracle version on every result | "the number changed" is always explainable |
| 7. Differential oracle | fixed-point reference cross-checked against main impl | disagreement in CI = failure |
| 8. Metrics export | structured JSON + documented schema + CLI | consumable by external tools |
| 9. Docs + CLI | run / replay / coverage / verify + README + design doc | `git clone && cmake && ctest` green on a clean machine |

## Roadmap

| Stage | Goal | Done when |
|---|---|---|
| V0 | scaffold + determinism gate in CI | a minimal sim's determinism gate is green in CI |
| V1 | fixed-point kernel, Simulation interface, multi-agent world | multi-agent sim provably deterministic under CI; adding a 2nd sim is trivial |
| V2 | declarative scenarios + provenance | a scenario corpus runs; every result traceable |
| V3 | coverage + differential oracle | coverage report + oracle gate in CI |
| V4 | metrics export + second sim + docs | the platform demonstrably evaluates ≥2 sims; docs excellent |

## Naming

The name is a plumb line: the tool a builder uses to establish what is true vertical. The tagline is the
explanation; the substance is the proof. (Name = hook, tagline = explanation, substance = proof.)

## Anti-bloat rule

A stage is "done" when its bar is met — then stop; no gold-plating. "Big" means nine excellent
components, not overbuilding each one. Minimal interfaces and clear boundaries are what keep it elegant,
readable, and extensible.

## Ownership

This is a flagship portfolio project. The owner can **re-derive and defend every line**; anything that
cannot be explained line-by-line is rewritten. That is what makes it a credibility artifact rather than
an AI-generated toy.
