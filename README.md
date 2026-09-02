# Constantum

> A deterministic simulation evaluation harness: prove a result is *trustworthy*, not just "looks right".

Constantum is a flagship personal project: a **simulation-agnostic evaluation platform** that answers one
question, over and over — *when a simulation produces a result, how do we know that result is true?*

It is not a game, not a toy. It is a platform: a minimal simulation interface, and a CI-enforced loop of
**determinism → coverage → provenance → verdict** around whatever simulation plugs in.

## Why determinism first

A simulation that is not deterministic cannot be evaluated: a "regression" might just be noise. Constantum
treats determinism as a first-class invariant, proven by CI (run the same scenario twice, compare
bit-identical per-tick checksums), not claimed.

## Status

**V1 in progress — rebuilt from scratch by the owner** (see "Ownership" in
[`docs/DESIGN.md`](docs/DESIGN.md)). Kernel primitives exist and are tested on GCC and MSVC: seeded
RNG (`SplitMix64`), order-sensitive checksum (`Fnv1a64`), fixed-point `Fixed`, and the `Simulation`
concept. Not yet present: the state visitor, record/replay, first-divergence detection, the CMake
gates. The architecture for all of it is in `docs/DESIGN.md`.

| Stage | Goal | Status |
|---|---|---|
| V1 | first failure story: Q32.32 kernel · state identity · cross-process record/replay · first divergence · CI gates | in progress |
| NEXT | trajectory invariants · declarative scenarios · provenance | when a workload demands it |
| LATER | coverage · differential oracle · metrics export | hypotheses |

## Build & test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Layout

```
include/constantum/   platform headers (fixed, rng, checksum, sim concept; state/harness/cli to come)
tests/                kernel tests and, later, the harness's own gates on a trivial toy simulation
docs/DESIGN.md        architecture and decision records
```

Reference simulations do **not** live here. They are separate repositories that consume Constantum as a
dependency — that boundary is the platform claim.
