# Testing Strategy

**Status:** M1.3B is Implemented and runtime verified on macOS, with package checks
and a manual cross-language runtime check. M1.4 automated integration testing and
Linux CI are Approved / not yet implemented; v0.1 remains In Progress.

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

## M1.3B verification evidence

The accepted M1.3B verification built all three foundation packages. Package checks
reported 17 tests, 0 errors, 0 failures, and 1 tooling skip. The Python tests cover
canonical payload acceptance and incorrect-field rejection. A separate manual
runtime check of the C++ publisher and Python subscriber produced repeated
`PASS canonical EgoState payload` logs. See
[`engineering-notes/M1_ROS2_FOUNDATION.md`](engineering-notes/M1_ROS2_FOUNDATION.md)
for the evidence and its limits. This verification did not establish automated
cross-process testing or Linux compatibility.

## M1.4 approved integration-test design

**Status:** **Approved / not yet implemented**

Use ROS 2 Jazzy `launch_pytest` to launch the actual installed
`sts_contract_publisher_cpp/ego_state_test_publisher` C++ executable and
`sts_contract_subscriber_py/ego_state_test_subscriber` Python executable from the
built workspace. The harness must observe `PASS canonical EgoState payload` in the
subscriber process output within an explicit, bounded timeout. Absence of that PASS
must fail the test; successful startup or exit is insufficient.

The test uses the existing `/sts/ego/state` topic, `sts_interfaces/EgoState` message,
and Reliable, Keep Last, depth 10, Volatile QoS contract. The Python subscriber
remains responsible for field-level canonical validation, including its existing
frame, timestamp, numeric-field, and validity-flag checks. The harness checks the
subscriber's PASS output without unnecessarily duplicating payload validation.
The full approved design is in
[`tasks/M1_ROS_FOUNDATION.md`](tasks/M1_ROS_FOUNDATION.md).

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

The approved M1.4 baseline adds Linux CI using GitHub Actions. Implementation must
retain `osx-arm64` and add `linux-64` in Pixi, update the tracked `pixi.lock` for both
platforms, and use that repository lockfile in CI without re-resolving dependencies.
CI must build the ROS workspace, activate its installed overlay, run package tests,
and run the `launch_pytest` integration test. Build and test failures, including a
missing subscriber PASS within the bounded timeout, must fail CI.

This baseline uses no CARLA, Docker, GPU perception, or S2-S7. It is a
ROS-foundation portability/regression check, not proof of production Linux/CARLA
compatibility. The workflow, automated integration test, Pixi platform addition,
and lockfile update remain unimplemented by this documentation change.

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
