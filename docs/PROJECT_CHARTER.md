# Project Charter

**Project:** Signals-to-Semantics ROS 2  
**Status:** Active Development  
**Current milestone:** v0.2 Streaming Vehicle Core — In Progress
**Architecture authority:** [`DESIGN_SESSION_0_V2.md`](DESIGN_SESSION_0_V2.md)

## Purpose

Signals-to-Semantics ROS 2 extends the existing
[Signals-to-Semantics research project](https://github.com/hariharan-c1/Signals_to_Semantics)
from an offline scenario-mining pipeline into a modular ROS 2
autonomous-driving scenario-intelligence platform. The extension preserves the
research system's traceability while introducing streaming inputs, physical risk
reasoning, event-triggered processing, ROS visualization, and source-independent
interfaces.

## Mission

Consume recorded or simulated autonomous-driving data; build a streaming world
model of the ego vehicle and surrounding actors; detect and measure safety-relevant
interactions; reuse the S2-S7 research pipeline; and expose results through RViz2,
diagnostics, storage, and semantic retrieval.

The project is a scenario-intelligence subsystem. It does not provide a complete
autonomous-driving stack.

## Objectives

- Establish stable ROS contracts for sensor, ego, actor, risk, event, scenario, and
  semantic information.
- Separate deterministic physical processing from ML and LLM processing.
- Support reproducible rosbag2 replay and future CARLA simulation through the same
  downstream interfaces.
- Support ground-truth and perception actor providers without changing downstream
  risk and AI components.
- Distinguish offline-parity evaluation from causally valid online operation.
- Integrate the original S2 actor representation, S3 GATv2 ranking, S4 evidence,
  S5 constrained reasoning, S6 knowledge base, and S7 retrieval stages.
- Build verifiable functionality incrementally, with measured rather than assumed
  performance.

## Current delivery boundary

### Implemented

- Approved architecture baseline.
- M0.1 repository documentation and governance foundation.
- v0.1 ROS 2 Foundation: Jazzy environment, colcon workspace, `EgoState v1`, C++
  publisher, Python subscriber, cross-language tests, and verified Ubuntu CI;
  see the [M1 acceptance record](tasks/M1_ROS_FOUNDATION.md).

### In Progress

- v0.2 Streaming Vehicle Core: M2.2B ego-state package implemented, built, and
  estimator unit-tested locally; ROS end-to-end verification pending. See the
  [M2 task](tasks/M2_STREAMING_VEHICLE_CORE.md).

### Planned

- M2 event detection and rolling temporal state; complete streaming-path acceptance
  awaits ROS runtime/integration verification.
- Later milestones: risk intelligence, scenario-window integration, TF2, replay,
  perception, CARLA, S2-S7 integration, visualization, diagnostics, Docker, and
  integrated evaluation.

## Stakeholders and roles

- **Product Owner — Hariharan Chandrasekaran:** goals, priorities, scope approval,
  and milestone acceptance.
- **Senior Architect / Mentor — ChatGPT architecture conversation:** architecture,
  technical decisions, teaching, review, and design critique.
- **Implementation Engineer — Codex:** repository changes, builds, tests, and
  approved refactoring.
- **Execution / Integration Agent — ChatGPT Work:** repository-wide analysis,
  multi-package integration, milestone audits, and release preparation.

Git is the source of truth.

## Constraints

Local development targets an Apple M1 Pro MacBook Pro with 16 GB unified memory and
limited free storage. Large datasets, long raw recordings, unnecessary duplicate
environments, and large generated Git artifacts must be avoided. Local work covers
ROS 2, interfaces, C++, Python, TF2, rosbag2, RViz2, synthetic streams, tests, and
lightweight inference. CARLA, GPU perception, large experiments, and performance
evaluation are reserved for a future Linux/NVIDIA environment.

Docker is not a Day-1 dependency. Native behavior must be understood and tested
before containerization.

## Success principles

- Planned capabilities are never presented as implemented.
- Every spatial value has explicit frame ownership before physical risk processing.
- Invalid or non-applicable TTC is represented explicitly.
- Online processing never consumes future ground-truth samples.
- LLM failure or latency cannot stop physical processing.
- Tests assert meaningful outputs and tolerances, not only process startup.
- Architectural conflicts are reviewed rather than silently resolved in code.

## V1 non-goals

Full planning, full vehicle control, SLAM from scratch, a production localization
stack, full Autoware, training a custom large 3D detector, building a complete
sensor-fusion stack from scratch, rewriting GATv2 in C++, Kubernetes, and production
safety certification are outside V1.
