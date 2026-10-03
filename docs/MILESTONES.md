# Milestones

**Project status:** Active Development  
**Current milestone:** v0.2 Streaming Vehicle Core — In Progress\
**Next planned milestone:** v0.3 Risk Intelligence

The milestone states distinguish repository evidence from architectural intent.

## Implemented

### M0.1 — Repository Foundation

Documentation, governance, architecture derivatives, ADRs, task specifications,
license, and ignore policy are established. This milestone intentionally contains
no production source code, ROS packages, Docker files, checkpoints, or datasets.

Acceptance criteria are defined in
[`tasks/M0_REPO_FOUNDATION.md`](tasks/M0_REPO_FOUNDATION.md).

### v0.1 — ROS 2 Foundation — Implemented

Implemented and accepted: ROS 2 Jazzy environment, colcon workspace, the approved
`EgoState v1` custom interface in `sts_interfaces`, C++ publisher in
`sts_contract_publisher_cpp`, Python subscriber in `sts_contract_subscriber_py`,
basic package tests, and an automated installed-process cross-language integration
test. Local macOS build/runtime verification and GitHub Actions Ubuntu 24.04 CI
passed. Linux CI reported **3 packages finished** and **23 tests, 0 errors,
0 failures, 1 skipped** (the documented cppcheck tooling skip).

The execution contract is
[`tasks/M1_ROS_FOUNDATION.md`](tasks/M1_ROS_FOUNDATION.md).

## In Progress

### v0.2 — Streaming Vehicle Core — In Progress

M2.0/M2.1 established the Odometry-to-EgoState specification, and M2.2A approved
the package/API design. M2.2B `sts_ego_state_cpp` is implemented locally with
`EgoKinematicsEstimator` and `EgoStateNode`; selected-package build and estimator
unit tests have passed. ROS end-to-end runtime/integration verification remains
pending. Event detection and rolling temporal state remain Planned and are not
implemented. Local unit evidence does not complete v0.2 acceptance.

The execution contract is
[`tasks/M2_STREAMING_VEHICLE_CORE.md`](tasks/M2_STREAMING_VEHICLE_CORE.md), with the
[M2 engineering note](engineering-notes/M2_STREAMING_VEHICLE_CORE.md).

## Planned

| Release | Outcome |
| --- | --- |
| v0.3 | Actor-relative risk intelligence and initial RViz2 visualization |
| v0.4 | TF2, rosbag2, frame-aware actor state, and deterministic replay |
| v0.5 | Image ingestion, pretrained detection, localization, and tracking |
| v0.6 | Linux/NVIDIA CARLA input with GT and perception actor modes |
| v0.7 | Feature adaptation plus S2 and S3 inference |
| v0.8 | S4 evidence, asynchronous S5 reasoning, and semantic visualization |
| v0.9 | S6/S7 knowledge platform, Docker, and diagnostics |
| v1.0 | Integrated evaluation, CI/CD, documentation, benchmarks, and demos |

## Milestone governance

- A milestone is **Implemented** only when its acceptance criteria are satisfied and
  evidence is present in the repository or its approved CI results.
- Work under active construction is **In Progress**.
- Later releases remain **Planned** even when their design is documented.
- Scope changes require review against the authoritative architecture and relevant
  ADRs.
