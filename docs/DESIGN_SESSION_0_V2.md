# Signals-to-Semantics ROS 2
## Design Session 0 — Architecture V2.0

**Status:** Approved Architecture  
**Project stage:** Pre-implementation  
**Repository:** `signals-to-semantics-ros2`

---

## 1. Project Mission

Extend the Master's thesis project **Signals-to-Semantics** from an offline
scenario-mining research pipeline into a modular ROS 2 autonomous-driving
scenario-intelligence platform.

The system shall consume recorded or simulated autonomous-driving data,
construct a streaming representation of ego and surrounding actors, perform
perception and physical risk reasoning, detect safety-relevant events, reuse
the existing Signals-to-Semantics actor-ranking and semantic-reasoning
pipeline, and expose the results through ROS visualization, storage, and
semantic retrieval.

Target flow:

Recorded AV / CARLA
→ ROS 2 topics
→ sensor and ego streams
→ perception
→ TF2 coordinate transforms
→ actor tracking
→ risk and scenario detection
→ Signals-to-Semantics S2/S3
→ S4 deterministic evidence
→ asynchronous S5 LLM reasoning
→ RViz2 / S6 database / S7 retrieval

This project is an autonomous-driving **scenario-intelligence subsystem**.
It is not intended to implement a complete autonomous-driving stack.

---

## 2. Research Foundation

The project extends the existing public repository:

`Signals_to_Semantics`

The original research pipeline consists of:

- S0: signal preparation and kinematic processing
- S1: event verification and temporal stabilization
- S2: actor representation
- S3: GATv2 actor ranking
- S4: deterministic evidence construction
- S5: constrained LLM semantic reasoning
- S6: PostgreSQL / pgvector scenario knowledge base
- S7: semantic retrieval and reranking

The ROS 2 project must preserve the traceability principles of the original
research system rather than replacing them.

---

## 3. System Architecture

The target architecture is:

Input Sources
├── recorded ROS / autonomous-driving replay
└── CARLA simulation

↓ ROS 2

Sensor Layer
├── RGB camera
├── LiDAR / depth
├── IMU
└── ego odometry

↓

Perception and World Understanding
├── object detection
├── sensor association / localization
├── actor tracking
├── velocity estimation
└── TF2 frame transformations

↓

Normalized World Model
└── ActorStateArray + EgoState

↓

Deterministic Streaming Core
├── ego kinematics
├── relative geometry
├── TTC
├── THW
├── closest approach / DCA
├── trajectory interaction
├── braking / event detection
└── scenario-window management

↓

Signals-to-Semantics AI
├── S2 actor representation
├── S3 GATv2 actor ranking
├── S4 deterministic evidence
└── S5 constrained semantic reasoning

↓

Output Layer
├── RViz2
├── diagnostics
├── S6 PostgreSQL / pgvector
└── S7 semantic retrieval

---

## 4. Implementation Language Boundary

### C++

C++ is used for deterministic, streaming, latency-sensitive ROS components:

- ego-state processing
- coordinate/frame normalization where appropriate
- risk calculation
- TTC and related metrics
- event detection
- scenario-window management
- RViz visualization where useful

Primary ROS API: `rclcpp`.

### Python

Python is used where the ML/data ecosystem is strongest:

- perception
- tracking initially
- feature adaptation
- S2 representation inference
- S3 GATv2 inference
- S4 evidence construction
- S5 LLM reasoning
- S6 database integration
- S7 retrieval

Primary ROS API: `rclpy`.

The project must not rewrite existing ML models in C++ merely for technology
demonstration.

---

## 5. Source Modes

The downstream architecture must support multiple data sources through common
ROS interfaces.

### Replay Mode

Recorded scenario / rosbag2
→ ROS topics
→ full processing stack

Purpose:

- reproducible debugging
- regression testing
- demonstrations
- offline algorithm development

### Simulation Mode

CARLA
→ ROS topics
→ same downstream processing stack

Purpose:

- controlled scenario generation
- ground-truth evaluation
- perception evaluation
- repeatable ADAS/AD scenarios

The downstream risk and AI components should not depend directly on CARLA
types.

---

## 6. Actor Modes

Two actor-state providers shall eventually be supported.

### Ground-Truth Mode

CARLA ground-truth actor states are converted into the common
`ActorStateArray` representation.

Purpose:

- isolate downstream algorithm performance
- debug risk and actor-ranking logic
- establish reference results

### Perception Mode

Camera / LiDAR / depth
→ detection
→ localization
→ tracking
→ `ActorStateArray`

Purpose:

- evaluate the complete perception-to-semantics pipeline

Both modes must expose the same downstream interface.

---

## 7. Offline-Parity and Online Modes

The original thesis is primarily an offline pipeline operating on completed
scenario windows.

The ROS 2 extension must explicitly distinguish:

### Offline-Parity Mode

May use complete recorded windows.

Purpose:

- regression against thesis behaviour
- algorithm comparison
- reproducibility

### Online Mode

May use only information available at the current time and in the past.

Online mode must never use future ground-truth trajectory samples.

Future interaction estimates must therefore use prediction approaches such as:

- constant velocity
- constant acceleration
- CTRV where appropriate

Any smoothing algorithm requiring future samples must be replaced by either:

- causal filtering, or
- explicit fixed-latency processing

The difference must be documented and evaluated.

---

## 8. ROS Interfaces

Use standard ROS messages whenever an established message already exists.

Examples:

- `sensor_msgs/Image`
- `sensor_msgs/CameraInfo`
- `sensor_msgs/PointCloud2`
- `sensor_msgs/Imu`
- `nav_msgs/Odometry`
- `geometry_msgs/Pose`
- `geometry_msgs/Twist`
- `visualization_msgs/MarkerArray`

Custom interfaces shall be created only for scenario-specific concepts.

Planned custom messages include:

- `EgoState`
- `ActorState`
- `ActorStateArray`
- `ActorRisk`
- `ActorRiskArray`
- `BrakeEvent`
- `ScenarioWindow`
- `RankedActor`
- `RankedActorArray`
- `ScenarioSemantic`

---

## 9. ROS Topic Namespace

Initial source topics:

- `/camera/front/image_raw`
- `/camera/front/camera_info`
- `/lidar/points`
- `/imu/data`
- `/vehicle/odometry`
- `/tf`
- `/tf_static`

Signals-to-Semantics namespace:

- `/sts/ego/state`
- `/sts/actors/tracked`
- `/sts/risk/actors`
- `/sts/events/braking`
- `/sts/scenario/window`
- `/sts/actors/ranked`
- `/sts/scenario/evidence`
- `/sts/scenario/semantics`
- `/sts/visualization/markers`

---

## 10. Coordinate-Frame Model

Initial conceptual TF tree:

map
└── odom
    └── base_link
        ├── front_camera_link
        │   └── front_camera_optical
        ├── lidar_link
        └── imu_link

Every spatial value must have explicit frame ownership.

Sensor measurements shall be transformed into a normalized frame before
physical risk calculations.

---

## 11. QoS Principles

QoS shall be selected according to data semantics.

High-rate sensor streams may prioritize freshness:

- Best Effort where appropriate
- shallow queues
- Keep Last

Important event and semantic outputs should prioritize delivery:

- Reliable
- Keep Last

QoS decisions must eventually be documented per topic and validated through
testing rather than selected blindly.

---

## 12. Perception Strategy

The project shall initially avoid training a large perception model from
scratch.

Planned perception path:

RGB camera
→ pretrained object detector
→ detections

combined with:

LiDAR / depth + CameraInfo
→ spatial association
→ 3D actor localization
→ tracking / Kalman filtering
→ ActorStateArray

The perception stack should demonstrate understanding of:

- image messages
- PointCloud2
- camera calibration
- projection
- coordinate transforms
- detection
- association
- tracking
- velocity estimation
- uncertainty

A 2D detection box alone is not sufficient for physical TTC reasoning.

---

## 13. Risk Engine

The deterministic risk subsystem shall calculate actor-relative quantities
such as:

- longitudinal distance
- lateral distance
- Euclidean distance
- relative velocity
- closing velocity
- TTC
- time headway
- bearing
- relative heading
- predicted closest approach / DCA
- trajectory interaction / overlap
- risk score

Invalid or non-applicable TTC cases must be represented explicitly instead of
forcing every interaction into a one-dimensional collision model.

---

## 14. Event and Scenario Windowing

Sensor and risk processing operate continuously.

The S2/S3 AI pipeline is event-triggered.

Conceptual process:

continuous stream
→ risk/event trigger
→ pre-event history
→ event interval
→ post-event history
→ frozen ScenarioWindow
→ S2/S3 inference

Scenario-window timing must be configurable.

---

## 15. Signals-to-Semantics Integration

The original intelligence pipeline remains the semantic core.

ScenarioWindow
→ ROS-to-thesis feature adapter
→ S2 actor representation
→ S3 GATv2 ranking
→ Top-K responsible actors
→ S4 deterministic evidence
→ asynchronous S5 LLM reasoning
→ S6 storage
→ S7 retrieval

Existing trained models shall be reused initially.

Retraining is not required for the first integrated system.

Potential domain shift between AV2-derived features and simulation /
perception-derived features must later be evaluated explicitly.

---

## 16. LLM Boundary

The LLM must never belong to the physical or safety-critical processing path.

Physical processing must complete independently of LLM availability.

Correct architecture:

event detected
├── visualization / storage immediately
└── semantic job asynchronously
    → LLM
    → semantic enrichment

LLM latency or failure must not stop risk or event processing.

---

## 17. rosbag2

rosbag2 is a core development and validation capability.

It shall be used for:

- scenario replay
- deterministic debugging
- regression testing
- short scenario recording
- demonstrations

Large bags shall not be committed to Git.

Recording should use:

- selected topics
- short scenario durations
- compression where appropriate
- storage limits

---

## 18. RViz2

RViz2 shall act as both visualization and engineering-debugging interface.

The mature view should display:

- ego vehicle
- coordinate frames
- detected / tracked actors
- track IDs
- velocity vectors
- trajectories
- TTC / risk
- GAT ranking
- scenario event state
- semantic scenario label

---

## 19. Simulation Strategy

CARLA is the primary long-term autonomous-driving simulator.

The local Apple M1 Pro shall not be forced into running an unsupported or
impractical CARLA server.

Development is divided into:

### Local macOS stage

- ROS 2
- C++
- Python
- interfaces
- TF2
- rosbag2
- RViz2
- synthetic streams
- unit/integration tests
- lightweight ML inference
- repository development

### Future Linux/NVIDIA stage

- CARLA server
- GPU perception
- full integration tests
- large simulation experiments
- performance evaluation

Git provides continuity between both environments.

---

## 20. Current Hardware Constraint

Current development machine:

Apple MacBook Pro
Apple M1 Pro
16 GB unified memory
approximately 100–150 GB free storage

The project must therefore avoid:

- full AV2 re-download
- large local datasets
- long raw sensor recordings
- unnecessary duplicate environments
- large generated artifacts in Git

The existing thesis artifacts and database shall be reused selectively.

---

## 21. Docker Strategy

Docker is not a Day-1 dependency.

Order:

native implementation
→ tests
→ stable integration
→ Dockerization

Planned Docker use:

- reproducible Linux runtime
- Python / AI services
- PostgreSQL / pgvector
- CI validation
- eventual GPU integration

Do not containerize components before their native behaviour is understood.

---

## 22. CI/CD

GitHub Actions shall eventually validate:

- formatting
- C++ checks
- Python checks
- ROS workspace build
- unit tests
- integration smoke tests
- interface-contract tests
- Docker builds

CARLA/GPU scenarios should not run on every ordinary commit.

They may later run through:

- manual workflows
- release validation
- GPU runners

---

## 23. Testing Strategy

Testing layers:

1. Unit tests
2. ROS component tests
3. ROS integration tests
4. rosbag regression tests
5. CARLA scenario tests
6. AI regression tests
7. end-to-end tests
8. GT-vs-perception comparison

Example acceptance principle:

Do not accept:
"The node starts."

Prefer:
"For a known constant-deceleration input, the node produces the expected
acceleration within a defined numerical tolerance."

---

## 24. Observability

The mature system should measure:

- topic rate
- processing latency
- perception FPS
- actor count
- dropped messages
- risk-node latency
- event-detection latency
- S3 inference latency
- LLM latency
- system health

Performance claims must be measured rather than guessed.

---

## 25. Package Architecture

Planned ROS packages:

- `sts_interfaces`
- `sts_bringup`
- `sts_replay`
- `sts_carla_adapter`
- `sts_perception`
- `sts_tracking`
- `sts_ego_state_cpp`
- `sts_actor_transform_cpp`
- `sts_risk_engine_cpp`
- `sts_event_detector_cpp`
- `sts_window_manager_cpp`
- `sts_feature_adapter`
- `sts_actor_ranker`
- `sts_evidence`
- `sts_semantic_reasoner`
- `sts_database`
- `sts_retrieval`
- `sts_visualization_cpp`

Packages shall not all be implemented at once.

---

## 26. Release Roadmap

### v0.1 — ROS Foundation

- ROS 2 Jazzy environment
- C++ and Python ROS packages
- custom interface
- cross-language publisher/subscriber
- build/test foundation
- CI baseline

### v0.2 — Streaming Vehicle Core

- ego-state processing
- streaming velocity / acceleration / jerk
- event detection
- rolling temporal state

### v0.3 — Risk Intelligence

- actor representation
- TTC / relative velocity
- risk engine
- initial RViz visualization

### v0.4 — Replay and TF

- TF2
- rosbag2
- frame-aware actor state
- deterministic replay

### v0.5 — Perception

- image ingestion
- detector
- localization
- tracking

### v0.6 — CARLA

- Linux/NVIDIA environment
- CARLA scenario input
- GT actor mode
- perception actor mode

### v0.7 — AI Integration

- feature adapter
- S2 inference
- S3 GATv2 inference

### v0.8 — Semantic Intelligence

- S4 evidence
- asynchronous S5 reasoning
- semantic RViz output

### v0.9 — Knowledge Platform

- S6
- S7
- Docker
- diagnostics

### v1.0 — Portfolio Release

- integrated evaluation
- CI/CD
- documentation
- benchmark results
- demo scenarios
- recruiter-facing video and release

---

## 27. Development Philosophy

Every significant feature follows:

UNDERSTAND
→ DESIGN
→ SPECIFY CONTRACT
→ LEARN REQUIRED CONCEPTS
→ WRITE PSEUDOCODE
→ IMPLEMENT
→ BUILD
→ TEST
→ REVIEW
→ EXPLAIN
→ COMMIT

AI-generated implementation is not considered complete until the product owner
understands the important design and implementation decisions.

---

## 28. Project Roles

Product Owner:
Hariharan Chandrasekaran

Responsibilities:

- goals
- priority
- scope approval
- milestone acceptance

Senior Architect / Mentor:
ChatGPT architecture conversation

Responsibilities:

- architecture
- technical decisions
- teaching
- implementation review
- design critique

Implementation Engineer:
Codex

Responsibilities:

- repository implementation
- source-code changes
- builds
- tests
- refactoring according to approved specifications

Execution / Integration Agent:
ChatGPT Work

Responsibilities:

- repository-wide analysis
- multi-package integration
- milestone audits
- release preparation
- large project-management tasks

Source of Truth:
Git repository

---

## 29. GitHub Strategy

The repository shall be public during development.

The README must distinguish clearly between:

- implemented
- in progress
- planned

The project must never imply that planned functionality is already working.

Development progress should be visible through:

- roadmap
- milestones
- issues
- focused commits
- releases
- architecture documentation
- tests
- demo artifacts

---

## 30. Explicit Non-Goals for V1

V1 does not attempt to implement:

- full autonomous planning
- full vehicle control
- SLAM from scratch
- production localization stack
- full Autoware
- custom large 3D detector training
- complete sensor-fusion stack from scratch
- GATv2 rewrite in C++
- Kubernetes
- production safety certification

These may become later extensions where justified.

---

## 31. Architecture Authority

This document is the authoritative approved architecture baseline for the
project.

Derived documentation, implementation tasks, and source code must remain
consistent with it.

If implementation reveals a genuine conflict, limitation, or better design,
the conflict must be reviewed before this architecture is changed.

Codex must not silently change architectural decisions for implementation
convenience.