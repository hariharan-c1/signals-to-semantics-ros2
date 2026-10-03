# ADR-007: Prefer Standard ROS Interfaces

- **Status:** Accepted
- **Decision scope:** ROS message design
- **Implementation status:** M1 custom-interface scope Implemented / Accepted; M2.2B standard Odometry subscription implemented locally; ROS end-to-end verification pending; other source integrations Planned
- **Authority:** [`../DESIGN_SESSION_0_V2.md`](../DESIGN_SESSION_0_V2.md)

## Context

The system consumes common sensor, ego-motion, geometry, transform, and visualization
data alongside scenario-specific concepts. Duplicating established ROS semantics in
custom messages would increase integration and maintenance cost.

## Decision

Use established standard ROS messages whenever one fits, including the planned use
of:

- `sensor_msgs/Image`;
- `sensor_msgs/CameraInfo`;
- `sensor_msgs/PointCloud2`;
- `sensor_msgs/Imu`;
- `nav_msgs/Odometry`;
- `geometry_msgs/Pose` and `geometry_msgs/Twist` where appropriate; and
- `visualization_msgs/MarkerArray`.

Create custom interfaces only for scenario-specific concepts such as normalized ego
and actor states, actor risk, events, scenario windows, ranked actors, and scenario
semantics.

## Consequences

- M1 must justify and fully specify its first custom message before generation.
- Custom contracts must define timestamps, frames, units, validity, and
  cross-language behavior.
- Standard tools and components can consume standard sensor and visualization
  topics.
- CARLA-specific data remains behind an adapter instead of becoming a shared
  downstream interface.
- Any departure from a suitable standard message requires explicit review.

The current planned catalogue is maintained in
[`../ROS_INTERFACE_SPEC.md`](../ROS_INTERFACE_SPEC.md).
