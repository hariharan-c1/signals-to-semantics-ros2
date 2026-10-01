# Learning Roadmap

**Status:** Planned learning sequence aligned with staged implementation.

The project is intended to demonstrate understanding as well as produce software.
Implementation is not complete until the product owner understands the important
design and engineering decisions.

## v0.1 — ROS 2 foundations

Completed-work reference through M1.3B:
[M1 ROS 2 Foundation engineering note](engineering-notes/M1_ROS2_FOUNDATION.md).

- ROS 2 Jazzy concepts and environment structure.
- colcon workspaces and package boundaries.
- standard versus custom ROS interfaces.
- interface generation in `sts_interfaces`.
- `rclcpp` and `rclpy` node fundamentals.
- cross-language serialization and topic communication.
- introductory QoS compatibility.
- unit/integration testing and CI foundations.

## v0.2-v0.4 — Deterministic streaming systems

- streaming velocity, acceleration, jerk, and rolling state.
- explicit numerical validity and tolerance-based tests.
- event detection and configurable scenario windows.
- relative geometry, TTC, THW, DCA, and trajectory interaction.
- TF2 frame ownership, transform timing, and sensor frames.
- rosbag2 recording, replay, compression, and regression testing.
- RViz2 as an engineering-debugging interface.

## v0.5-v0.6 — Perception and simulation

- `sensor_msgs/Image`, `CameraInfo`, and `PointCloud2`.
- camera calibration and projection.
- pretrained detection rather than large-model training from scratch.
- camera/LiDAR or depth association and 3D localization.
- tracking, Kalman filtering, velocity estimation, and uncertainty.
- CARLA adapters and source-independent ROS contracts.
- GT-vs-perception evaluation on Linux/NVIDIA.

## v0.7-v0.8 — Research pipeline integration

- feature-contract mapping from `ScenarioWindow` to thesis inputs.
- S2 actor representations and S3 GATv2 inference.
- Top-K actor responsibility ranking.
- domain-shift evaluation across AV2, simulation, and perception-derived features.
- S4 deterministic evidence and traceability.
- asynchronous S5 reasoning, failure isolation, and latency measurement.

## v0.9-v1.0 — Knowledge platform and delivery

- PostgreSQL/pgvector scenario storage.
- semantic retrieval and reranking.
- diagnostics, health, latency, throughput, and dropped-message measurement.
- native-first integration followed by purposeful Dockerization.
- layered CI/CD, manual GPU validation, benchmarks, demos, and release evidence.

## Recurring engineering method

For each significant feature:

```text
UNDERSTAND -> DESIGN -> SPECIFY CONTRACT -> LEARN REQUIRED CONCEPTS
-> WRITE PSEUDOCODE -> IMPLEMENT -> BUILD -> TEST -> REVIEW -> EXPLAIN -> COMMIT
```

Learning artifacts should clarify why a design is correct, which assumptions it
makes, how it is tested, and how it differs between offline-parity and online use.
