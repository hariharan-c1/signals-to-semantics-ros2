# M1 — ROS 2 Foundation

**Status:** In Progress  
**Release:** v0.1 ROS 2 Foundation  
**Architecture authority:** [`../DESIGN_SESSION_0_V2.md`](../DESIGN_SESSION_0_V2.md)

## Objective

Establish the smallest verified ROS 2 foundation that proves a custom
Signals-to-Semantics interface can be generated and communicated correctly between
C++ and Python.

## Scope

M1 is limited to:

1. ROS 2 Jazzy environment.
2. A colcon workspace.
3. The `sts_interfaces` package.
4. The first approved custom message.
5. One C++ ROS node.
6. One Python ROS node.
7. Cross-language communication through the custom interface.
8. Basic tests.
9. A CI baseline.

## Design gate: first custom message

**Decision:** The interface-contract selection gate has passed. `EgoState v1` is the
first approved custom message for M1.

- Topic: `/sts/ego/state`
- Message type: `sts_interfaces/EgoState`
- Contract status: **Implemented and runtime verified in M1.3B**
- QoS: Reliable, Keep Last, depth 10, Volatile

The complete frozen schema, field meanings, types, units, timestamp and frame
behavior, validity rules, exclusions, and canonical cross-language test payload are
recorded in [`../ROS_INTERFACE_SPEC.md`](../ROS_INTERFACE_SPEC.md). The QoS contract
is also recorded in [`../QOS.md`](../QOS.md). Package generation and implementation
must use these approved contracts without guessing or silently changing them.

The `.msg` file has been generated and built, and C++ to Python communication has
been runtime verified in M1.3B. Accepted evidence is recorded in
[`../engineering-notes/M1_ROS2_FOUNDATION.md`](../engineering-notes/M1_ROS2_FOUNDATION.md).
The interface and QoS documents retain earlier implementation-status labels;
their approved contracts remain unchanged.

## M1.3B cross-language communication design

**Status:** **Implemented and runtime verified**

M1.3B fixes the following design for the M1 contract-verification exchange:

| Contract item | Approved value |
| --- | --- |
| Direction | C++ publisher → Python subscriber |
| C++ package | `sts_contract_publisher_cpp` |
| C++ executable/node purpose | Publish the canonical `sts_interfaces/EgoState` test payload. |
| Python package | `sts_contract_subscriber_py` |
| Python node purpose | Subscribe to `/sts/ego/state` and validate the canonical payload. |
| Topic | `/sts/ego/state` |
| Message type | `sts_interfaces/EgoState` |
| QoS | Reliable, Keep Last, depth 10, Volatile |
| Publication period | 1 second for the M1 manual demonstration |

The canonical payload remains:

```ini
frame_id = "base_link"
longitudinal_velocity_mps = 13.5
longitudinal_acceleration_mps2 = -2.25
longitudinal_jerk_mps3 = -4.0
velocity_valid = true
acceleration_valid = true
jerk_valid = true
```

These packages are M1 contract-verification support components only. They are not
the future production `sts_ego_state_cpp` implementation. M1.3B includes no
odometry processing, kinematic derivation, TF2 integration, or scenario logic.

All three M1 packages built, and package checks reported 17 tests, 0 errors,
0 failures, and 1 tooling skip. The manual runtime check produced repeated Python
subscriber `PASS canonical EgoState payload` logs. This accepted macOS evidence is
recorded in the linked engineering reference; automated cross-process integration
testing and Linux CI remain to be implemented in M1.4.

## M1.4 automated integration test and Linux CI design

**Decision status:** **Approved / not yet implemented**

M1.4 adds an automated cross-language integration test using ROS 2 Jazzy
`launch_pytest`. It must launch the actual installed executables from the built
workspace:

| Package | Installed executable | Role |
| --- | --- | --- |
| `sts_contract_publisher_cpp` | `ego_state_test_publisher` | Publish the canonical payload in C++. |
| `sts_contract_subscriber_py` | `ego_state_test_subscriber` | Receive and validate the payload in Python. |

The test succeeds only when output captured from the subscriber process contains
`PASS canonical EgoState payload` within an explicit, bounded timeout. If that PASS
is not observed before the timeout, the test must fail, including when either
process cannot start, no message arrives, or received payloads fail validation.
Process startup or clean exit alone cannot satisfy the test.

The exchange must use the existing `/sts/ego/state`, `sts_interfaces/EgoState`, and
Reliable, Keep Last, depth 10, Volatile QoS contract. Canonical field-level
validation remains the Python subscriber's responsibility, including the existing
frame, timestamp, numeric-field, and validity-flag checks. The integration harness
observes the subscriber's PASS output and must not unnecessarily duplicate that
validation logic or substitute mock processes for the installed executables.

M1.4 also adds Linux CI using GitHub Actions. During implementation:

- Retain `osx-arm64` in `pixi.toml` and add `linux-64`.
- Update the repository `pixi.lock` to resolve both platforms. CI must use that
  tracked lockfile without re-resolving dependencies during the run.
- Build the ROS workspace in the locked Linux Pixi environment, activate the built
  workspace overlay, run package tests, and run the automated integration test.
- Propagate build, package-test, and integration-test failures to the CI result,
  including failure to observe the required subscriber PASS within the timeout.

CI uses no CARLA, Docker, GPU perception, or S2-S7. It is a ROS-foundation
portability/regression check; passing it does not prove production Linux/CARLA
compatibility.

This records the approved design only. The integration test, test dependencies,
GitHub Actions workflow, Pixi platform addition, and lockfile update remain
unimplemented by this documentation change. The overall M1 milestone remains
**In Progress**.

## Required deliverables

- Documented ROS 2 Jazzy environment assumptions for the supported build host.
- A minimal colcon workspace containing only the packages required for this scope.
- `sts_interfaces` with exactly the first approved custom message and required
  build metadata.
- One minimal `rclcpp` node and one minimal `rclpy` node that exchange that message.
- Basic package-level tests and one cross-language communication test.
- A CI workflow that builds and runs the v0.1 test scope.
- Updated status documentation that reports only verified behavior as implemented.

The M1.3B package names, node roles, message direction, publication period,
`EgoState v1` schema, topic, semantics, and QoS are approved decisions and must
match the recorded contracts during implementation.

## Acceptance criteria

M1 is accepted only when:

1. The selected environment is ROS 2 Jazzy and its supported setup is documented.
2. The colcon workspace builds cleanly from a documented clean state.
3. `sts_interfaces` generates the approved first custom message without adding
   unrelated interfaces.
4. The exact message contract is documented before or with implementation.
5. A C++ ROS node and a Python ROS node successfully exchange the generated message.
6. An automated integration test verifies cross-language payload fidelity rather
   than only checking that both processes start.
7. Basic C++ and Python tests pass through the workspace test command.
8. The CI baseline runs the approved build and tests and reports failure correctly.
9. No additional roadmap packages or runtime capabilities are represented as
   implemented.
10. Relevant README, milestone, interface, and test status is updated with evidence.

## Out of scope

M1 does not implement streaming ego kinematics, risk metrics, event detection,
scenario-window logic, TF2 integration, rosbag2 workflows, perception, tracking,
CARLA, S2-S7 integration, RViz2 features, PostgreSQL/pgvector, Docker, model
checkpoints, or datasets.

It also does not finalize all custom message schemas, all topic QoS profiles, the
full package graph, or production deployment behavior.
