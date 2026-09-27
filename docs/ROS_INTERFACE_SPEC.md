# ROS Interface Specification

**Status:** Planned interface catalogue; wire schemas are not yet approved or
implemented.  
**Current milestone:** v0.1 will define and validate the first custom message.

This document records the interfaces named by the approved architecture without
inventing fields, units, or validity rules that have not yet been designed. Before
an interface is implemented, its exact contract must be reviewed here or in a
linked specification.

## Interface policy

- Use a standard ROS message whenever an established message fits.
- Add custom messages only for scenario-specific concepts.
- Every spatial field must identify its frame ownership.
- Every implemented contract must specify units, timestamp meaning, validity,
  unavailable-value behavior, ordering, identifiers, and compatibility expectations.
- Invalid or non-applicable TTC must be representable explicitly.
- Source adapters may understand CARLA; downstream risk and AI components may not
  depend on CARLA-specific types.
- GT and perception providers must publish the same downstream actor contract.

## Standard source interfaces

| Topic | Standard message | Purpose | Status |
| --- | --- | --- | --- |
| `/camera/front/image_raw` | `sensor_msgs/Image` | Front RGB image | Planned |
| `/camera/front/camera_info` | `sensor_msgs/CameraInfo` | Camera calibration | Planned |
| `/lidar/points` | `sensor_msgs/PointCloud2` | LiDAR/depth point data | Planned |
| `/imu/data` | `sensor_msgs/Imu` | Inertial measurements | Planned |
| `/vehicle/odometry` | `nav_msgs/Odometry` | Ego odometry | Planned |
| `/tf` | Standard TF2 transform messages | Dynamic transforms | Planned |
| `/tf_static` | Standard TF2 transform messages | Static transforms | Planned |

`geometry_msgs/Pose`, `geometry_msgs/Twist`, and
`visualization_msgs/MarkerArray` are also preferred where their standard semantics
fit.

## Planned custom messages

| Interface | Scenario-specific purpose | Status |
| --- | --- | --- |
| `EgoState` | Normalized ego state for downstream processing | Planned; schema not approved |
| `ActorState` | One normalized tracked or ground-truth actor state | Planned; schema not approved |
| `ActorStateArray` | Common actor-provider boundary | Planned; schema not approved |
| `ActorRisk` | Risk quantities for one actor | Planned; schema not approved |
| `ActorRiskArray` | Actor-relative risk output collection | Planned; schema not approved |
| `BrakeEvent` | Braking-event representation | Planned; schema not approved |
| `ScenarioWindow` | Frozen pre-event, event, and post-event context | Planned; schema not approved |
| `RankedActor` | Ranked actor result from S2/S3 | Planned; schema not approved |
| `RankedActorArray` | Top-K/ranked actor collection | Planned; schema not approved |
| `ScenarioSemantic` | Constrained semantic enrichment result | Planned; schema not approved |

The “first custom message” required by v0.1 has not been selected by the approved
architecture. M1 must select it and approve its complete contract before generating
the interface package. No field layout is implied by the names above.

## Planned Signals-to-Semantics topics

| Topic | Intended payload | Status |
| --- | --- | --- |
| `/sts/ego/state` | `EgoState` | Planned |
| `/sts/actors/tracked` | `ActorStateArray` | Planned |
| `/sts/risk/actors` | `ActorRiskArray` | Planned |
| `/sts/events/braking` | `BrakeEvent` | Planned |
| `/sts/scenario/window` | `ScenarioWindow` | Planned |
| `/sts/actors/ranked` | `RankedActorArray` | Planned |
| `/sts/scenario/evidence` | Evidence payload; exact interface not yet approved | Planned |
| `/sts/scenario/semantics` | `ScenarioSemantic` | Planned |
| `/sts/visualization/markers` | `visualization_msgs/MarkerArray` | Planned |

## Contract checklist for implementation

Before a topic or custom message becomes **Implemented**, document and test:

1. Producer and consumer responsibilities.
2. Exact message type and schema.
3. Header, clock, and timestamp semantics.
4. Frame IDs and required TF relationships.
5. Units and coordinate conventions.
6. Stable actor, event, scenario, and track identity rules where applicable.
7. Valid, unavailable, invalid, and non-applicable value representation.
8. Ordering and array consistency rules.
9. QoS profile and compatibility tests.
10. Cross-language serialization behavior when both `rclcpp` and `rclpy` consume
    the contract.

Changing an approved contract requires explicit review and, when architectural,
an ADR. It must never happen silently inside a producer or consumer.
