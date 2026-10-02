# ADR-001: Use ROS 2 Jazzy

- **Status:** Accepted
- **Decision scope:** ROS foundation
- **Implementation status:** Implemented / Accepted in v0.1; see [M1 evidence](../tasks/M1_ROS_FOUNDATION.md)
- **Authority:** [`../DESIGN_SESSION_0_V2.md`](../DESIGN_SESSION_0_V2.md)

## Context

The ROS 2 extension needs one defined distribution for package metadata, interfaces,
builds, tests, and CI. The approved v0.1 roadmap selects ROS 2 Jazzy.

## Decision

Use ROS 2 Jazzy as the project foundation. The v0.1 environment, colcon workspace,
C++ and Python packages, generated interfaces, tests, and CI baseline will target
Jazzy.

## Consequences

- M1 documentation must state the supported Jazzy environment assumptions.
- `rclcpp` and `rclpy` code will be verified against the same ROS distribution.
- Interface generation and cross-language tests must pass in the Jazzy workspace.
- Supporting another distribution is not part of v0.1 and requires explicit review.
- Selecting Jazzy does not imply that the environment or packages already exist.
