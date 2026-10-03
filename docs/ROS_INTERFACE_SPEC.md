# ROS Interface Specification

**Status:** Interface catalogue; `EgoState v1` is **Implemented and cross-language
verified**. Other wire schemas are not yet approved or implemented.
**Current milestone:** v0.2 Streaming Vehicle Core — In Progress; M2.2B ego-state
package implemented and estimator unit-tested locally, with ROS end-to-end
verification pending; see the [M2 task](tasks/M2_STREAMING_VEHICLE_CORE.md).
v0.1 is Implemented; the first custom message is generated, built, and tested
across C++ and Python on macOS and Ubuntu 24.04 CI. `EgoState v1` remains unchanged.

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
| `/vehicle/odometry` | `nav_msgs/Odometry` | Ego odometry | M2.2B subscription implemented locally; source/end-to-end verification pending |
| `/tf` | Standard TF2 transform messages | Dynamic transforms | Planned |
| `/tf_static` | Standard TF2 transform messages | Static transforms | Planned |

`geometry_msgs/Pose`, `geometry_msgs/Twist`, and
`visualization_msgs/MarkerArray` are also preferred where their standard semantics
fit.

## Planned custom messages

| Interface | Scenario-specific purpose | Status |
| --- | --- | --- |
| `EgoState` | Normalized ego state for downstream processing | **Implemented and cross-language verified** (v1) |
| `ActorState` | One normalized tracked or ground-truth actor state | Planned; schema not approved |
| `ActorStateArray` | Common actor-provider boundary | Planned; schema not approved |
| `ActorRisk` | Risk quantities for one actor | Planned; schema not approved |
| `ActorRiskArray` | Actor-relative risk output collection | Planned; schema not approved |
| `BrakeEvent` | Braking-event representation | Planned; schema not approved |
| `ScenarioWindow` | Frozen pre-event, event, and post-event context | Planned; schema not approved |
| `RankedActor` | Ranked actor result from S2/S3 | Planned; schema not approved |
| `RankedActorArray` | Top-K/ranked actor collection | Planned; schema not approved |
| `ScenarioSemantic` | Constrained semantic enrichment result | Planned; schema not approved |

`EgoState v1` is the first custom message selected for v0.1. Its approved contract
is frozen below. No field layout is implied for any of the other names above.

## EgoState v1 approved contract

**Status:** **Implemented and cross-language verified**. The `.msg` file is
generated and built, and the installed C++ publisher → Python subscriber contract
exchange passed automated testing on macOS and Ubuntu 24.04 CI.

| Contract item | Approved value |
| --- | --- |
| Topic | `/sts/ego/state` |
| Message type | `sts_interfaces/EgoState` |
| Contract version | `EgoState v1` |
| Timestamp | `header.stamp` is the measurement/source timestamp. For the M1 synthetic publisher, it may be the ROS clock time at sample creation. |
| Frame | `header.frame_id` is `base_link`. |
| QoS | Reliable, Keep Last, depth 10, Volatile; see [`QOS.md`](QOS.md). |

Approved message schema:

```text
std_msgs/Header header

float64 longitudinal_velocity_mps
float64 longitudinal_acceleration_mps2
float64 longitudinal_jerk_mps3

bool velocity_valid
bool acceleration_valid
bool jerk_valid
```

Approved field semantics:

- The longitudinal direction is the ego vehicle X axis; positive is forward.
- `longitudinal_velocity_mps` is a signed scalar velocity in metres per second.
- `longitudinal_acceleration_mps2` is signed acceleration in metres per second
  squared. Negative values represent deceleration along the longitudinal axis.
- `longitudinal_jerk_mps3` is signed jerk in metres per second cubed.
- Each validity flag applies to its corresponding numeric field. A numeric field
  whose validity flag is `false` must not be interpreted by consumers.
- Producers use `0.0` for an invalid or unavailable numeric field as a transport
  placeholder only. The corresponding validity flag, not the placeholder value,
  determines whether the quantity has semantic meaning.

For M1, the producer must populate the approved header, numeric fields, and validity
flags without changing their meanings. The consumer must preserve and verify those
values across ROS serialization and must obey the validity rules. Either language
may fill the producer or consumer role; the contract is identical for `rclcpp` and
`rclpy`.

Canonical cross-language M1 test payload:

```ini
frame_id = "base_link"
longitudinal_velocity_mps = 13.5
longitudinal_acceleration_mps2 = -2.25
longitudinal_jerk_mps3 = -4.0
velocity_valid = true
acceleration_valid = true
jerk_valid = true
```

The fixture does not freeze a numeric `header.stamp`; the test must set and verify a
source timestamp consistent with the timestamp contract above.

`EgoState v1` intentionally does not contain pose, yaw, yaw rate, lateral velocity,
or 3D velocity. Those quantities are not required by this first
scenario-intelligence contract, and generic pose and twist information already has
standard ROS message representations. The v1 field names, order, types, units, and
semantics are frozen for M1. A change requires explicit contract review and matching
test updates; it must not be made silently in a producer or consumer.

## Planned Signals-to-Semantics topics

| Topic | Intended payload | Status |
| --- | --- | --- |
| `/sts/ego/state` | `sts_interfaces/EgoState` | **Implemented and cross-language verified** (v1) |
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
