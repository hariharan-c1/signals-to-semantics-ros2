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

The approved architecture names several planned custom messages but does not choose
the first one or specify its fields. Before package generation, M1 must select one
message and approve its complete contract in
[`../ROS_INTERFACE_SPEC.md`](../ROS_INTERFACE_SPEC.md), including field meanings,
types, units, timestamp and frame behavior, validity rules, and cross-language test
examples. Implementation must not guess this schema.

## Required deliverables

- Documented ROS 2 Jazzy environment assumptions for the supported build host.
- A minimal colcon workspace containing only the packages required for this scope.
- `sts_interfaces` with exactly the first approved custom message and required
  build metadata.
- One minimal `rclcpp` node and one minimal `rclpy` node that exchange that message.
- Basic package-level tests and one cross-language communication test.
- A CI workflow that builds and runs the v0.1 test scope.
- Updated status documentation that reports only verified behavior as implemented.

Final package and executable names beyond `sts_interfaces`, message direction, and
the message schema are decisions for the M1 design/specification step; they are not
pre-approved by this task.

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
