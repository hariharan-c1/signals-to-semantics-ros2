# Testing Strategy

**Status:** M1 / v0.1 is Implemented / Accepted. Local macOS build and runtime
verification passed, including the M1.4 automated installed-process integration
test. GitHub Actions `ROS 2 foundation` passed on Ubuntu 24.04, including the
C++ publisher → Python subscriber integration test.

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

The completed v0.1 milestone is limited to:

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

**Status:** **Implemented and verified on macOS and Ubuntu 24.04 CI**

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

The implementation is
[`test_ego_state_integration.py`](../sts_ws/src/sts_contract_subscriber_py/test/test_ego_state_integration.py).
The PASS timeout is 30 seconds. The fixture uses a shared test-only ROS domain,
temporary node logs, and bounded shutdown escalation. It captures the subscriber's
stderr with ROS logging explicitly directed there. The macOS-only child-process
library-path restoration uses the installed interface package prefix; Linux uses
the normal workspace environment. No canonical field validation is copied into
the harness.

## M1.4 local validation

On macOS arm64, using Pixi 0.81.0, `launch_pytest` 3.4.11, and pytest 8.4.2:

- A fresh build of all three M1 packages succeeded in separate ignored output
  directories.
- Both canonical validation unit tests and all three Python lint checks passed.
- The real installed C++/Python integration test passed. The Python suite reported
  6 passed; the verbose colcon result summary reported 23 tests, 0 errors,
  0 failures, and 1 skipped (the existing cppcheck tooling skip).
- A temporary negative test copy suppressed the real subscriber's INFO logs and
  shortened the wait to 3 seconds. It failed with the missing-PASS assertion and
  exit code 1 after 3.27 seconds, and both real processes shut down. This temporary
  validation did not add a repository test or replace either executable.
- `pixi lock --check --offline` passed for the dual-platform lockfile. The Linux
  resolution contains 752 packages and the macOS resolution 704; their references
  were checked against lock records and platform/noarch URLs. This establishes
  dependency resolution, not Linux installation or runtime behavior.

Jazzy's installed `launch_testing` pytest plugin fails to load with pytest 9.
Pixi therefore constrains pytest to `>=8.1,<9`. The package uses the supported
setuptools `test` extra so colcon selects pytest and discovers the launch test.
DDS validation required local network access
outside the execution sandbox; launch logs used a writable temporary directory.

Fresh-build validation commands, run from `sts_ws/`:

```sh
pixi install --locked
pixi run --locked colcon --log-base log/m1_4 build --build-base build/m1_4 --install-base install/m1_4 --packages-select sts_interfaces sts_contract_publisher_cpp sts_contract_subscriber_py
ROS_LOG_DIR=/tmp/sts_m1_4_ros_logs pixi run --locked bash -c 'set -e; source install/m1_4/setup.bash; colcon --log-base log/m1_4 test --build-base build/m1_4 --install-base install/m1_4 --packages-select sts_interfaces sts_contract_publisher_cpp sts_contract_subscriber_py --return-code-on-test-failure --pytest-args -v'
pixi run --locked colcon test-result --test-result-base build/m1_4 --verbose
pixi lock --check --offline
```

### Known tooling warning

For M1, `launch_pytest` 3.4.11 on Python 3.12 emits
`DeprecationWarning: There is no current event loop`. A diagnostic traceback
identified `launch_pytest/fixture.py` calling `policy.get_event_loop()` before
setting its newly created loop with `policy.set_event_loop(loop)`. The warning
originates in the dependency stack, not project test code. Normal M1 integration
execution passes. The warning is not suppressed or patched locally. Re-evaluate
when the ROS Jazzy testing stack or Python version changes.

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

## M1.4 Ubuntu CI verification

The GitHub Actions **ROS 2 foundation** workflow passed on **Ubuntu 24.04**.
Linux CI built `sts_interfaces`, `sts_contract_publisher_cpp`, and
`sts_contract_subscriber_py`, reporting **3 packages finished**. Its final test
summary was **23 tests, 0 errors, 0 failures, 1 skipped**. The skip is the
previously documented cppcheck tooling skip. The automated `launch_pytest` test
launched the real installed C++ publisher and Python subscriber and passed by
observing `PASS canonical EgoState payload` within the bounded timeout.

This evidence completes M1's Linux foundation verification. The known dependency
event-loop warning above remains documented; it was not fixed or suppressed.

## CI evolution

The implemented M1.4 baseline runs Linux CI using GitHub Actions. Pixi retains
`osx-arm64` and adds `linux-64`; the tracked `pixi.lock` resolves both platforms.
CI uses that repository lockfile without re-resolving dependencies, builds the ROS
workspace, activates its installed overlay, runs package tests, and runs the
`launch_pytest` integration test. Build and test failures, including a missing
subscriber PASS within the bounded timeout, fail CI.

This baseline uses no CARLA, Docker, GPU perception, or S2-S7. It is a
ROS-foundation portability/regression check, not proof of production Linux/CARLA
compatibility. The workflow, automated integration test, Pixi platform addition,
and regenerated lockfile are implemented. The workflow uses GitHub-hosted Ubuntu
24.04, `actions/checkout@v7`, `prefix-dev/setup-pixi@v0.10.0`, and Pixi `v0.81.0`.
Locked installation and runs enforce the committed lockfile; caching is enabled.
The job has a 30-minute timeout, and test results are reported verbosely even after
failure. The successful GitHub-hosted Linux run is recorded above.

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
