# ADR-006: Run CARLA in a Future Linux/NVIDIA Environment

- **Status:** Accepted
- **Decision scope:** Development and simulation environment
- **Implementation status:** Planned for v0.6
- **Authority:** [`../DESIGN_SESSION_0_V2.md`](../DESIGN_SESSION_0_V2.md)

## Context

The current development computer is an Apple M1 Pro MacBook Pro with 16 GB unified
memory and limited storage. It should not be forced to run an unsupported or
impractical CARLA server. The project still needs CARLA for controlled scenarios,
ground-truth evaluation, and perception evaluation.

## Decision

Use a Mac-first, Linux/NVIDIA-later strategy:

- Local macOS development covers ROS 2, C++, Python, interfaces, TF2, rosbag2,
  RViz2, synthetic streams, unit/integration tests, lightweight ML inference, and
  repository work.
- A future Linux/NVIDIA environment runs the CARLA server, GPU perception, full
  integration tests, large simulation experiments, and performance evaluation.
- Git and stable ROS contracts provide continuity between the environments.

“Remote” in this ADR filename means CARLA is separated from the constrained macOS
development stage; it does not prescribe a specific hosting provider or network
topology.

## Consequences

- v0.1 does not require CARLA.
- Source-independent interfaces and adapters must allow later CARLA integration.
- Large datasets, long raw recordings, duplicate environments, and large generated
  Git artifacts are avoided locally.
- CARLA/GPU tests are not expected on every ordinary commit; later execution may use
  manual workflows, release validation, or GPU runners.
- Dockerization follows understood and tested native behavior rather than preceding
  it.
