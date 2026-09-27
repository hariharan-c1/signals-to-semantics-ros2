# Agent Instructions

These instructions apply to the entire repository.

## Required reading before implementation

Before writing or changing production code, every coding agent must:

1. Read [`docs/DESIGN_SESSION_0_V2.md`](docs/DESIGN_SESSION_0_V2.md) completely.
   It is the authoritative approved architecture.
2. Read the ADRs relevant to the requested change under [`docs/adr/`](docs/adr/).
3. Read the applicable interface contracts, especially
   [`docs/ROS_INTERFACE_SPEC.md`](docs/ROS_INTERFACE_SPEC.md),
   [`docs/TF_TREE.md`](docs/TF_TREE.md), and [`docs/QOS.md`](docs/QOS.md).
4. Read the active task specification under [`docs/tasks/`](docs/tasks/).
5. Check the current milestone and status in [`docs/MILESTONES.md`](docs/MILESTONES.md).

If these sources disagree, stop implementation and surface the conflict for review.
The authoritative architecture wins until an explicit, reviewed decision changes it.

## Architecture and interface control

- Never silently change an architectural decision for implementation convenience.
- Never silently change a message schema, topic, frame, QoS contract, actor-mode
  contract, or online/offline semantic.
- Propose architecture changes through review and an ADR before implementation.
- Prefer standard ROS interfaces when an established message fits.
- Keep CARLA-specific representations inside adapters.
- Preserve the shared downstream `ActorStateArray` interface for both ground-truth
  and perception modes.
- Keep LLM processing asynchronous and outside the physical or safety-critical path.
- Do not rewrite existing ML models in C++ merely as a technology demonstration.

## Scope and truthfulness

The project is in **Active Development** and the current milestone is
**v0.1 ROS 2 Foundation**. Distinguish **Implemented**, **In Progress**, and
**Planned** in code comments, documentation, tests, and reports. Never claim planned
functionality is operational without implementation and verification evidence.

Implement only the approved task scope. Do not add speculative packages,
dependencies, Dockerization, datasets, checkpoints, or placeholder production
components. Preserve unrelated user changes and do not commit unless explicitly
asked.

## Engineering expectations

- Specify contracts before implementation.
- Make frame ownership, units, timestamps, validity, and failure semantics explicit.
- Test outputs and tolerances rather than only checking that a node starts.
- Measure performance claims; do not guess them.
- Keep local development compatible with the Apple M1 Pro constraint, reserving
  CARLA/GPU work for the planned Linux/NVIDIA phase.
- Do not commit credentials, local configuration overrides, raw datasets, rosbag
  recordings, database volumes, or large/private model artifacts.
