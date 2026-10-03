# ROS Graph

**Status:** Target conceptual graph beyond the Implemented M1 contract exchange.
M1 implements `sts_contract_publisher_cpp` → `/sts/ego/state` (`EgoState v1`) →
`sts_contract_subscriber_py`; see the [M1 task](tasks/M1_ROS_FOUNDATION.md).
M2 / v0.2 is In Progress. `sts_ego_state_cpp` implements the Odometry-to-EgoState
node locally, with build and pure estimator unit evidence; ROS end-to-end graph
verification is pending. Other production nodes remain Planned. See the
[M2 task](tasks/M2_STREAMING_VEHICLE_CORE.md).

The graph below describes target functional boundaries and topics. The M2 task
fixes the ego-state executable/node names; other executable names, process
composition, launch structure, and node mappings remain milestone-level decisions.

## Continuous path

```text
recorded replay or CARLA adapters
    ├── /camera/front/image_raw
    ├── /camera/front/camera_info
    ├── /lidar/points
    ├── /imu/data
    ├── /vehicle/odometry
    ├── /tf
    └── /tf_static
            |
            v
perception / localization / tracking OR CARLA ground-truth adapter
            |
            +--> /sts/actors/tracked      [ActorStateArray]
            |
ego processing
            +--> /sts/ego/state           [EgoState]
            |
            v
frame normalization and deterministic risk engine
            +--> /sts/risk/actors         [ActorRiskArray]
            |
            v
event detection
            +--> /sts/events/braking      [BrakeEvent]
            |
            v
rolling history and scenario-window management
```

Both the GT and perception branches terminate at `/sts/actors/tracked`; downstream
components are intentionally source-independent.

## Event-triggered path

```text
risk/event trigger + pre/event/post history
            |
            v
/sts/scenario/window             [ScenarioWindow]
            |
            v
ROS-to-thesis feature adapter -> S2 representation -> S3 GATv2 ranking
            |
            +--> /sts/actors/ranked       [RankedActorArray]
            |
            v
S4 deterministic evidence
            +--> /sts/scenario/evidence   [type not yet approved]
            |
            +------------------> immediate visualization / storage path
            |
            └--> asynchronous S5 semantic job
                    |
                    +--> /sts/scenario/semantics [ScenarioSemantic]
                    |
                    ├--> S6 PostgreSQL / pgvector
                    └--> S7 semantic retrieval and reranking
```

The physical path must finish without waiting for S5. Failure or latency in semantic
reasoning cannot stop risk or event processing.

## Visualization and diagnostics

The planned visualization path publishes
`/sts/visualization/markers` as `visualization_msgs/MarkerArray`. The mature RViz2
view is intended to show ego state, frames, actors, IDs, velocity vectors,
trajectories, TTC/risk, GAT ranking, event state, and semantic label. Diagnostics
and system-health interfaces remain to be specified in a future milestone.

## Graph invariants

- Common ROS contracts isolate downstream components from source-specific types.
- Spatial data is transformed into a normalized frame before physical risk
  calculations.
- High-rate continuous processing and event-triggered AI are separate paths.
- Scenario-window timing is configurable.
- Online operation is causal; offline-parity operation is explicitly identified.
