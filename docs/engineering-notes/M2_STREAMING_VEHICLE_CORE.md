# M2 Streaming Vehicle Core — Engineering Reference

**Coverage:** Approved M2.0/M2.1 specification and M2.2A package/API design; documentation only.\
**Overall milestone:** v0.2 In Progress; production implementation and runtime
verification remain Planned. M1 / v0.1 remains Implemented / Accepted.

References: [M2 task](../tasks/M2_STREAMING_VEHICLE_CORE.md),
[approved architecture](../DESIGN_SESSION_0_V2.md),
[interface contract](../ROS_INTERFACE_SPEC.md), [QoS policy](../QOS.md),
[online/offline boundary](../ONLINE_VS_OFFLINE.md), and
[unchanged M1 evidence](M1_ROS2_FOUNDATION.md).

## Stateful streaming and causal processing

The future `EgoStateNode` in `sts_ego_state_cpp` will consume `/vehicle/odometry` as
`nav_msgs/Odometry` and publish `/sts/ego/state` as the existing `EgoState v1`.
It is not created by these documentation steps. Unlike the M1 canonical-payload
publisher, a streaming estimator needs history: the last accepted source timestamp
and velocity, and a previous valid acceleration when available. Processing one sample changes the
reference used by the next accepted sample.

Stateful does not mean waiting for a completed scenario window. Every otherwise
accepted input publishes an `EgoState`, even during derivative warm-up. Causal
processing uses only the current sample and accepted past samples. It does not
look ahead, use a centered difference, or revise an earlier output after a future
sample arrives. Replayed inputs can still be processed causally; having a recording
available does not authorize future-data use in the online path.

M2.1 freezes three conceptual estimator states: `EMPTY` has no accepted baseline,
`HAVE_VELOCITY` has a valid velocity/time baseline, and `HAVE_ACCEL` also has valid
acceleration history. Successful continuous samples progress through V → V/A →
V/A/J. Rejected inputs never advance the state. A gap or non-finite acceleration
returns to `HAVE_VELOCITY`; non-finite jerk keeps `HAVE_ACCEL` because its current
acceleration remains usable for the next difference.

## M2.2A implementation architecture — planned

```text
sts_ego_state_cpp
├── ROS-facing EgoStateNode
└── ROS-independent EgoKinematicsEstimator
```

```yaml
package: sts_ego_state_cpp
estimator class: EgoKinematicsEstimator
executable: ego_state_node
ROS node name: ego_state
```

Frozen planned package layout:

```text
sts_ws/src/sts_ego_state_cpp/
├── CMakeLists.txt
├── package.xml
├── include/sts_ego_state_cpp/ego_kinematics_estimator.hpp
├── src/ego_kinematics_estimator.cpp
├── src/ego_state_node.cpp
└── test/test_ego_kinematics_estimator.cpp
```

These paths are documentation, not existing package files. `EgoStateNode` handles
ROS-message contracts, `child_frame_id`, ROS timestamp components, parameter input,
QoS, publication, source-stamp preservation, output-frame assignment, and
diagnostics. It rejects invalid frame/component inputs before touching estimator
history. `EgoKinematicsEstimator` receives `int64_t timestamp_ns` and
`double velocity_mps` and owns the causal numerical behavior. It must not depend
on ROS messages, DDS, executors, logging, or frame names. Frame interpretation has
already been validated by the node; the numerical estimator knows only the agreed
scalar meaning and units.

Ordering and gap comparison stay in integer nanoseconds. Validate the finite
positive `max_sample_gap_s` parameter and convert it once when establishing the
accepted configuration into an integer nanosecond threshold, not for each sample.
The default is exactly `250000000` ns. A gap is strictly `dt_ns > threshold_ns`;
an equal delta is continuous. Convert a positive continuous delta to seconds only
for the unchanged backward finite differences. Conversion must preserve the M2.1
boundary at nanosecond resolution; this design approves no new rounding/clamping
policy. Supported integer timestamp arithmetic must avoid overflow.

### `.hpp` versus `.cpp`

The estimator `.hpp` is the header: it declares the API and types callers need to
compile against. Both the ROS node and numerical tests include that declaration.
The estimator `.cpp` defines the non-inline numerical implementation, compiled
and linked into the callers' build. Keeping calculations there avoids copying
their definitions into every caller. The node `.cpp` contains the ROS adaptation;
the test `.cpp` contains GoogleTest cases. `CMakeLists.txt` will describe build/test
targets and `package.xml` package metadata/dependencies. No build metadata or
source files are created in M2.2A.

### Constructors and `explicit`

A constructor runs when an instance is created and establishes its initial
invariants. For the estimator, that means accepted configuration is available and
history is absent (`EMPTY`) before the first update. It does not consume a sample,
publish, or discover ROS peers. The node constructor initializes its owned
estimator and ROS endpoints; numerical updates remain a separate operation.

If a constructor can accept one configuration scalar, marking it `explicit`
prevents that scalar from silently becoming an estimator object through implicit
conversion or copy-initialization. Deliberate direct construction remains possible.
This protects against an accidental number-to-estimator conversion; it does not
perform parameter validation. See the
[C++ explicit specifier rules](https://eel.is/c++draft/dcl.fct.spec).
Exact constructor signatures and unnecessary private method names are not frozen.

### `std::optional` and one history object

`std::optional<T>` represents either a contained value or no value. The contained
value is owned inside the optional; absence does not need a numeric sentinel. See
the [C++ optional specification](https://eel.is/c++draft/optional.optional).

Use one optional history object containing accepted timestamp, signed velocity,
and optional previous valid acceleration. Use `std::optional<double>` for
unavailable acceleration/jerk inside estimator results. A present `0.0` is a valid
zero; an absent derivative means unavailable. A non-finite derivative must never
be present as a valid value.

| Stored representation | Derived conceptual state |
| --- | --- |
| No history | `EMPTY` |
| History, no acceleration | `HAVE_VELOCITY` |
| History with valid acceleration | `HAVE_ACCEL` |

Independent validity booleans or a separately mutable state enum could disagree
with the data they describe. Deriving state from presence keeps the history and
conceptual state consistent. No previous jerk needs to be stored: the next jerk
uses current and previous valid acceleration. A gap or invalid calculated
acceleration replaces the history with current velocity/time and absent
acceleration. Invalid jerk retains current velocity/time and current valid
acceleration, preserving all M2.1 recovery behavior.

### Update outcomes, diagnostics, and ROS mapping

The result distinguishes accepted, accepted-reset, and rejected updates, with
enough reason information for node diagnostics. Exact enum identifiers and result
member/private method names may be chosen clearly at implementation time.
Accepted includes first-sample warm-up, continuous processing, and invalid jerk
with valid acceleration. Accepted-reset identifies a gap or non-finite calculated
acceleration: there is output, but derivative history restarts. Rejected has no
output and leaves history unchanged, including ordering and non-finite velocity
failures. The estimator returns this information without logging.

The node translates results into the existing wire contract. A present derivative
becomes its numeric `EgoState` value and a true flag; an absent derivative becomes
the `0.0` placeholder and false flag. Accepted velocity is valid. Both accepted
categories publish exactly one message with the source stamp and `base_link` frame;
rejected results publish nothing. The node uses result reasons for diagnostics,
including the required warning for `dt <= 0`.

### Ownership and middleware/domain separation

`EgoStateNode` owns its estimator instance and controls its lifetime. No global or
static estimator state is allowed. Different node instances and unit-test fixtures
therefore start with independent history; one stream cannot contaminate another
through shared estimator storage. This describes ownership, not a requirement for
a particular pointer type or allocation strategy.

Domain logic here is the kinematic calculation and its history/validity contract.
Middleware logic is message handling, discovery, transport, callbacks, and
publication. Separating them lets pure GoogleTest/ament tests call numerical
updates directly, inspect optional outputs and diagnostic outcomes, and verify
the M2.1 matrix without DDS, executor timing, logging, frames, or ROS graph startup.
ament integrates the build and tests; it does not make middleware a dependency of
the estimator. Separate ROS integration tests still verify the node's contracts
and communication. All classes, package files, and tests remain Planned.

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

Validate standard timestamp components before conversion: `sec` is a signed
32-bit integer and `nanosec` is an integer in `[0, 999999999]`. Reject malformed
components without normalization, publication, or state mutation. A nanosecond
component of 1000000000 must not be silently carried into the next second.
The representation comes from the standard
[ROS Time message](https://github.com/ros2/rcl_interfaces/blob/jazzy/builtin_interfaces/msg/Time.msg).

Timestamp ordering must be evaluated at nanosecond resolution. Construct integer
nanoseconds and subtract the previous accepted timestamp before converting a
positive difference to seconds. Subtracting two large floating-point absolute
timestamps could erase a 1 ns interval or change an ordering decision. Exact
timestamp checks do not use the unit-test numeric tolerance. M2.2A also requires
the positive delta's gap comparison to use the configured integer nanosecond
threshold before conversion to seconds for derivatives.

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

Assume the required child frame, finite inputs, and `max_sample_gap_s = 0.25`, which
treats 0.10 s as continuous. Units are s, m/s, m/s², and m/s³. At 0.20 s,
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
`dt <= 0` rule. Wrong or missing `child_frame_id` and malformed timestamp components
likewise reject the sample with no publication or state/history changes. Validate
these inputs before state mutation, evaluate ordering before gap handling, and
handle gaps before calculating derivatives.

## Stream discontinuity

A positive interval can still be too large to represent a continuous stream.
When `dt > max_sample_gap_s`, accept the current velocity, reset derivative
history, and publish acceleration and jerk invalid. That sample becomes the first
sample of a new warm-up sequence. The next continuous sample can supply
acceleration; the following one can supply jerk.

This prevents a difference across a long missing interval from being presented
as a continuous local derivative. The strict condition is `>`, not `>=`.
M2.1 resolves the M2.0 open decision: `max_sample_gap_s = 0.25` seconds is the
approved ROS parameter default. It must be finite and strictly positive; reject
zero, negative, NaN, and infinite values before use. At the default, 250000000 ns
is continuous, while 250000001 ns resets derivative history. The limit is a
continuity decision, not an event threshold. Frame semantics and derivative
equations must not be configurable.

## Odometry subscription QoS

The approved subscription profile is KEEP_LAST, depth 5, BEST_EFFORT, VOLATILE,
with deadline/lifespan/liveliness left at defaults. The implementation should use
the semantics of `rclcpp::SensorDataQoS()`; see the
[Jazzy declaration](https://github.com/ros2/rclcpp/blob/jazzy/rclcpp/include/rclcpp/qos.hpp)
and [QoS policy](../QOS.md). Existing output QoS stays Reliable, Keep Last, depth
10, Volatile.

Odometry is timely streaming state, so freshness is prioritized. A BEST_EFFORT
subscription can match either a BEST_EFFORT or RELIABLE publisher under ROS 2
requested/offered reliability rules when other policies are compatible. This
does not turn delivery into a lossless guarantee. Matching and actual reception
with both publisher profiles require future integration tests; no M2 runtime QoS
verification is claimed here.

## Numerical validity

The schema keeps signed longitudinal velocity, acceleration, and jerk in m/s,
m/s², and m/s³, with one validity flag per quantity. Invalid derivatives are sent
as `0.0` placeholders with false flags. A valid zero acceleration or jerk is sent
with a true flag. Consumers must never infer validity from the numeric value.

Finite input velocity and a positive interval do not guarantee finite computed
derivatives: floating-point differences or division can overflow, and small
intervals amplify noise. M2.1 freezes these recovery rules:

- Non-finite calculated acceleration: accept and retain the current valid velocity
  and timestamp as a new baseline, publish V only, discard acceleration history,
  and transition to `HAVE_VELOCITY`. The next continuous sample can produce A;
  the following one can produce J.
- Valid acceleration but non-finite calculated jerk: publish V/A with jerk invalid,
  retain the current valid acceleration and velocity/time history, and remain
  `HAVE_ACCEL`. The next sample can produce valid jerk from that acceleration.

Neither case is an input rejection, and both publish one `EgoState`. Never mark a
non-finite derivative valid or store it as valid acceleration history. There is
no physical clipping/clamping: a finite extreme value remains finite data, rather
than being forced into an assumed vehicle envelope.

Deterministic unit tests use absolute tolerance `1e-9` for valid computed numeric
quantities. Flags, invalid `0.0` placeholders, timestamps, frames, state transitions,
and publication counts are exact checks. M1's exactly representable canonical
fixture values do not justify exact equality for ordinary calculated derivatives.

Future verification must check numerical outputs, validity, stamps, frames, and
publication counts. It must also show that rejected samples leave subsequent
results unchanged and that gaps restart warm-up. Node startup alone is insufficient.

## Deterministic verification specification

The complete [M2.1 test matrix](../tasks/M2_STREAMING_VEHICLE_CORE.md#m21-deterministic-test-matrix--specified-not-implemented)
defines inputs and expected outputs for constant velocity/acceleration, braking
onset, irregular sampling, signed reverse motion, first timestamp zero,
duplicate/decreasing timestamps, the exact 0.25 s boundary, larger gaps and renewed
warm-up, NaN/+Inf/-Inf inputs, wrong/missing child frame, rejected-sample history,
malformed components and nanosecond ordering, non-finite derivatives, callback-delay
invariance, parameter validation, and both publisher reliability profiles.

Overflow fixtures deliberately use extreme finite float64 values. Powers of two
and 0.125 s intervals allow exact finite acceleration references and explicit
overflow, without physical limits or a new numerical tolerance. Compare subsequent
outputs to verify acceleration-reset recovery and retention after invalid jerk.
Run rejection cases from all three states and compare later outputs with a control
stream that omits the rejected sample.

Callback-delay invariance assumes identical source samples arrive in the same
order. Best Effort loss can change the delivered sequence; it must not be confused
with using callback time in the derivative equations. Numerical unit tests stay
independent of transport; ROS integration verifies reception and publication.

All tests remain Planned. This note records no production implementation or
measured baseline correctness/noise sensitivity.

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

Documentation through M2.2A specifies no `BrakeEvent` fields or event thresholds,
creates no ROS packages or production code, and changes neither `EgoState v1`
nor M1 evidence.
