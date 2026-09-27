# Roadmap

**Project status:** Active Development  
**Current milestone:** v0.1 ROS 2 Foundation

This roadmap is derived from the approved architecture in
[`docs/DESIGN_SESSION_0_V2.md`](docs/DESIGN_SESSION_0_V2.md). A planned item is not
implemented merely because it appears here.

## Status key

- **Implemented:** delivered in the repository and verified at its stated scope.
- **In Progress:** the current approved delivery focus.
- **Planned:** architecturally approved future work, not yet delivered.

## Foundation

### M0.1 — Repository Foundation — Implemented

Establish the public-facing project documents, architecture derivatives, task
specifications, ADRs, contribution rules, license, and repository hygiene. This
milestone creates no production code or ROS packages.

### v0.1 — ROS 2 Foundation — In Progress

- ROS 2 Jazzy environment
- colcon workspace
- C++ and Python ROS packages
- `sts_interfaces`
- first approved custom message
- cross-language publisher/subscriber
- build and basic test foundation
- CI baseline

## Planned releases

### v0.2 — Streaming Vehicle Core

- ego-state processing
- streaming velocity, acceleration, and jerk
- event detection
- rolling temporal state

### v0.3 — Risk Intelligence

- actor representation
- TTC and relative velocity
- deterministic risk engine
- initial RViz2 visualization

### v0.4 — Replay and TF

- TF2
- rosbag2
- frame-aware actor state
- deterministic replay

### v0.5 — Perception

- image ingestion
- pretrained detector
- spatial localization
- tracking and velocity estimation

### v0.6 — CARLA

- Linux/NVIDIA environment
- CARLA scenario input
- ground-truth actor mode
- perception actor mode

### v0.7 — AI Integration

- ROS-to-thesis feature adapter
- S2 actor-representation inference
- S3 GATv2 actor-ranking inference

### v0.8 — Semantic Intelligence

- S4 deterministic evidence
- asynchronous S5 constrained semantic reasoning
- semantic RViz2 output

### v0.9 — Knowledge Platform

- S6 PostgreSQL/pgvector storage
- S7 semantic retrieval and reranking
- Dockerization after native stabilization
- diagnostics and system-health reporting

### v1.0 — Portfolio Release

- integrated evaluation
- complete CI/CD baseline for the delivered scope
- benchmark results
- documented demonstration scenarios
- recruiter-facing video and release documentation

CARLA and GPU scenarios are not intended to run on every ordinary commit. They may
later run through manual workflows, release validation, or GPU runners.
