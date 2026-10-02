# Interview Notes

**Status:** Project narrative based on the approved architecture and current
repository evidence. Keep these notes synchronized with delivered functionality.

## One-sentence description

Signals-to-Semantics ROS 2 is a modular scenario-intelligence subsystem that extends
an offline autonomous-driving research pipeline into a staged ROS 2 architecture for
recorded and simulated streams.

## Research-to-systems story

The original
[Signals-to-Semantics](https://github.com/hariharan-c1/Signals_to_Semantics)
research project processes completed scenarios through signal preparation, event
verification, actor representation, GATv2 ranking, deterministic evidence,
constrained LLM reasoning, vector-backed storage, and semantic retrieval. This new
repository does not replace that work. It adds the systems architecture required to
feed it from ROS 2 while preserving traceability.

## Architectural talking points

- **Clear real-time boundary:** deterministic kinematics, risk, events, and scenario
  windows belong in C++; ML/data-heavy stages remain in Python.
- **Source independence:** replay and CARLA converge on standard/common ROS
  interfaces instead of leaking simulator types downstream.
- **Comparable actor providers:** CARLA ground truth and perception both produce
  `ActorStateArray`, enabling controlled GT-vs-perception evaluation.
- **Causal honesty:** offline-parity can reproduce completed-window research
  behavior, while online mode is prohibited from using future ground truth.
- **Safe LLM placement:** S5 enrichment is asynchronous and cannot block physical
  processing.
- **Traceability:** S3 ranking is followed by deterministic S4 evidence before
  semantic enrichment, storage, and retrieval.
- **Validation focus:** tests check numerical outcomes, frames, contracts, and
  regressions rather than only node startup.

## Platform strategy

The local Apple M1 Pro is used for interfaces, C++/Python development, TF2, rosbag2,
RViz2, synthetic streams, tests, and lightweight inference. CARLA and GPU-heavy
perception move to a future Linux/NVIDIA environment. This avoids distorting the
architecture around an impractical local simulator setup while maintaining one Git
history and shared contracts.

## Current honest status

- **Implemented:** approved architecture, M0.1 repository foundation, and v0.1 ROS 2
  Foundation; see the [M1 acceptance record](tasks/M1_ROS_FOUNDATION.md).
- **In Progress:** v0.2 Streaming Vehicle Core at M2.0 approved design and
  documentation; see the [M2 task](tasks/M2_STREAMING_VEHICLE_CORE.md).
- **Planned:** M2 runtime implementation and later risk, replay, perception, CARLA,
  AI, semantic, database, retrieval, and mature visualization capabilities.

Do not describe any planned package as operational during an interview until its
acceptance evidence exists.

## Intended engineering evidence by v1.0

The portfolio release is planned to present integrated evaluation, benchmark
results, documented scenario demonstrations, CI/CD, architecture decisions, tests,
and a recruiter-facing video. Performance claims will be measured rather than
guessed.

## Scope discipline

This is not a full AV stack. It deliberately excludes planning, control, full
Autoware, SLAM from scratch, production localization, custom large 3D-detector
training, complete sensor fusion from scratch, a GATv2 C++ rewrite, Kubernetes, and
production safety certification for V1.
