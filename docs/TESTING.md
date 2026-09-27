# Testing Strategy

**Status:** Test architecture planned; v0.1 basic tests and CI baseline are In
Progress. No runtime test suite exists yet.

## Acceptance philosophy

Tests must establish behavior, not only process existence. “The node starts” is
insufficient where a deterministic result can be asserted. For example, a known
constant-deceleration input should produce the expected acceleration within a
defined numerical tolerance.

Every test should identify its input, expected output, tolerance or invariant,
clock assumptions, frame assumptions, and applicable operating mode.

## Test layers

1. **Unit tests** — pure calculations, adapters, validation rules, and failure
   behavior.
2. **ROS component tests** — a package or node with its ROS-facing contract.
3. **ROS integration tests** — communication and behavior across packages and
   languages.
4. **rosbag regression tests** — reproducible processing of short selected recorded
   scenarios.
5. **CARLA scenario tests** — controlled GT and perception-mode scenarios on the
   future Linux/NVIDIA environment.
6. **AI regression tests** — S2/S3 outputs and deterministic evidence behavior
   against approved fixtures.
7. **End-to-end tests** — source through visualization/storage/retrieval for the
   delivered release scope.
8. **GT-vs-perception comparison** — downstream behavior with equivalent scenes
   supplied by the two actor providers.

## v0.1 test boundary

The current milestone is limited to:

- interface generation/build tests for the first approved custom message;
- basic C++ node tests;
- basic Python node tests;
- a cross-language communication test that verifies payload transfer through the
  generated interface;
- a clean colcon workspace build and test run; and
- a CI baseline that runs the approved build and tests.

Risk equations, TF2, rosbag regression, perception, CARLA, ML, database, and
end-to-end semantic tests belong to later milestones.

## Future contract-focused checks

- Frame and timestamp correctness, including missing or stale transforms.
- Valid, invalid, unavailable, and non-applicable TTC behavior.
- Deterministic replay of short selected scenarios.
- Causal online processing with no future-data leakage.
- Offline-parity regression against thesis behavior.
- QoS compatibility, overload behavior, and important-output delivery.
- LLM timeout/failure isolation from physical processing.
- S2/S3 regression and domain-shift evaluation.
- GT/perception equivalence at the normalized actor boundary.

## CI evolution

CI is planned to grow from v0.1 build and basic tests toward formatting, C++ checks,
Python checks, workspace builds, unit tests, integration smoke tests,
interface-contract tests, and eventually Docker builds. CARLA/GPU scenarios should
not run on every ordinary commit; later they may use manual workflows, release
validation, or GPU runners.

## Test-data policy

Use synthetic inputs or short, selected, compressed recordings with storage limits.
Do not commit large bags, full datasets, database volumes, or large generated
artifacts. Test fixtures must be safe for a public repository and small enough for
the local Apple M1 Pro development constraint.
