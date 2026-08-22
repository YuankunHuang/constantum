# Plumbline

> A deterministic simulation evaluation harness: prove a result is *trustworthy*, not just "looks right".

Plumbline is a flagship personal project: a **simulation-agnostic evaluation platform** that answers one
question, over and over — *when a simulation produces a result, how do we know that result is true?*

It is not a game, not a toy. It is a platform: a minimal simulation interface, and a CI-enforced loop of
**determinism → coverage → provenance → verdict** around whatever simulation plugs in.

## Why determinism first

A simulation that is not deterministic cannot be evaluated: a "regression" might just be noise. Plumbline
treats determinism as a first-class invariant, proven by CI (run the same scenario twice, compare
bit-identical per-tick checksums), not claimed.

## Status

**V0 — scaffold.** Repo, CMake, CI, clang-format, and a determinism smoke test: a seeded deterministic
wanderer sim, run twice, must produce identical checksums. The gate runs in CI on every push.

| Stage | Goal | Status |
|---|---|---|
| V0 | scaffold + determinism gate in CI | in progress |
| V1 | fixed-point kernel, Simulation interface, multi-agent world | next |
| V2 | declarative scenarios + provenance | |
| V3 | coverage + differential oracle | |
| V4 | metrics export + second sim (reusability proof) + docs | |

## Build & test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Layout

```
include/plumbline/    core platform headers (checksum, rng, sim concept)
examples/             reference simulations that plug into the harness
tests/                CI-enforced determinism gates
```

---

*The name is a plumb line: the tool a builder uses to establish what is true vertical.*
