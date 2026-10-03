# Architecture

**Status:** Derived architecture guide; M1 ROS 2 Foundation is Implemented.
M2 / v0.2 is In Progress: the M2.2B ego-state package is implemented, built, and
estimator unit-tested locally; ROS end-to-end verification is pending. The complete
target system below remains Planned. See [milestones](MILESTONES.md).
**Authority:** [`DESIGN_SESSION_0_V2.md`](DESIGN_SESSION_0_V2.md) remains the
authoritative approved architecture if this summary is incomplete or ambiguous.

## System context

Signals-to-Semantics ROS 2 sits between autonomous-driving data sources and
scenario-intelligence consumers. It normalizes replay or simulation data, computes
physical interactions, captures safety-relevant windows, applies the existing
research intelligence stages, and publishes results for engineering visualization,
storage, and retrieval.

```text
Inputs                 Streaming world model          Intelligence          Outputs
------------------     ---------------------------    ------------------    ---------
Recorded ROS data  ->  sensors + ego                -> deterministic      -> RViz2
CARLA simulation   ->  perception / GT adapter      -> risk + events      -> diagnostics
                       tracking + TF2               -> scenario window    -> S6 storage
                       EgoState + ActorStateArray   -> S2/S3/S4/S5        -> S7 retrieval
```

Downstream risk and AI components consume common ROS interfaces and do not depend
on CARLA-specific types.

## Layers

### 1. Input sources

- Recorded autonomous-driving data or rosbag2 replay supports deterministic
  debugging, regression, demonstrations, and offline development.
- CARLA supports controlled scenario generation, ground-truth evaluation,
  perception evaluation, and repeatable ADAS/AD scenarios in a future Linux/NVIDIA
  phase.

### 2. Sensor and ego streams

The planned source streams are front RGB image and calibration, LiDAR/depth, IMU,
ego odometry, and TF. Established ROS messages are preferred for these streams.

### 3. Perception and world understanding

The planned perception path uses a pretrained detector rather than training a large
model from scratch. Camera detections are associated with LiDAR or depth using
camera calibration, then localized, tracked, transformed, and used for velocity
estimation. A 2D box alone is insufficient for physical TTC reasoning.

Ground-truth mode bypasses perception by adapting CARLA actor states. Both paths
produce the same normalized `ActorStateArray` representation.

### 4. Normalized world model

`EgoState` and `ActorStateArray` form the source-independent boundary into the
deterministic streaming core. Every spatial value must own an explicit coordinate
frame. Sensor measurements are transformed into a normalized frame before physical
risk calculations.

### 5. Deterministic streaming core

The core continuously calculates ego kinematics, relative geometry, closing
velocity, TTC, time headway, bearing, relative heading, predicted closest approach
or DCA, trajectory interaction, and risk. It detects braking or other configured
events and maintains configurable pre-event, event, and post-event history.

Invalid or non-applicable TTC values remain explicit rather than being forced into
a one-dimensional collision model.

### 6. Event-triggered AI

Continuous processing freezes a `ScenarioWindow` only when an event trigger and its
configured history are complete. The frozen window then flows through:

1. ROS-to-thesis feature adaptation.
2. S2 actor representation.
3. S3 GATv2 actor ranking and Top-K actor selection.
4. S4 deterministic evidence construction.
5. Asynchronous S5 constrained semantic reasoning.
6. S6 PostgreSQL/pgvector storage.
7. S7 semantic retrieval and reranking.

Existing trained models are reused initially; retraining is not required for the
first integrated system. Domain shift between AV2-derived and simulation- or
perception-derived features must later be evaluated explicitly.

### 7. Outputs and observability

The mature system exposes physical and semantic state through RViz2, diagnostics,
the S6 knowledge base, and S7 retrieval. Planned measurements include topic rate,
processing latency, perception FPS, actor count, dropped messages, risk and event
latency, S3 and LLM latency, and overall health.

## Language boundary

- **C++ with `rclcpp`:** deterministic, streaming, latency-sensitive components—ego
  processing, appropriate frame normalization, physical risk, events, window
  management, and useful visualization.
- **Python with `rclpy`:** perception, initial tracking, feature adaptation, S2/S3
  inference, S4 evidence, S5 reasoning, S6 integration, and S7 retrieval.

The boundary follows behavior and ecosystem needs. Existing ML models are not
rewritten in C++ merely to demonstrate C++.

## Critical isolation boundary

The LLM is outside the physical and safety-critical path:

```text
event detected
├── publish/visualize/store physical result immediately
└── enqueue semantic job
    └── S5 result enriches the scenario when available
```

Physical processing remains available when S5 is slow, unavailable, or fails.

## Deployment evolution

The project develops natively on macOS first, moves CARLA and GPU work to
Linux/NVIDIA later, and adds Docker only after stable native integration. Packages
are delivered incrementally according to the release roadmap; the complete target
package list is not a commitment to create every package at once.
