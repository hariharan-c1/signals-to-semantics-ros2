# M2 Streaming Vehicle Core — Engineering Reference

**Coverage:** M2.0 approved design and documentation only.\
**Overall milestone:** v0.2 In Progress; production implementation and runtime
verification remain Planned. M1 / v0.1 remains Implemented / Accepted.

References: [M2 task](../tasks/M2_STREAMING_VEHICLE_CORE.md),
[approved architecture](../DESIGN_SESSION_0_V2.md),
[interface contract](../ROS_INTERFACE_SPEC.md), [QoS policy](../QOS.md),
[online/offline boundary](../ONLINE_VS_OFFLINE.md), and
[unchanged M1 evidence](M1_ROS2_FOUNDATION.md).

## Stateful streaming and causal processing

The future `sts_ego_state_cpp` node will consume `/vehicle/odometry` as
`nav_msgs/Odometry` and publish `/sts/ego/state` as the existing `EgoState v1`.
It is not created in M2.0. Unlike the M1 canonical-payload publisher, a streaming
estimator needs history: the last accepted source timestamp and velocity, and a
previous valid acceleration when available. Processing one sample changes the
reference used by the next accepted sample.

Stateful does not mean waiting for a completed scenario window. Every otherwise
accepted input publishes an `EgoState`, even during derivative warm-up. Causal
processing uses only the current sample and accepted past samples. It does not
look ahead, use a centered difference, or revise an earlier output after a future
sample arrives. Replayed inputs can still be processed causally; having a recording
available does not authorize future-data use in the online path.

## Source timestamp versus callback time

`Odometry.header.stamp` is the measurement/source time. Callback time is when the
executor runs the receive callback. Transport delay, queueing, and executor
scheduling can change callback spacing without changing the physical sample
interval. Derivatives therefore use source timestamp differences, not callback
arrival intervals or the node's current clock.

Preserve the input timestamp exactly in `EgoState.header.stamp`. With identical
source samples and arrival order, callback delay should not change the numerical
baseline. This is a future verification requirement, not a measured result.

Timestamp zero is permitted as an initial simulation/replay timestamp. History
presence must be represented separately from the numeric timestamp; treating zero
as an uninitialized sentinel would incorrectly discard the first sample. The M1
subscriber's nonzero check belongs to its synthetic canonical fixture; it does
not redefine the M2 source-time contract or require changing M1 evidence.

## Odometry twist frame semantics and velocity source

Odometry has separate frame ownership: pose belongs to `header.frame_id`, while
twist belongs to `child_frame_id`. Require `child_frame_id == "base_link"` before
interpreting `twist.twist.linear.x` as the signed longitudinal ego velocity in m/s.
Positive X is forward under `EgoState v1`. This is a scalar velocity, not an
unsigned speed or a 3D velocity magnitude.

The output frame remains `base_link`. M2 performs no TF2 conversion; a mismatched
or missing child frame fails the precondition and cannot be repaired by labeling
its numeric X component as `base_link`. The input pose frame does not change the
twist frame's meaning.

The original thesis derived kinematics from ego pose because direct vehicle
telemetry was not assumed available. Here Odometry twist already supplies
velocity, so the primary M2 source is `twist.twist.linear.x`.
`nav_msgs/Odometry.pose` is intentionally not used to reconstruct velocity in M2.
Pose-derived reconstruction is deferred to a later offline-parity/replay/regression
study and must not be implemented in M2. That later study can compare source and
algorithm differences explicitly; it must not silently substitute pose-derived
velocity into this baseline.

## Backward finite differences and derivative warm-up

For continuous accepted samples, with source time in seconds:

```text
dt  = t_k - t_(k-1)
a_k = (v_k - v_(k-1)) / dt
j_k = (a_k - a_(k-1)) / dt
```

Acceleration compares current velocity with the previous accepted velocity.
Jerk compares current valid acceleration with the preceding valid acceleration,
using their source timestamps. Acceleration is associated with `t_k`; on an
uninterrupted stream the acceleration timestamp interval equals `dt`. Both are
backward differences and remain causal for unequal positive sample intervals.

One velocity alone provides no velocity change, so the first accepted sample has
valid velocity and invalid acceleration/jerk. The second continuous accepted
sample provides one acceleration, but no acceleration change: velocity and
acceleration are valid, jerk is invalid. The third and subsequent continuous
samples can have all three valid, subject to numerical validity.

The deterministic reference is:

```ini
t=0.00, v=10.00 -> a invalid, j invalid
t=0.10, v=10.00 -> a=0.0,  j invalid
t=0.20, v=9.80  -> a=-2.0, j=-20.0
t=0.30, v=9.60  -> a=-2.0, j=0.0
t=0.40, v=9.40  -> a=-2.0, j=0.0
```

Assume the required child frame, finite inputs, and a future gap limit that treats
0.10 s as continuous. Units are s, m/s, m/s², and m/s³. At 0.20 s,
`(9.80 - 10.00) / 0.10 = -2.0`; the previous acceleration was zero, giving
`(-2.0 - 0.0) / 0.10 = -20.0`. Constant deceleration thereafter gives zero jerk.
These values describe the approved mathematics, not executed test evidence.

## Invalid inputs and non-monotonic timestamps

Compute `dt` against the last accepted timestamp. For `dt <= 0`, reject the
incoming sample, warn, publish nothing, and leave all estimator history unchanged.
A duplicate timestamp would divide by zero; a decreasing timestamp would reverse
the temporal interval and break the causal sequence. Do not sort, overwrite
history, or automatically restart on these samples. A later valid input is still
compared with the last accepted sample.

Non-finite longitudinal velocity (NaN or either infinity) is rejected without
publishing or updating estimator state. It must not contaminate later derivatives.
Zero time by itself is allowed; a repeated zero after an accepted zero fails the
`dt <= 0` rule. Malformed timestamp handling beyond these approved cases remains
to be finalized in M2.1 before implementation.

## Stream discontinuity

A positive interval can still be too large to represent a continuous stream.
When `dt > max_sample_gap_s`, accept the current velocity, reset derivative
history, and publish acceleration and jerk invalid. That sample becomes the first
sample of a new warm-up sequence. The next continuous sample can supply
acceleration; the following one can supply jerk.

This prevents a difference across a long missing interval from being presented
as a continuous local derivative. The strict condition is `>`, not `>=`.
`max_sample_gap_s` has no approved value or default in M2.0; M2.1 must choose and
justify it. Exact Odometry input QoS also remains an M2.1 open decision. Existing
output QoS stays Reliable, Keep Last, depth 10, Volatile.

## Numerical validity

The schema keeps signed longitudinal velocity, acceleration, and jerk in m/s,
m/s², and m/s³, with one validity flag per quantity. Invalid derivatives are sent
as `0.0` placeholders with false flags. A valid zero acceleration or jerk is sent
with a true flag. Consumers must never infer validity from the numeric value.

Finite input velocity and a positive interval do not guarantee finite computed
derivatives: floating-point differences or division can overflow, and small
intervals amplify noise. Non-finite results cannot be marked valid. M2.1 must
finalize numerical edge-case behavior and justified tolerances before production
implementation, preserving `EgoState v1` semantics. Decimal reference values need
tolerance-based comparisons; M1's exactly representable fixture values do not
justify exact equality for calculated derivatives.

Future verification must check numerical outputs, validity, stamps, frames, and
publication counts. It must also show that rejected samples leave subsequent
results unchanged and that gaps restart warm-up. Node startup alone is insufficient.

## Why filtering follows baseline verification

The first implementation uses no smoothing/filtering. A deterministic raw causal
baseline makes the timestamp, sign, finite-difference, and state-transition
behavior directly inspectable. Filtering would add its own history, delay, and
parameter effects before that baseline is established.

Differentiation can amplify measurement noise, especially in jerk. First verify
baseline numerical correctness and measure noise sensitivity. Only then evaluate
filtering against the baseline, documenting any causal behavior or explicit fixed
latency as required by the online/offline contract. No filtering constants or
performance claims are approved here.

M2.0 specifies no `BrakeEvent` fields or event thresholds, creates no ROS packages
or production code, and changes neither `EgoState v1` nor M1 evidence.
