# Constantum — Design

> A deterministic simulation evaluation harness: prove a result is *trustworthy*, not just "looks right".

**Status:** architecture for V1, revised 2026-09-01 after a review of the V0 kernel. Decision records
are appended to the relevant sections as the corresponding code lands.

## What it is

A **simulation-agnostic evaluation platform** that answers one question, over and over: *when a
simulation produces a result, how do we know that result is true?*

It is a library, not an application. A simulation author adds a dependency on Constantum, exposes the
simulation's state through one interface, and gets — without writing any of it — deterministic
record/replay across processes, first-divergence detection, a structural diff that names the exact
field that diverged, and CTest/CI gates that turn all of that into a build that goes red.

## Architecture

```
┌────────────────────────────────────────────────────────────┐
│  Constantum (this repo, header-only library)               │
│                                                            │
│  kernel:   Fixed (Q32.32) · SplitMix64 · Fnv1a64           │
│  state:    StateVisitor → Snapshot → Checksum / Diff       │
│  harness:  Recorder · Replayer · FirstDivergence           │
│  cli:      constantum::Main<Sim>(argc, argv)               │
│  cmake:    constantum_add_replay_gate() · _injection_gate()│
│  toy sim:  the smallest thing that satisfies Simulation,   │
│            used only to test the harness itself            │
└──────────────────────────┬─────────────────────────────────┘
                           │  Simulation concept (Reset / Step / Tick / Visit)
                           │  consumed via CMake FetchContent, pinned to a tag
          ┌────────────────┼────────────────┐
          ▼                ▼                ▼
   workload repo 1   workload repo 2   workload repo N
   (physics)         (order book, later)
```

**The platform boundary is a repository boundary.** Workloads live in their own repositories and
consume Constantum as a dependency. This is not an aesthetic choice: a harness that is only ever used
by simulations in its own tree has never had its "simulation-agnostic" claim tested. A workload repo
contains **zero harness code**; whatever a workload needs from the harness is built here, which is
also how this platform's roadmap is driven — by demand from a real workload, not by a feature list.

The one simulation that does live here is a deliberately trivial toy, present only so the harness's
own tests have something to run. It is not a reference simulation and makes no claims.

## Simulation interface

A simulation is a deterministic state machine stepped in fixed-size ticks. It exposes **one** thing
about its state — how to walk it — and everything else is derived.

```cpp
namespace constantum {

class StateVisitor {
public:
    virtual ~StateVisitor() = default;
    virtual void Begin(std::string_view entity, std::uint32_t index) = 0;
    virtual void Field(std::string_view name, Fixed value) = 0;
    virtual void Field(std::string_view name, std::int64_t value) = 0;
    virtual void End() = 0;
};

template <typename S>
concept Simulation = requires(S& s, const S& cs, std::uint64_t seed, StateVisitor& v) {
    { s.Reset(seed) } -> std::same_as<void>;
    { s.Step() } -> std::same_as<void>;
    { cs.Tick() } -> std::same_as<std::uint64_t>;
    { cs.Visit(v) } -> std::same_as<void>;
};

// Optional. Detected with `requires`, never required.
//   void Configure(const Options&);   // runtime switches, incl. failure injection
//   static constexpr std::string_view kName;

}  // namespace constantum
```

Design decisions behind this:

- **`Checksum()` and `Serialize()` are not on the interface** — deliberately. The V0 concept asked
  the simulation for a checksum, which invites hand-written per-field folding, which has no
  completeness guarantee: forget one field and the gate stays green while coverage silently drops.
  Instead the simulation walks its state once through `Visit`, and the harness derives the
  snapshot, the checksum and the diff from the *same* walk. Completeness is mechanical: a field that
  is not visited does not exist to the harness, and that absence is visible in the snapshot.
- **`Step()` takes no input.** V1 workloads are closed worlds (physics, orbital mechanics). When a
  scenario engine needs inputs, the extension is `Step(const Input&)`, not smuggling inputs through
  `Reset`.
- **`Configure` is optional** because most simulations do not need runtime switches, but failure
  injection does — and injection must be a runtime switch, not an `#ifdef`, so a single binary can
  run both the "normal path is green" test and the "injected path must be red" test.
- **Determinism rules the simulation author must follow** (the harness catches violations; it does
  not prevent them): no floating point in `Step`; no global or static mutable state; no iteration
  over unordered containers; RNG only via a `SplitMix64` seeded in `Reset`; no wall clock; no
  address-dependent ordering.

## State identity = canonical serialization

A **Snapshot** is the ordered sequence of `(path, raw value)` pairs produced by one `Visit`, where
`path` is `entity[index].field`. Everything is derived from it:

- **Checksum** — FNV-1a-64 over the snapshot's paths and values, in order. Order-sensitive on purpose.
- **Serialize** — the snapshot written as a tagged byte stream (paths are written; a reader never
  has to know the simulation's layout to interpret it).
- **Diff** — two snapshots walked side by side; the output is a list of `(path, expected, actual)`.

This is what the harness means by "state identity": not a number the simulation reports about
itself, but a representation the harness owns and the simulation cannot partially fill.

## Determinism gate = cross-process record → replay

Two objects in one process agreeing with each other is near-tautological: it catches uninitialized
memory and wall-clock reads, and stays green against everything that actually bites — ASLR,
address-dependent iteration order, heap-layout dependence, compiler and STL differences. So:

1. **`record`** runs the simulation in process A for T ticks and writes a **trace**: header
   (magic, format version, simulation name, seed, T, snapshot interval, provenance) + per-tick
   checksums + full snapshots every K ticks (K = 1 for small simulations).
2. **`replay`** runs the same simulation in process B from the trace's seed, compares its per-tick
   checksum against the recorded one, and stops at the **first divergence tick N**. It then diffs
   its own snapshot against the nearest recorded snapshot at or after N and prints the paths that
   differ. The harness reports N without knowing it in advance; that is the whole point.
3. **Cross-build** is the same protocol with process A and process B on different OS/compilers: the
   CI records on one runner, uploads the trace as an artifact, and replays it on another. Two
   self-consistent builds prove nothing; a trace crossing a build boundary does.

**Provenance in the trace header:** git commit, compiler id + version, OS, simulation name and the
simulation's own version string. "The number changed" must always be explainable.

### Tick numbering (fixed, so record and replay cannot be off by one)

- `Reset(seed)` produces **tick 0**. `checksum[0]` is the checksum of the state immediately after
  `Reset`, before any `Step`. It catches nondeterministic scene generation.
- After `t` calls to `Step`, `Tick()` returns `t` and `checksum[t]` is the checksum of that state.
- A trace with `ticks = T` contains `T + 1` checksums (0..T) and snapshots at every tick that is a
  multiple of `K` (tick 0 always included).

### Trace format (`.ctr`, "constantum trace")

Binary, **little-endian regardless of host**, all integers fixed-width. Strings are `u32 length` +
UTF-8 bytes, no terminator. The format is versioned; a reader rejects an unknown version.

| Section | Fields |
|---|---|
| Header | magic `"CTR1"` (4 bytes) · `u32 format_version = 1` · `string sim_name` · `string sim_version` · `u64 seed` · `u64 ticks` · `u32 snapshot_interval` · `string provenance` (free-form: commit, compiler, OS) · `string options` (the `Options` map, `k=v` joined by `;`, sorted by key) |
| Checksums | `u64 checksum[ticks + 1]` |
| Snapshots | `u32 count`, then `count` × Snapshot |
| Snapshot | `u64 tick` · `u32 field_count` · `field_count` × (`string path` · `u8 type_tag` · `i64 raw`) — `type_tag`: `0 = Fixed (Q32.32 raw)`, `1 = int64` |

Paths are `entity[index].field`, e.g. `bodies[7].vel.y`. The path is written for every field on every
snapshot: verbose, and deliberately so for V1 — a reader needs no schema, and a diff is a string
comparison. Compaction (a path table) is a LATER item and only if a workload's traces get too big.

### Options

`using Options = std::map<std::string, std::string>;` — flat, string-typed, parsed from the CLI as
`--opt key=value` (repeatable). Passed to the simulation's optional `Configure(const Options&)` after
construction and before `Reset`. The harness records the options in the trace header so a replay can
refuse to compare traces produced under different options — except when explicitly told to
(`--opt` on `replay` overrides, which is how injection tests replay a clean trace under a dirty
configuration).

### CLI contract (`constantum::Main<Sim>`)

```
<exe> record --seed S --ticks T --out run.ctr [--snapshot-interval K] [--opt k=v]...
<exe> replay --in run.ctr [--opt k=v]... [--report report.json]
<exe> run    --seed S --ticks T                     # prints final checksum; smoke use only
```

Exit codes — these are the contract CTest builds on: **0** = replay matched every tick · **1** =
divergence detected (N and the diff were printed to stdout / written to the report) · **2** = usage
or I/O error (bad args, unreadable trace, version mismatch) · **3** = the simulation threw.
`constantum_add_injection_gate` marks the replay `WILL_FAIL`, so exit 1 is the required outcome and
exit 0, 2 or 3 all fail the gate — a broken injection cannot pass as a caught one.

### The toy simulation (harness self-test only)

`Toy`: one entity, fields `counter` (int64) and `draw` (Fixed), stepped as `counter += 1; draw = next
RNG value as Fixed raw`. Two options: `toy.leak_address=1` mixes `reinterpret_cast<uintptr_t>(this)`
into `draw` on every step — a nondeterminism source that is stable within a process and differs
across processes under ASLR, which is exactly the class of bug the gate exists for; `toy.leak_at=N`
makes the leak start at tick N so the "harness reports N without knowing it" behaviour is tested
against a known answer. `Toy` lets every harness feature be developed and gated **without any
workload repo present**. It makes no claims and is not a reference simulation.

### Versioning and the workload pin

SemVer tags. While a workload and the harness are evolving together, the workload pins a **commit SHA**
(`FetchContent` accepts one); a tag is cut when the interface is stable for that workload's story. Interface changes to the
`Simulation` concept or the trace format bump the minor version pre-1.0 and are listed in
`CHANGELOG.md`.

## CTest / CI integration

The harness ships CMake functions so a workload gets its gates in one line each:

```cmake
constantum_add_replay_gate(NAME stack20 TARGET my_sim SEED 7 TICKS 1000)
#   -> tests stack20.record (FIXTURES_SETUP) and stack20.replay (FIXTURES_REQUIRED)
#      CTest runs each test as its own process, so cross-process is guaranteed by construction.

constantum_add_injection_gate(NAME stack20_contact_order TARGET my_sim SEED 7 TICKS 1000
                              OPTIONS contact_order=by_address)
#   -> same pair, replay marked WILL_FAIL: the build is red if the injected bug is NOT caught.
```

Cross-build comparison is a CI pattern (record job → artifact → replay job on another OS), documented
in `.github/workflows/` once implemented.

## Kernel

- **`Fixed` — Q32.32, `int64_t` storage, 128-bit intermediates.**
  - *Multiply / divide:* a **portable reference path** written on two `uint64_t` halves (schoolbook
    128-bit multiply, 128-by-64 long division) is the definition of correctness and always compiles
    everywhere. Fast paths (`__int128` on GCC/Clang, `_mul128` / `_umul128` on MSVC) are optional
    and a test asserts fast path == reference path over a large seeded sample on every compiler.
    `_div128` is **not** relied on (availability varies by MSVC version); division uses the portable
    path unless a fast path is proven equal.
  - *Rounding:* **one rule — floor** (toward −∞) for multiply, divide and sqrt. Arithmetic right
    shift of a negative value is defined in C++20 as floor; division must be adjusted from the
    hardware's truncation to floor explicitly, with a test on negative operands.
  - *`Sqrt`:* `floor(sqrt(x))` in Q32.32, computed as the integer square root of the 96-bit value
    `raw << 32`, digit-by-digit on the two-`uint64_t` pair. Portable, no intrinsics, full 32
    fractional bits. Negative input is a precondition violation (assert in debug, returns 0 in
    release — documented).
  - *Overflow policy:* **wrapping in release, asserted in debug.** All internal arithmetic is
    performed on `uint64_t` and cast back (two's-complement wrap is defined behaviour; signed
    overflow is not, and undefined behaviour is exactly where compilers diverge). Debug builds
    additionally check for overflow with `__builtin_*_overflow` / manual checks and assert.
    Saturation was rejected: it silently changes simulation semantics at the boundary, which is
    worse for a verification harness than a loud debug failure and a defined wrap.
  - `FromFloat` / `ToFloat` exist for tests and display only; simulation code never touches
    floating point. `FromInt`, `Abs`, `Min`, `Max`, unary minus, `kZero`, `kOne`, `ToString`
    (raw hex + decimal approximation, for diffs) are part of the kernel.

  **Decision record — Q32.32 (to be appended with the kernel commit):** why Q16.16 was retired, why
  controlled floating point was rejected, the range and precision the first workload requires.

- **`SplitMix64`** — seeded, no global state. Tested against the reference implementation's known
  output vector, not only against itself.
- **`Fnv1a64`** — order-sensitive mixing; `MixBytes(const void*, size_t)` for paths, `Mix(uint64)`
  for values.

## Stages (contracted 2026-08-23; unchanged)

- **V1 — first failure story.** Kernel at Q32.32 · `StateVisitor` + snapshot/checksum/diff ·
  record/replay CLI · first-divergence · CMake gates · cross-build CI. **Done when a real failure
  exists in a real workload, is caught cross-process, localized to a field by the diff, fixed, and
  locked in as a regression gate whose injected variant is proven red.** Not done when the feature
  table is filled.
- **NEXT (evidence-driven):** trajectory invariants (a workload asserts energy / penetration /
  joint-error bounds per tick and the harness reports the first violation — the second class of
  failure, which determinism alone cannot see); declarative scenarios; provenance beyond the header.
- **LATER (hypotheses):** input-space coverage, differential oracle, metrics export, multi-workload
  reports. None of these is built until a workload needs it.

## Defaults

Snapshot interval `K = 1` (every tick) — correct for the workload sizes V1 targets; workloads override
per gate.

## Anti-bloat rule

A stage is "done" when its bar is met — then stop; no gold-plating. Minimal interfaces and clear
boundaries are what keep it readable and extensible, not features. The Simulation concept has four
methods; a proposal to add a fifth needs a workload that cannot be expressed without it.

## Standard of ownership

Every line in this repository is one its author can derive and defend from first principles; anything
that cannot be explained line by line is rewritten until it can. Decision records in this document are
written to that standard: they state the alternative that was rejected and why.
