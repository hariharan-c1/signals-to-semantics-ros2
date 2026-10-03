# Signals-to-Semantics ROS 2

**Status:** Active Development  
**Current milestone:** v0.2 Streaming Vehicle Core — In Progress\
**Next planned milestone:** v0.3 Risk Intelligence\
**Implementation state:** Verified M1 ROS 2 foundation; M2.2B ego-state package built and estimator unit-tested locally; ROS end-to-end verification pending.

Signals-to-Semantics ROS 2 is a systems extension of the Master's thesis project
[Signals-to-Semantics](https://github.com/hariharan-c1/Signals_to_Semantics).
The research project established an offline, traceable pipeline for turning vehicle
signals into ranked actors, deterministic evidence, semantic explanations, and
retrievable scenarios. This repository is building the ROS 2 architecture needed
to apply that intelligence to recorded and simulated autonomous-driving streams.

The goal is an autonomous-driving **scenario-intelligence subsystem**: it observes
ego and surrounding actors, computes physical risk, detects safety-relevant events,
and enriches frozen scenario windows with the existing research pipeline. It is not
a planner, controller, or complete autonomous-driving stack.

## Target system

The following system is the target architecture. M1 foundation and the M2.2B
ego-state package exist at the verification scopes described below; the complete
production system remains Planned.

```text
Recorded AV data / CARLA
        -> ROS 2 sensor and ego topics
        -> perception, localization, tracking, and TF2
        -> normalized EgoState + ActorStateArray
        -> deterministic risk and event processing
        -> event-triggered ScenarioWindow
        -> S2 actor representation + S3 GATv2 ranking
        -> S4 deterministic evidence
        -> asynchronous S5 semantic reasoning
        -> RViz2 / S6 PostgreSQL + pgvector / S7 retrieval
```

The physical pipeline remains independent of the LLM. Semantic enrichment is
asynchronous and must never block risk calculation, event detection, visualization,
or immediate storage.

## Delivery status

| State | What it means here |
| --- | --- |
| **Implemented** | Approved architecture baseline, M0.1 repository documentation foundation, and v0.1 ROS 2 Foundation: Jazzy environment, colcon workspace, approved `EgoState v1` interface, C++ publisher, Python subscriber, tests, and Ubuntu CI |
| **In Progress** | v0.2 Streaming Vehicle Core: M2.2B `sts_ego_state_cpp` implemented, built, and estimator unit-tested locally; ROS end-to-end verification pending; event detection and rolling temporal state Planned |
| **Planned** | Later milestones: risk intelligence, TF2 and replay, perception, CARLA, S2-S7 integration, diagnostics, Docker, and integrated evaluation |

The three implemented M1 ROS packages are `sts_interfaces`,
`sts_contract_publisher_cpp`, and `sts_contract_subscriber_py`. C++ → Python
communication is automatically regression tested with `launch_pytest` and verified
locally on macOS arm64 and in GitHub Actions on Ubuntu 24.04. Linux CI built all
three packages and reported **23 tests, 0 errors, 0 failures, 1 skipped** (the
documented cppcheck tooling skip). See the [M1 acceptance record](docs/tasks/M1_ROS_FOUNDATION.md).

Those M1 nodes exchange and validate a synthetic canonical payload. M2.2B adds
`sts_ego_state_cpp`, containing `EgoKinematicsEstimator` and `EgoStateNode`, following
the [M2 task](docs/tasks/M2_STREAMING_VEHICLE_CORE.md) and
[engineering note](docs/engineering-notes/M2_STREAMING_VEHICLE_CORE.md). The package
has local build and estimator unit evidence; ROS end-to-end runtime/integration
verification remains pending. Event detection, rolling temporal state, TF2,
perception, CARLA, GAT, LLM runtime, and risk intelligence remain Planned. Local M2
unit evidence does not establish full v0.2 acceptance or Linux runtime verification.

## Development strategy

Development starts locally on an Apple M1 Pro MacBook Pro with 16 GB unified
memory. The intended macOS stage targets repository development, ROS 2 interfaces,
C++ and Python, TF2, rosbag2, RViz2, synthetic streams, tests, and lightweight
inference. Each capability becomes implemented only after it has been built and
verified. The local machine will not be forced to host an unsupported or impractical
CARLA server.

A later Linux/NVIDIA phase will add the CARLA server, GPU perception, full
integration tests, large simulation experiments, and performance evaluation. Git
provides continuity between the two environments. Docker is planned only after
native behavior is understood and tested.

## Operating modes

- **Replay and simulation sources** feed common ROS interfaces; downstream logic
  must not depend directly on CARLA types.
- **Ground-truth and perception actor modes** both publish the same normalized
  `ActorStateArray` contract.
- **Offline-parity mode** may use complete recorded windows for thesis regression.
- **Online mode** uses only present and past information; it must never consume
  future ground-truth trajectory samples.

## Documentation

- [Authoritative architecture](docs/DESIGN_SESSION_0_V2.md)
- [Project charter](docs/PROJECT_CHARTER.md)
- [Derived architecture guide](docs/ARCHITECTURE.md)
- [Package map](docs/PACKAGE_MAP.md)
- [ROS interface specification](docs/ROS_INTERFACE_SPEC.md)
- [ROS graph](docs/ROS_GRAPH.md)
- [TF tree](docs/TF_TREE.md)
- [QoS policy](docs/QOS.md)
- [Online versus offline modes](docs/ONLINE_VS_OFFLINE.md)
- [Design decisions and ADRs](docs/DESIGN_DECISIONS.md)
- [Testing strategy](docs/TESTING.md)
- [Milestones](docs/MILESTONES.md)
- [Learning roadmap](docs/LEARNING_ROADMAP.md)
- [Interview notes](docs/INTERVIEW_NOTES.md)
- [Release roadmap](ROADMAP.md)
- [Contribution guide](CONTRIBUTING.md)

## Explicit V1 non-goals

V1 does not attempt full planning or vehicle control, SLAM from scratch, a
production localization stack, full Autoware, custom large 3D-detector training, a
complete sensor-fusion stack from scratch, a C++ rewrite of GATv2, Kubernetes, or
production safety certification.

## License

This repository is licensed under the [MIT License](LICENSE).
