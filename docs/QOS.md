# QoS Policy

**Status:** Principles approved; per-topic QoS profiles are not yet finalized or
implemented.

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
| IMU and odometry | Timely state | Select by source and consumer semantics | Reliability, depth, and loss behavior |
| `/tf` and `/tf_static` | Transform correctness | Follow established TF2 semantics | Validation with chosen ROS 2 deployment |
| Normalized ego and actor state | Timely world model | To be specified | Reliability, depth, and late-joiner behavior |
| Actor risk | Timely physical output | To be specified | Reliability, depth, and overload behavior |
| Events and frozen scenario windows | Delivery | Reliable and Keep Last direction | Depth, durability, and replay expectations |
| Rankings, evidence, and semantics | Delivery | Reliable and Keep Last direction | Depth, durability, and duplicate handling |
| Visualization markers | Freshness | To be specified | Profile and behavior when RViz2 is absent |

“Direction” is not a complete QoS contract. No topic is **Implemented** until the
exact profile is recorded alongside its interface and verified.

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
