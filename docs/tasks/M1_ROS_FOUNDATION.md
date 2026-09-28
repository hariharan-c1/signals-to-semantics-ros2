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
- Contract status: **Approved / not yet implemented**
- QoS: Reliable, Keep Last, depth 10, Volatile

The complete frozen schema, field meanings, types, units, timestamp and frame
behavior, validity rules, exclusions, and canonical cross-language test payload are
recorded in [`../ROS_INTERFACE_SPEC.md`](../ROS_INTERFACE_SPEC.md). The QoS contract
is also recorded in [`../QOS.md`](../QOS.md). Package generation and implementation
must use these approved contracts without guessing or silently changing them.

Passing this documentation gate is not implementation evidence. `EgoState v1` must
remain **Approved / not yet implemented** until the `.msg` file is generated, built,
and tested across C++ and Python.

## Required deliverables

- Documented ROS 2 Jazzy environment assumptions for the supported build host.
- A minimal colcon workspace containing only the packages required for this scope.
- `sts_interfaces` with exactly the first approved custom message and required
  build metadata.
- One minimal `rclcpp` node and one minimal `rclpy` node that exchange that message.
- Basic package-level tests and one cross-language communication test.
- A CI workflow that builds and runs the v0.1 test scope.
- Updated status documentation that reports only verified behavior as implemented.

Final package and executable names beyond `sts_interfaces` and message direction
remain decisions for the M1 implementation step. The `EgoState v1` schema, topic,
semantics, and QoS are no longer open decisions and must match the approved
contracts.

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
