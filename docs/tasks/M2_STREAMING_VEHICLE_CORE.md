# M2 — Streaming Vehicle Core

**Status:** In Progress — M2.0 approved design and documentation; runtime implementation remains Planned.\
**Release:** v0.2 Streaming Vehicle Core\
**Branch:** `feat/streaming-vehicle-core`\
**Architecture authority:** [`../DESIGN_SESSION_0_V2.md`](../DESIGN_SESSION_0_V2.md)

## Objective and delivery boundary

Specify the deterministic, causal conversion of an ego odometry stream into the
existing `EgoState v1` contract before implementing the production node.
M2.0 is documentation-only: this task and the
[engineering note](../engineering-notes/M2_STREAMING_VEHICLE_CORE.md) record the
approved decisions and future verification requirements. They do not establish
runtime or numerical test evidence.

M1 / v0.1 ROS 2 Foundation is **Implemented / Accepted**; its contracts and
[evidence](../engineering-notes/M1_ROS2_FOUNDATION.md) remain unchanged. M2 / v0.2
is **In Progress**. Later milestones remain **Planned**. The release roadmap also
includes event detection and rolling temporal state, but M2.0 does not approve
`BrakeEvent` fields, event thresholds, or event/window implementations.

Relevant decisions: [ADR-002](../adr/ADR-002-cpp-python-boundary.md),
[ADR-004](../adr/ADR-004-online-vs-offline.md), and
[ADR-007](../adr/ADR-007-standard-ros-interfaces.md). Applicable contracts:
[ROS interfaces](../ROS_INTERFACE_SPEC.md), [QoS](../QOS.md),
[TF tree](../TF_TREE.md), and [online/offline modes](../ONLINE_VS_OFFLINE.md).

## Approved input and output contract

| Item | M2.0 decision |
| --- | --- |
| Input topic | `/vehicle/odometry` |
| Input type | `nav_msgs/Odometry` |
| Future production node | `sts_ego_state_cpp`, C++ / `rclcpp`; not created in M2.0 |
| Primary longitudinal velocity source | `twist.twist.linear.x`, signed m/s |
| Required twist frame | `child_frame_id == "base_link"` before interpreting `linear.x` as ego longitudinal velocity |
| Pose | `nav_msgs/Odometry.pose` is intentionally not used to reconstruct velocity in M2 |
| TF2 | No TF2 conversion in M2 |
| Measurement/source time | Input `header.stamp`; zero is permitted as an initial simulation/replay timestamp |
| Output topic and type | `/sts/ego/state`, `sts_interfaces/EgoState` (`EgoState v1`) |
| Output timestamp | Preserve the input `header.stamp` exactly in `EgoState.header.stamp` |
| Output frame | `EgoState.header.frame_id = "base_link"` |
| Output QoS | Existing Reliable, Keep Last, depth 10, Volatile profile remains unchanged |
| Input QoS | Exact Odometry input profile remains an M2.1 open decision |
| Gap limit | `max_sample_gap_s` remains an M2.1 open decision; no value or default approved |
| Filtering | No smoothing/filtering in the first implementation |

Odometry pose is expressed in `header.frame_id`; its twist is expressed in
`child_frame_id`. The output body frame follows the validated twist frame, not the
pose frame. A different or missing child frame cannot be treated as `base_link` or
fixed by merely relabeling the output; it fails the input precondition. M2 performs
no transform lookup or conversion to accommodate it.

The original thesis derived kinematics from ego pose because direct vehicle
telemetry was not assumed available. The ROS input already supplies velocity
through Odometry twist. Pose-derived reconstruction is deferred to a later
offline-parity/replay/regression study and must not be implemented in M2.

## Approved causal estimator behavior

State is based on accepted source samples. Let `t_k` be the current source time,
`v_k` its longitudinal velocity, and `dt = t_k - t_(k-1)` in seconds, measured
against the last accepted sample. For continuous samples:

```text
a_k = (v_k - v_(k-1)) / (t_k - t_(k-1))
j_k = (a_k - a_(k-1)) / (t_k - t_(k-1))
```

Acceleration is a causal backward finite difference. Jerk is a causal backward
finite difference between valid accelerations associated with their source sample
times. Only current and previously accepted information may contribute; future
samples, centered differences, and completed-window smoothing are excluded.

| Incoming sample / state | Required behavior |
| --- | --- |
| First accepted sample | Publish valid velocity; acceleration and jerk invalid. Establish the velocity/time history. |
| Second accepted continuous sample | Publish valid velocity and acceleration; jerk invalid. Establish the first valid acceleration. |
| Subsequent continuous samples | Publish all three valid when the calculated quantities are numerically valid. |
| `dt <= 0` | Reject the incoming sample, warn, publish nothing, and leave estimator history unchanged. This covers duplicate and decreasing source timestamps. |
| Non-finite longitudinal velocity | Reject without updating estimator state or publishing. |
| `dt > max_sample_gap_s` | Accept current velocity, reset derivative history, publish acceleration and jerk invalid, and begin warm-up again from this sample. |
| Every otherwise accepted sample | Publish one `EgoState` with the preserved source stamp and `base_link` frame. |

Zero time must be distinguished from absence of estimator history: a first sample
at `t=0` is allowed; a later duplicate at `t=0` is rejected by `dt <= 0`. Rejected
samples do not become the reference for any later difference. A gap equal to the
eventually approved limit does not satisfy the strict `>` discontinuity condition.
After a gap reset, the next continuous accepted sample has valid acceleration and
invalid jerk; the following continuous sample can have both derivatives valid.

The existing per-field validity flags and SI units remain unchanged. An invalid
acceleration or jerk uses `0.0` only as the `EgoState v1` transport placeholder;
consumers must check the matching flag. A physical zero with a true flag is valid.
Non-finite results must never be advertised as valid. M2.1 must finalize numerical
edge-case handling and test tolerances before implementation without altering v1.

## Deterministic reference sequence

Assume `child_frame_id = "base_link"`, finite velocities, and an eventual
`max_sample_gap_s` that classifies each 0.10 s interval as continuous. Every listed
sample publishes with velocity valid and with its input source timestamp preserved.
Times are seconds, velocity is m/s, acceleration is m/s², and jerk is m/s³.

```ini
t=0.00, v=10.00 -> a invalid, j invalid
t=0.10, v=10.00 -> a=0.0,  j invalid
t=0.20, v=9.80  -> a=-2.0, j=-20.0
t=0.30, v=9.60  -> a=-2.0, j=0.0
t=0.40, v=9.40  -> a=-2.0, j=0.0
```

These are mathematical reference values, not executed test results. Future tests
must compare computed values using justified numerical tolerances, and check
timestamps, frames, validity flags, publication counts, and history behavior.

## M2.1 open decisions and future verification

- Select and justify the exact `/vehicle/odometry` input QoS and compatibility tests.
- Select and justify `max_sample_gap_s`, including its configuration and validation.
- Finalize malformed timestamp and non-finite derivative handling, warning behavior
  for failed frame preconditions, and numerical tolerances before implementation.
- Specify tests for the reference sequence, constant velocity, variable positive
  intervals, first-sample zero time, duplicate/decreasing timestamps, non-finite
  velocity, child-frame mismatch, and gap resets. Rejection tests must demonstrate
  unchanged history through subsequent outputs; gap tests must check renewed warm-up.
- Verify invariance to callback delay when source samples and arrival order are the
  same, and measure baseline numerical correctness and noise sensitivity.

Filtering is evaluated only after the deterministic raw causal baseline has been
verified and its noise sensitivity measured. No filtering constants are approved.
Later comparisons must state information horizons and avoid future-data leakage.

## M2.0 documentation acceptance

- [x] Record the approved source, frame, time, derivative, rejection, gap, warm-up,
  publication, and filtering decisions without changing `EgoState v1`.
- [x] Explain the streaming concepts and thesis-to-ROS velocity-source decision in
  the engineering note, including the deterministic sequence.
- [x] Identify input QoS and `max_sample_gap_s` as M2.1 open decisions.
- [x] Distinguish implemented M1, in-progress M2 design, and planned runtime work.
- [x] Create no ROS package, production code, new schema, dependency, dataset,
  checkpoint, Dockerization, or placeholder component.

M2.0 documentation completion does not mean that M2 runtime acceptance criteria
have passed. Do not create `sts_ego_state_cpp`, change `EgoState v1`, alter M1
evidence, stage files, or commit as part of this task. Run `git diff --check` and
report the created/modified documents and corrected stale status statements.
