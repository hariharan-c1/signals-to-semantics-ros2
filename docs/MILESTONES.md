# Milestones

**Project status:** Active Development  
**Current milestone:** v0.1 ROS 2 Foundation

The milestone states distinguish repository evidence from architectural intent.

## Implemented

### M0.1 — Repository Foundation

Documentation, governance, architecture derivatives, ADRs, task specifications,
license, and ignore policy are established. This milestone intentionally contains
no production source code, ROS packages, Docker files, checkpoints, or datasets.

Acceptance criteria are defined in
[`tasks/M0_REPO_FOUNDATION.md`](tasks/M0_REPO_FOUNDATION.md).

## In Progress

### v0.1 — ROS 2 Foundation

Scope is limited to:

- ROS 2 Jazzy environment;
- colcon workspace;
- `sts_interfaces`;
- first approved custom message;
- C++ ROS node;
- Python ROS node;
- cross-language communication;
- basic tests; and
- CI baseline.

The execution contract is
[`tasks/M1_ROS_FOUNDATION.md`](tasks/M1_ROS_FOUNDATION.md).

## Planned

| Release | Outcome |
| --- | --- |
| v0.2 | Streaming ego kinematics, event detection, and rolling temporal state |
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
