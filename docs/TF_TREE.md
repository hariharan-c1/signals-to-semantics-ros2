# TF Tree

**Status:** Planned conceptual TF model; no transforms are implemented yet.

## Approved conceptual tree

```text
map
└── odom
    └── base_link
        ├── front_camera_link
        │   └── front_camera_optical
        ├── lidar_link
        └── imu_link
```

This tree establishes the intended ownership relationships but does not yet approve
transform publishers, calibration values, axis conventions beyond standard ROS
message expectations, or the normalized calculation frame.

## Required invariants

- Every spatial value has explicit frame ownership.
- Every message carrying spatial data identifies or inherits an unambiguous frame.
- Sensor observations are transformed into a normalized frame before physical risk
  calculations.
- Transform timestamps must be compatible with the sensor or actor observation
  being processed.
- Replay, CARLA ground truth, and perception must obey the same downstream frame
  contract.
- Missing, stale, or unavailable transforms must be handled explicitly rather than
  silently treating coordinates as if they share a frame.

## Expected frame roles

- `map`: conceptual global frame at the top of the initial tree.
- `odom`: locally continuous odometry frame beneath `map`.
- `base_link`: ego-vehicle body frame.
- `front_camera_link`: physical front-camera frame.
- `front_camera_optical`: optical frame used by camera projection.
- `lidar_link`: LiDAR sensor frame.
- `imu_link`: IMU sensor frame.

The descriptions above state conceptual roles only. Exact origins, orientation
conventions, static extrinsics, transform authority, and whether a risk computation
uses `base_link`, `odom`, or another approved normalized frame must be specified and
tested before implementation.

## Validation plan

When TF2 enters scope in v0.4, validation should include:

- tree connectivity and single-parent checks;
- timestamp and transform-availability behavior;
- known-point transformations with numerical tolerances;
- camera optical projection consistency;
- replay determinism; and
- equivalent downstream actor coordinates for GT and perception inputs that
  describe the same physical state.
