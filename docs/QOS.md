# QoS Policy

**Status:** Principles approved; the `/sts/ego/state` M1 profile is implemented and
cross-language verified. The M2.1 `/vehicle/odometry` subscription profile is
approved below and implemented locally in M2.2B; runtime compatibility verification
remains pending. Other per-topic profiles remain unfinalized.

QoS must follow data semantics and be validated in tests. Profiles must not be
selected blindly or copied across every topic.

## Approved principles

- High-rate sensor streams may prioritize freshness with Best Effort, shallow
  queues, and Keep Last where appropriate.
- Important event and semantic outputs should prioritize delivery with Reliable and
  Keep Last.
- Exact queue depths and other policies require measurement and integration testing.
- Producer and consumer profiles must be compatibility-tested.

## Preliminary topic classification

| Topic group | Delivery priority | Approved direction | Unresolved before implementation |
| --- | --- | --- | --- |
| Images, point clouds, high-rate sensor streams | Freshness | Best Effort where appropriate; Keep Last; shallow queue | Exact profile and depth per topic |
| IMU | Timely state | Select by source and consumer semantics | Reliability, depth, and loss behavior |
| `/vehicle/odometry` subscription | Timely streaming state; freshness | KEEP_LAST; depth 5; BEST_EFFORT; VOLATILE; remaining policies default | Implemented locally in M2.2B; runtime compatibility tests pending |
| `/tf` and `/tf_static` | Transform correctness | Follow established TF2 semantics | Validation with chosen ROS 2 deployment |
| `/sts/ego/state` | Timely normalized ego state | Reliable; Keep Last; depth 10; Volatile | None for M1: implementation and cross-language compatibility verified on macOS and Ubuntu CI |
| Normalized actor state | Timely world model | To be specified | Reliability, depth, and late-joiner behavior |
| Actor risk | Timely physical output | To be specified | Reliability, depth, and overload behavior |
| Events and frozen scenario windows | Delivery | Reliable and Keep Last direction | Depth, durability, and replay expectations |
| Rankings, evidence, and semantics | Delivery | Reliable and Keep Last direction | Depth, durability, and duplicate handling |
| Visualization markers | Freshness | To be specified | Profile and behavior when RViz2 is absent |

“Direction” is not a complete QoS contract. No topic is **Implemented** until the
exact profile is recorded alongside its interface and verified.

## Approved M1 topic profile

The following profile is part of the approved `EgoState v1` contract:

| Topic | Reliability | History | Depth | Durability | Status |
| --- | --- | --- | --- | --- | --- |
| `/sts/ego/state` | Reliable | Keep Last | 10 | Volatile | **Implemented and cross-language verified** |

The M1 publisher and subscriber must use compatible QoS settings, and the profile
must be verified as part of cross-language communication testing before the topic is
reported as implemented.

## Approved M2.1 Odometry subscription profile

**Status:** Approved specification; implemented locally in M2.2B; not runtime verified.
This is the `ego_state` node's subscription to `/vehicle/odometry` using
`nav_msgs/Odometry`, not a mandate to change the source publisher's QoS.

| Policy | Approved value |
| --- | --- |
| History | KEEP_LAST |
| Depth | 5 |
| Reliability | BEST_EFFORT |
| Durability | VOLATILE |
| Deadline / lifespan / liveliness | Defaults; no overrides |

Implementation should use the semantics of `rclcpp::SensorDataQoS()`. The
[Jazzy SensorDataQoS declaration](https://github.com/ros2/rclcpp/blob/jazzy/rclcpp/include/rclcpp/qos.hpp)
and [sensor-data profile](https://github.com/ros2/rmw/blob/jazzy/rmw/include/rmw/qos_profiles.h)
define this policy combination. No new deadline, lifespan, or liveliness constants
are selected here.

Odometry is timely streaming state, so freshness is prioritized. Under ROS 2
requested/offered reliability rules, a BEST_EFFORT subscription is compatible with
both BEST_EFFORT and RELIABLE publishers, assuming all other policies are
compatible. Reliability compatibility alone does not guarantee a connection or
lossless delivery; see the [ROS 2 Jazzy QoS compatibility documentation](https://repo.test.ros2.org/en/jazzy/Concepts/Intermediate/About-Quality-of-Service-Settings.html).

| Publisher offers | Subscription requests | Reliability compatible? |
| --- | --- | --- |
| BEST_EFFORT | BEST_EFFORT | Yes |
| RELIABLE | BEST_EFFORT | Yes |

Future integration tests must verify endpoint settings and actual reception from
both publisher profiles, with the other policies held compatible. They must check
accepted-sample outputs, not only node startup, and must not assume Best Effort is
lossless. The complete [M2.1 test matrix](tasks/M2_STREAMING_VEHICLE_CORE.md#m21-deterministic-test-matrix)
also separates callback-delay invariance from delivery loss.

The `/sts/ego/state` output remains Reliable, Keep Last, depth 10, Volatile with
unchanged `EgoState v1` semantics. M1 verification evidence remains unchanged.

## QoS validation

Tests should cover:

- `rclcpp`/`rclpy` compatibility for v0.1 communication;
- expected behavior when publishers and subscribers start in different orders;
- loss and freshness under high-rate sensor load;
- queue behavior during slow consumption;
- reliable delivery of important event and semantic outputs;
- replay behavior with rosbag2; and
- observable dropped-message and latency metrics where applicable.

QoS changes that alter the externally visible interface require explicit review and
must not be made silently in one node.
