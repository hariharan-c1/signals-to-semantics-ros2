# M2 — Streaming Vehicle Core

**Status:** In Progress — approved M2.0/M2.1 specification and M2.2A package/API design; runtime implementation remains Planned.\
**Release:** v0.2 Streaming Vehicle Core\
**Branch:** `feat/streaming-vehicle-core`\
**Architecture authority:** [`../DESIGN_SESSION_0_V2.md`](../DESIGN_SESSION_0_V2.md)

## Objective and delivery boundary

Specify the deterministic, causal conversion of an ego odometry stream into the
existing `EgoState v1` contract before implementing the production node.
M2.0, M2.1, and M2.2A are documentation-only: this task and the
[engineering note](../engineering-notes/M2_STREAMING_VEHICLE_CORE.md) record the
approved decisions and future verification requirements. They do not establish
runtime or numerical test evidence.

M1 / v0.1 ROS 2 Foundation is **Implemented / Accepted**; its contracts and
[evidence](../engineering-notes/M1_ROS2_FOUNDATION.md) remain unchanged. M2 / v0.2
is **In Progress**. Later milestones remain **Planned**. The release roadmap also
includes event detection and rolling temporal state, but these documentation steps
do not approve `BrakeEvent` fields, event thresholds, or event/window implementations.

Relevant decisions: [ADR-002](../adr/ADR-002-cpp-python-boundary.md),
[ADR-004](../adr/ADR-004-online-vs-offline.md), and
[ADR-007](../adr/ADR-007-standard-ros-interfaces.md). Applicable contracts:
[ROS interfaces](../ROS_INTERFACE_SPEC.md), [QoS](../QOS.md),
[TF tree](../TF_TREE.md), and [online/offline modes](../ONLINE_VS_OFFLINE.md).

## Approved input and output contract

| Item | Approved M2.0/M2.1 decision |
| --- | --- |
| Input topic | `/vehicle/odometry` |
| Input type | `nav_msgs/Odometry` |
| Future production package / node | `sts_ego_state_cpp`, C++ / `rclcpp`; ROS node `ego_state`; not created in these documentation steps |
| Primary longitudinal velocity source | `twist.twist.linear.x`, signed m/s |
| Required twist frame | `child_frame_id == "base_link"` before interpreting `linear.x` as ego longitudinal velocity |
| Pose | `nav_msgs/Odometry.pose` is intentionally not used to reconstruct velocity in M2 |
| TF2 | No TF2 conversion in M2 |
| Measurement/source time | Input `header.stamp`; zero is permitted as an initial simulation/replay timestamp |
| Output topic and type | `/sts/ego/state`, `sts_interfaces/EgoState` (`EgoState v1`) |
| Output timestamp | Preserve the input `header.stamp` exactly in `EgoState.header.stamp` |
| Output frame | `EgoState.header.frame_id = "base_link"` |
| Output QoS | Existing Reliable, Keep Last, depth 10, Volatile profile remains unchanged |
| Input QoS | KEEP_LAST, depth 5, BEST_EFFORT, VOLATILE; deadline/lifespan/liveliness remain defaults; use `rclcpp::SensorDataQoS()` semantics |
| Gap limit | `max_sample_gap_s = 0.25` s; finite positive ROS parameter; discontinuity iff `dt > max_sample_gap_s` |
| Filtering | No smoothing/filtering in the first implementation |
| Physical clipping/clamping | None |
| Deterministic unit-test absolute tolerance | `1e-9` for valid computed numeric quantities |

Odometry pose is expressed in `header.frame_id`; its twist is expressed in
`child_frame_id`. The output body frame follows the validated twist frame, not the
pose frame. A different or missing child frame cannot be treated as `base_link` or
fixed by merely relabeling the output; it fails the input precondition. M2 performs
no transform lookup or conversion to accommodate it.

The original thesis derived kinematics from ego pose because direct vehicle
telemetry was not assumed available. The ROS input already supplies velocity
through Odometry twist. Pose-derived reconstruction is deferred to a later
offline-parity/replay/regression study and must not be implemented in M2.

## Finalized M2.1 subscription and parameter contract

Odometry is timely streaming state: prioritize freshness with the semantics of
`rclcpp::SensorDataQoS()`, retaining at most the latest five samples in the
subscription history. A BEST_EFFORT subscription is compatible with both
BEST_EFFORT and RELIABLE publishers under ROS 2 requested/offered reliability
rules, provided the other QoS policies are compatible. Compatibility does not
guarantee receipt of every sample. The approved profile and future compatibility
tests are recorded in [QoS policy](../QOS.md).

The ROS parameter `max_sample_gap_s` has approved default `0.25` seconds and must
be finite and strictly positive. Reject zero, negative, NaN, or infinite parameter
values before they can be used. The discontinuity comparison is strictly `>`;
an exact 0.25 s interval is continuous with the default parameter. Frame semantics
and derivative equations are fixed contracts, not configurable parameters.

## M2.2A approved package/API design — not implemented

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

Frozen planned layout (paths are specifications, not files created here):

```text
sts_ws/src/sts_ego_state_cpp/
├── CMakeLists.txt
├── package.xml
├── include/sts_ego_state_cpp/ego_kinematics_estimator.hpp
├── src/ego_kinematics_estimator.cpp
├── src/ego_state_node.cpp
└── test/test_ego_kinematics_estimator.cpp
```

### Responsibility and API boundary

- `EgoStateNode` is ROS-facing. It validates `child_frame_id`, the ROS timestamp
  representation, parameter input, and message contracts before forwarding data.
  It owns subscription/publication, output-frame assignment, source-stamp
  preservation, and logging/diagnostics. Failed frame/component validation does
  not call the estimator or mutate its history.
- `EgoKinematicsEstimator` accepts `int64_t timestamp_ns` and
  `double velocity_mps`. It must not depend on ROS messages, DDS, executors,
  logging, or frame names. It owns numerical validation, ordering, continuity,
  history, and causal finite-difference behavior. The input velocity has already
  been checked for the required twist frame by the node.
- Ordering and gap comparisons use integer nanoseconds. Validate
  `max_sample_gap_s` as finite and positive, then convert it once when establishing
  the accepted configuration into an integer nanosecond threshold, not on every
  sample. The default `0.25` s corresponds exactly to `250000000` ns. Discontinuity
  is strictly `dt_ns > threshold_ns`; only a positive continuous delta is converted
  to seconds for derivative calculation. Conversion must preserve M2.1 boundary
  behavior at nanosecond resolution; no new rounding/clamping policy is approved.
  Integer timestamp arithmetic must avoid overflow for supported inputs.
- `EgoStateNode` owns its estimator instance. No global/static estimator state is
  permitted; independent nodes and test fixtures have independent history.

### History and optional quantities

Use one optional history object, rather than independent mutable validity
booleans. That object contains the last accepted timestamp in nanoseconds, signed
velocity in m/s, and an optional previous valid acceleration in m/s². Use
`std::optional<double>` for unavailable acceleration/jerk inside the estimator;
absence is different from a valid physical zero. Never store a non-finite
acceleration as present valid history.

| Representation | Conceptual state |
| --- | --- |
| No history object | `EMPTY` |
| History present, acceleration absent | `HAVE_VELOCITY` |
| History present, valid acceleration present | `HAVE_ACCEL` |

These states emerge from history presence and its optional acceleration. Do not
maintain a separate mutable state enum that can disagree with history. Jerk is an
optional output; a previous jerk is not required in derivative history.

### Update result and ROS mapping

The update result must distinguish accepted, accepted-reset, and rejected outcomes
and carry enough reason information for node diagnostics. These are semantic
categories; exact enum identifiers, result/member names, and private method names
are not frozen here.

| Outcome | Required result and history behavior |
| --- | --- |
| Accepted | Output contains current valid velocity/time and optional valid A/J. Includes first-sample warm-up, ordinary continuous updates, and valid A with non-finite J; retain history as specified by M2.1. |
| Accepted-reset | Output contains current valid velocity/time, with A/J absent. A large gap or non-finite calculated A clears derivative history and retains the current velocity/time baseline. |
| Rejected | No output and no history mutation, including duplicate/decreasing timestamps and non-finite input velocity. |

The estimator reports outcomes/reasons without logging or publishing. The node
emits the required warning for `dt <= 0`, diagnoses other outcomes, and publishes
one `EgoState` for each accepted or accepted-reset result. It maps present A/J
optionals to numeric fields with true validity flags, and absent optionals to
`0.0` placeholders with false flags. Accepted velocity is valid. Rejected results
produce no ROS publication. The existing source stamp, `base_link` frame, units,
field order, and QoS contract remain unchanged.

### Planned testing and implementation discretion

Pure estimator logic will be tested with GoogleTest through ament, using
`test/test_ego_kinematics_estimator.cpp`. Unit tests must link only what is needed
for the ROS-independent estimator and test harness; they require no DDS, ROS graph
startup, ROS messages, or executor. ament supplies build/test integration, not a
middleware runtime dependency for the estimator. ROS integration tests separately
verify message validation, serialization, publication, callback-delay invariance,
and QoS compatibility.

The estimator header (`.hpp`) declares the C++ API and types shared with callers;
its implementation (`.cpp`) defines the numerical behavior. The node `.cpp` owns
the ROS adaptation. A constructor establishes validated configuration and empty
history before updates; `explicit` on a converting constructor prevents accidental
implicit construction from a configuration scalar. The
[engineering note](../engineering-notes/M2_STREAMING_VEHICLE_CORE.md) explains
these concepts, optionals, and ownership in detail. Exact constructor signatures,
unnecessary private method names, and enum identifiers may be chosen clearly
during implementation without changing the approved behavior.

Do not create the planned package, C++, CMake, `package.xml`, tests, or any other
runtime files in M2.2A. This design refines representation and responsibility only;
M2.1 equations, validity transitions, parameter default, input/output QoS, numerical
tolerance, rejection, and derivative-recovery semantics remain frozen.

## Approved causal estimator behavior

State is based on accepted source samples. Let `t_k` be the current source time,
`v_k` its longitudinal velocity, and `dt = t_k - t_(k-1)` in seconds, measured
against the last accepted sample.

Validate timestamp components before conversion: `sec` must be a signed 32-bit
integer and `nanosec` an integer in `[0, 999999999]`, as in the standard ROS Time
representation. Reject malformed components without silently normalizing them,
publishing, or updating state. `(sec=0, nanosec=0)` is permitted. Form an exact
integer nanosecond timestamp, compare ordering and subtract at nanosecond
resolution. Compare a strictly positive delta with the configured integer
nanosecond gap threshold; convert to seconds only for derivatives. Do not subtract
floating-point absolute timestamps: it can erase a small interval at a large epoch.

For continuous accepted samples, use the fixed backward differences:

```text
a_k = (v_k - v_(k-1)) / (t_k - t_(k-1))
j_k = (a_k - a_(k-1)) / (t_k - t_(k-1))
```

Acceleration is a causal backward finite difference. Jerk is a causal backward
finite difference between valid accelerations associated with their source sample
times. Only current and previously accepted information may contribute; future
samples, centered differences, and completed-window smoothing are excluded.

Conceptual states are frozen:

- `EMPTY`: no accepted velocity/time baseline.
- `HAVE_VELOCITY`: accepted velocity/time baseline; no valid acceleration history.
- `HAVE_ACCEL`: accepted velocity/time baseline plus valid acceleration history.

The state tracks available derivative history, not whether the last jerk was valid.
`V`, `A`, and `J` below denote true velocity, acceleration, and jerk validity flags;
every unlisted flag is false and its numeric field uses the v1 `0.0` placeholder.

| Incoming sample / state | Publication and next state |
| --- | --- |
| First valid sample in `EMPTY` | Publish V only; retain current velocity/time; transition to `HAVE_VELOCITY`. |
| Next continuous sample in `HAVE_VELOCITY`, finite acceleration | Publish V/A; retain current velocity/time and valid acceleration; transition to `HAVE_ACCEL`. |
| Further continuous sample in `HAVE_ACCEL`, finite acceleration and jerk | Publish V/A/J; retain current velocity/time and acceleration; remain `HAVE_ACCEL`. |
| `dt <= 0` | Reject, warn, publish nothing, and leave state and all history unchanged. Covers duplicate/decreasing source timestamps. |
| Invalid/missing child frame, malformed timestamp, or non-finite input velocity | Reject, publish nothing, and leave state and all history unchanged. |
| `dt > max_sample_gap_s` (default `0.25` s) | Accept current velocity, reset derivative history, publish V only, retain current velocity/time, and transition to `HAVE_VELOCITY`. |
| Non-finite calculated acceleration | Retain current valid velocity/time as the new baseline, publish V only, reset derivative history, and transition to `HAVE_VELOCITY`. |
| Valid acceleration but non-finite calculated jerk | Publish V/A with jerk invalid, retain current velocity/time and valid acceleration history, and remain `HAVE_ACCEL`. |
| Every otherwise accepted sample | Publish one `EgoState` with the preserved source stamp and `base_link` frame. |

Frame, timestamp-component, and input-velocity validation precedes state mutation;
ordering rejection precedes gap handling; gap handling precedes derivative
calculation. Non-finite derivative recovery applies only to otherwise accepted
inputs, so it does not override input rejection. No physical clipping/clamping is
used, including for finite extreme values.

Zero time must be distinguished from absence of estimator history: a first sample
at `t=0` is allowed; a later duplicate at `t=0` is rejected by `dt <= 0`. Rejected
samples do not become the reference for any later difference. A gap equal to the
approved limit does not satisfy the strict `>` discontinuity condition.
After a gap reset, the next continuous accepted sample has valid acceleration and
invalid jerk; the following continuous sample can have both derivatives valid.

The existing per-field validity flags and SI units remain unchanged. An invalid
acceleration or jerk uses `0.0` only as the `EgoState v1` transport placeholder;
consumers must check the matching flag. A physical zero with a true flag is valid.
Non-finite results must never be advertised as valid. M2.1 freezes the derivative
recovery policies above and deterministic unit-test absolute tolerance `1e-9`.
Check validity flags, source timestamps, frames, publication counts, and states
exactly; numerical tolerance does not apply to timestamp ordering or gap decisions.

## Deterministic reference sequence

Assume `child_frame_id = "base_link"`, finite velocities, and the default
`max_sample_gap_s = 0.25`, making every 0.10 s interval continuous. Every listed
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
must compare valid computed values using absolute tolerance `1e-9`, and check
timestamps, frames, validity flags, publication counts, and history behavior.

## M2.1 deterministic test matrix — specified, not implemented

Unless stated otherwise, each row starts with `EMPTY`, required `base_link` child
frame, default gap parameter `0.25`, and canonical source timestamps. `(t,v)` pairs
use seconds and m/s; derivative units remain m/s² and m/s³. For pure estimator
tests, compare each valid numeric result with absolute tolerance `1e-9` and check
optional presence, exact nanosecond timestamps, update outcomes, and history
transitions. Node-boundary tests check numeric placeholders, validity flags,
exact source stamps, output frame, and publication count. Frame and ROS component
validation cases belong to that boundary, not to the scalar estimator API.
All cases below are future acceptance requirements, not executed evidence.

| Case / test layer | Deterministic input or stimulus | Expected result |
| --- | --- | --- |
| Constant velocity / unit | `(0,10), (.1,10), (.2,10), (.3,10)` | V → V/A → V/A/J → V/A/J; valid A and J are zero; `HAVE_VELOCITY` then `HAVE_ACCEL`. |
| Constant acceleration / unit | `(0,1), (.1,1.2), (.2,1.4), (.3,1.6)` | Warm-up V → V/A → V/A/J; valid A = 2, valid J = 0. |
| Braking onset / unit | Complete M2.0 reference sequence above | A = 0, -2, -2, -2 after first sample; J = -20, 0, 0 after first valid A; correct flags and states. |
| Irregular positive intervals / unit | `(0,1), (.05,1.1), (.20,1.4), (.40,1.8)` | Use actual intervals .05, .15, .20; valid A = 2, valid J = 0 with normal warm-up. |
| Signed reverse motion / unit | `(0,-1), (.1,-1.2), (.2,-1.4)` | Preserve negative V; A = -2; final J = 0; no magnitude conversion or clipping. |
| First timestamp zero / unit + ROS boundary | First source components `(0,0)`, V = 10 | Accept and preserve zero stamp; publish V only, transition to `HAVE_VELOCITY`; normal recovery on later increasing stamps. |
| Duplicate timestamp / unit + ROS boundary | `(0,10), (.1,10), (.1,99), (.2,9.8)` | Third input rejected with warning, no publication/history change; last A = -2, J = -20 from accepted history. |
| Decreasing timestamp / unit + ROS boundary | `(0,10), (.1,10), (.05,99), (.2,9.8)` | Third input rejected with warning, no publication/history change; last A = -2, J = -20. |
| Exact .25 s gap boundary / unit | `(0,0), (.25,.5), (.5,1)` | No reset at 250000000 ns; V → V/A → V/A/J, A = 2 and final J = 0. |
| Greater-than-.25 gap / unit | `(0,10), (.1,10), (.350000001,20), (.450000001,20), (.550000001,20)` | 250000001 ns gap accepts V = 20, publishes V only, resets to `HAVE_VELOCITY`; then V/A with A = 0, then V/A/J with J = 0. |
| NaN/+Inf/-Inf input / unit + ROS boundary | In separate runs inject each after `(0,10), (.1,10)` at .15, then send `(.2,9.8)`; repeat injections in `EMPTY` | No publication or state change for each injection; final A = -2, J = -20; `EMPTY` remains empty on rejection. |
| Wrong/missing child frame / ROS boundary | Inject `child_frame_id = "odom"` or `""` at .15 after `(0,10), (.1,10)`, then valid `(.2,9.8)`; also test from `EMPTY` | Reject with no publication/state change; no TF2/relabeling; subsequent accepted history produces A = -2, J = -20. |
| Rejected-sample history preservation / unit + ROS boundary | Repeat ordering, velocity, frame, and malformed-stamp rejections in `EMPTY`, `HAVE_VELOCITY`, and `HAVE_ACCEL`; compare with a run omitting rejected samples | Same later values, flags, state, accepted source stamps and publication sequence; rejections add zero publications. |
| Malformed timestamp components / ROS boundary | `nanosec = 1000000000`; at an untyped validation boundary also noninteger/out-of-range components; then a valid increasing sample | Reject rather than carry/borrow/normalize components; preserve state and history. Wire-typed `sec`/`nanosec` cannot represent values outside their integer types. |
| Nanosecond ordering / unit + ROS boundary | Large epoch `sec = 2000000000`, successive `nanosec = 0,1,2`, V = 0 throughout; duplicate/decrease by 1 ns | Positive 1 ns intervals are accepted and produce zero derivatives after warm-up; duplicate/decrease rejected before float conversion. |
| Non-finite acceleration / unit | Let `B = 2^1023` (finite float64). `(0,-B), (.125,B), (.25,B), (.375,B)`; also `(0,0), (.125,B), (.25,B), (.375,B)` to exercise division overflow | Subtraction or division overflow gives V only with current valid V as baseline and `HAVE_VELOCITY`; next continuous constant samples recover V/A then V/A/J with zero derivatives. |
| Non-finite jerk / unit | Let `U = 2^1019`. `(0,0), (.125,-U), (.25,0), (.375,U)` | Finite A values -`2^1022`, +`2^1022`, +`2^1022`; jerk overflows at .25: publish V/A, retain +`2^1022` acceleration, remain `HAVE_ACCEL`; final J = 0 is valid. No clamping. |
| Callback-delay invariance / estimator harness + ROS integration | Replay the same source-stamped inputs in the same order with different callback delays, ensuring identical delivered samples | Identical numerical outputs, flags, source stamps, and state transitions; callback intervals never substitute for source intervals. |
| BEST_EFFORT publisher / ROS integration | Odometry publisher KEEP_LAST 5, BEST_EFFORT, VOLATILE, remaining defaults; approved subscriber profile | Discover compatible endpoints and verify received accepted source samples produce correct EgoState; assert subscriber settings. Do not require lossless delivery from Best Effort. |
| RELIABLE publisher / ROS integration | Same profile as preceding row except publisher RELIABLE; approved BEST_EFFORT subscriber | Requested/offered reliability remains compatible; verify actual reception and accepted-sample outputs, with other policies held compatible. |
| Parameter validation / parameter boundary | Default .25, other finite positive values; zero, negative, NaN, +Inf, -Inf | Default .25 and finite positive values accepted; invalid values rejected before use; frame and equations remain fixed. |

This matrix separates deterministic numerical unit tests from message validation
and DDS/executor integration checks. Delivery differences under loss are not
callback-delay invariance failures unless the same samples were delivered.

Filtering is evaluated only after the deterministic raw causal baseline has been
verified and its noise sensitivity measured. No filtering constants are approved.
Later comparisons must state information horizons and avoid future-data leakage.

## M2.0 documentation acceptance — historical scope

- [x] Record the approved source, frame, time, derivative, rejection, gap, warm-up,
  publication, and filtering decisions without changing `EgoState v1`.
- [x] Explain the streaming concepts and thesis-to-ROS velocity-source decision in
  the engineering note, including the deterministic sequence.
- [x] Identify input QoS and `max_sample_gap_s` as M2.1 open decisions.
- [x] Distinguish implemented M1, in-progress M2 design, and planned runtime work.
- [x] Create no ROS package, production code, new schema, dependency, dataset,
  checkpoint, Dockerization, or placeholder component.

Those M2.0 open decisions are resolved by the approved M2.1 specification above.

## M2.1 documentation acceptance

- [x] Freeze input SensorDataQoS semantics, default gap parameter, timestamp
  validation, three estimator states, and all acceptance/recovery transitions.
- [x] Freeze no filtering, no physical clipping/clamping, and unit absolute
  tolerance `1e-9`; specify future package/estimator/node boundaries.
- [x] Record the complete deterministic test matrix with expected outputs and
  separate unit and ROS integration responsibilities.
- [x] Preserve `EgoState v1`, output QoS, M1 evidence, and documentation-only scope.

## M2.2A documentation acceptance

- [x] Freeze planned package layout, names, ROS/domain boundary, and scalar API.
- [x] Specify integer ordering/gap comparison and once-per-configuration conversion.
- [x] Derive conceptual states from one optional history object and optional A/J.
- [x] Specify diagnostic outcomes, rejection immutability, node ownership, ROS
  mapping, and GoogleTest/ament testing without middleware startup.
- [x] Explain headers/source files, constructors, `explicit`, `std::optional`, and
  ownership without freezing unnecessary implementation details.
- [x] Preserve M2.1 numerical behavior and create no runtime files.

Documentation completion through M2.2A does not mean that M2 runtime acceptance
criteria have passed. Do not create `sts_ego_state_cpp`, change `EgoState v1`, alter M1
evidence, stage files, or commit as part of this task. Run `git diff --check` and
report changed files and any conflicts found.
