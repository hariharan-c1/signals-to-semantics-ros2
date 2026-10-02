# ADR-002: Separate Deterministic C++ from ML/Data Python

- **Status:** Accepted
- **Decision scope:** Implementation-language boundary
- **Implementation status:** M1 cross-language proof Implemented / Accepted; production language boundary Planned, with M2.0 design In Progress
- **Authority:** [`../DESIGN_SESSION_0_V2.md`](../DESIGN_SESSION_0_V2.md)

## Context

The system combines continuous latency-sensitive physical processing with
perception, ML inference, semantic reasoning, storage, and retrieval. These concerns
have different runtime and ecosystem needs.

## Decision

Use C++ with `rclcpp` for deterministic, streaming, latency-sensitive ROS work:

- ego-state processing;
- coordinate/frame normalization where appropriate;
- risk and TTC-related calculations;
- event detection;
- scenario-window management; and
- useful RViz2 visualization components.

Use Python with `rclpy` where the ML and data ecosystem is strongest:

- perception and initial tracking;
- feature adaptation;
- S2 representation and S3 GATv2 inference;
- S4 evidence and S5 semantic reasoning;
- S6 database integration; and
- S7 retrieval.

Connect both sides with explicit ROS interface contracts. Do not rewrite existing ML
models in C++ merely for technology demonstration.

## Consequences

- v0.1 must prove generated-interface compatibility across C++ and Python.
- Interface changes affect both language ecosystems and require contract tests.
- Package boundaries should follow responsibility rather than forcing one language
  across the full system.
- Cross-process and serialization behavior becomes part of integration testing.
