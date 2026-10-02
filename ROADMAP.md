# Roadmap

**Project status:** Active Development  
**Current milestone:** v0.2 Streaming Vehicle Core — In Progress

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

### v0.1 — ROS 2 Foundation — Implemented

- ROS 2 Jazzy environment
- colcon workspace
- C++ and Python ROS packages
- `sts_interfaces`
- first approved custom message
- cross-language publisher/subscriber
- build and basic test foundation
- CI baseline

## In Progress

### v0.2 — Streaming Vehicle Core — In Progress

M2.0 records the approved streaming design in the
[M2 task](docs/tasks/M2_STREAMING_VEHICLE_CORE.md) and
[engineering note](docs/engineering-notes/M2_STREAMING_VEHICLE_CORE.md).
Production implementation remains Planned; no M2 package is created in M2.0.

- ego-state processing
- streaming velocity, acceleration, and jerk
- event detection
- rolling temporal state

## Planned releases

### v0.3 — Risk Intelligence — Planned

- actor representation
- TTC and relative velocity
- deterministic risk engine
- initial RViz2 visualization

### v0.4 — Replay and TF — Planned

- TF2
- rosbag2
- frame-aware actor state
- deterministic replay

### v0.5 — Perception — Planned

- image ingestion
- pretrained detector
- spatial localization
- tracking and velocity estimation

### v0.6 — CARLA — Planned

- Linux/NVIDIA environment
- CARLA scenario input
- ground-truth actor mode
- perception actor mode

### v0.7 — AI Integration — Planned

- ROS-to-thesis feature adapter
- S2 actor-representation inference
- S3 GATv2 actor-ranking inference

### v0.8 — Semantic Intelligence — Planned

- S4 deterministic evidence
- asynchronous S5 constrained semantic reasoning
- semantic RViz2 output

### v0.9 — Knowledge Platform — Planned

- S6 PostgreSQL/pgvector storage
- S7 semantic retrieval and reranking
- Dockerization after native stabilization
- diagnostics and system-health reporting

### v1.0 — Portfolio Release — Planned

- integrated evaluation
- complete CI/CD baseline for the delivered scope
- benchmark results
- documented demonstration scenarios
- recruiter-facing video and release documentation

CARLA and GPU scenarios are not intended to run on every ordinary commit. They may
later run through manual workflows, release validation, or GPU runners.
