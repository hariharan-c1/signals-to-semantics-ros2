# M2 Streaming Vehicle Core — Engineering Reference

**Coverage:** Approved M2.0/M2.1 specification, M2.2A package/API design, and M2.2B local implementation and unit-test evidence.\
**Overall milestone:** v0.2 **In Progress**; ROS end-to-end verification pending.
M1 / v0.1 remains **Implemented / Accepted**.

This reference explains the implemented baseline and its verification limits.
The [M2 task](../tasks/M2_STREAMING_VEHICLE_CORE.md) remains the detailed acceptance
specification. Other references are the [approved architecture](../DESIGN_SESSION_0_V2.md),
[interface contract](../ROS_INTERFACE_SPEC.md), [QoS policy](../QOS.md),
[online/offline boundary](../ONLINE_VS_OFFLINE.md), and
[unchanged M1 evidence](M1_ROS2_FOUNDATION.md).

## 1. M2 purpose and system position

```text
/vehicle/odometry
       ↓
sts_ego_state_cpp
       ↓
/sts/ego/state
```

M2 converts streaming Odometry into deterministic causal longitudinal velocity,
acceleration, and jerk under the existing `EgoState v1` contract.
`EgoStateNode` and `EgoKinematicsEstimator` have been implemented locally in
M2.2B; selected-package builds and estimator unit tests have passed. ROS
end-to-end operation remains unverified.

Unlike the M1 canonical-payload publisher, this estimator is stateful: the last
accepted source timestamp and velocity, plus a previous valid acceleration when
available, determine the next result. Every otherwise accepted input produces an
output, including samples during derivative warm-up. It does not wait for a
completed scenario window.

Event detection and rolling temporal state belong to later M2 work. They are
pending and are not part of the current estimator implementation. No
`BrakeEvent` fields, event thresholds, or event/window implementation are defined
here.

## 2. Thesis continuity

The original thesis reconstructed kinematics from ego pose because direct vehicle
velocity telemetry was not assumed available. The ROS input already provides
velocity through `nav_msgs/Odometry` twist. When its `child_frame_id` is
`base_link`, that twist is expressed in the required vehicle body frame. The
causal online baseline therefore uses:

```text
twist.twist.linear.x
```

`nav_msgs/Odometry.pose` is intentionally not used to reconstruct velocity in
M2. Pose-derived reconstruction remains part of the broader project's later
offline-parity/replay/regression evaluation; it must not be implemented in M2.
That study can compare the thesis method with direct telemetry explicitly,
rather than silently mixing two velocity definitions in this baseline.

Replay can still process samples causally. Possessing a recording does not
authorize future-data use in the online path. Offline-parity evaluation may
use a complete window only under its declared information horizon, and must
not present those results as evidence of online performance.

## 3. Input/output contract

| Contract item | Input | Output |
| --- | --- | --- |
| Topic | `/vehicle/odometry` | `/sts/ego/state` |
| ROS message | `nav_msgs/Odometry` | `sts_interfaces/EgoState` (`EgoState v1`) |
| Required frame | Exact `child_frame_id == "base_link"` | `header.frame_id = "base_link"` |
| Velocity | `twist.twist.linear.x` | `longitudinal_velocity_mps` |
| Time | Measurement/source `header.stamp` | Accepted input `header.stamp` preserved exactly |

Odometry pose belongs to `header.frame_id`; twist belongs to
`child_frame_id`. The pose frame does not redefine the twist frame.
Wrong or missing child frames are rejected before calling the estimator.
M2 performs no TF2 lookup or conversion, and cannot repair a mismatched frame
by relabeling its X component.

Longitudinal velocity is a signed scalar along the ego X axis: positive is
forward, negative is reverse. It is neither unsigned speed nor a 3D magnitude.
Units are SI: velocity in m/s, acceleration in m/s², and jerk in m/s³.
Acceleration and jerk retain their signs; negative acceleration is acceleration
toward negative X, so its effect on speed depends on the velocity sign.

Each accepted or accepted-reset update maps to one `EgoState`. Velocity is valid.
A present acceleration/jerk optional maps to its numeric field and a true validity
flag. An absent derivative maps to numeric `0.0` and a false flag. A valid physical
zero has a true flag; consumers must never infer validity from the numeric value.
Rejected updates publish nothing. The v1 schema, field order, types, units,
timestamp/frame meanings, validity semantics, and M1 evidence remain unchanged.

## 4. QoS

| Policy | Odometry subscription | EgoState publication |
| --- | --- | --- |
| Profile | `rclcpp::SensorDataQoS()` | Explicit existing v1 profile |
| History | KEEP_LAST | KEEP_LAST |
| Depth | 5 | 10 |
| Reliability | BEST_EFFORT | RELIABLE |
| Durability | VOLATILE | VOLATILE |

Input deadline, lifespan, and liveliness remain defaults; no overrides are
selected. The implemented subscription uses `rclcpp::SensorDataQoS()`, consistent
with the [Jazzy declaration](https://github.com/ros2/rclcpp/blob/jazzy/rclcpp/include/rclcpp/qos.hpp)
and the approved [QoS policy](../QOS.md).

Odometry is timely streaming state: the shallow Best Effort subscription
prioritizes freshness without requesting reliable retransmission of every
sample. Loss can change the delivered sequence; freshness does not mean delivery
is guaranteed.

A Best Effort subscription can match both Best Effort and Reliable publishers
under ROS 2 requested/offered reliability rules, provided the other policies are
compatible. That compatibility does not guarantee lossless reception. Actual
reception from both publisher profiles remains a ROS integration requirement.

The Reliable output retains the approved normalized-state delivery contract for
downstream consumers, with a depth of ten. Input and output deliberately serve
different boundaries; publishing reliably cannot recover an input sample that
was never received. Both profiles are Volatile, so retained history is not a
durable replay service for late joiners.

## 5. Causal mathematics

For continuous accepted source samples, with time in seconds:

```text
dt  = t_k - t_(k-1)
a_k = (v_k - v_(k-1)) / dt
j_k = (a_k - a_(k-1)) / dt
```

A backward finite difference compares the current value with the previous
accepted value. Acceleration uses velocity change; jerk uses the change between
valid accelerations. Acceleration is associated with the current source time
`t_k`; on a continuous stream the interval between acceleration timestamps is
the current `dt`, including when sampling intervals are unequal.

Both calculations are causal: they use current and accepted past information.
They do not use future samples, centered differences, completed-window smoothing,
or revise an earlier output after a later sample arrives.

One velocity alone supplies no velocity change. Two continuous samples supply one
acceleration, but no acceleration change. A third can supply jerk. Thus ordinary
warm-up is V → V/A → V/A/J, subject to finite calculated values.

The approved braking-onset reference is:

```ini
t=0.00, v=10.00 -> a invalid, j invalid
t=0.10, v=10.00 -> a=0.0,  j invalid
t=0.20, v=9.80  -> a=-2.0, j=-20.0
t=0.30, v=9.60  -> a=-2.0, j=0.0
t=0.40, v=9.40  -> a=-2.0, j=0.0
```

Assume the required child frame, finite velocities, and the default gap limit.
Units are s, m/s, m/s², and m/s³. At 0.20 s,
`(9.80 - 10.00) / 0.10 = -2.0`; the preceding acceleration was zero, so
`(-2.0 - 0.0) / 0.10 = -20.0`. Constant deceleration thereafter gives zero jerk.
This sequence is covered by the estimator tests.

The irregular-interval fixture `(t,v) = (0,0), (.1,.1), (.3,.5)` gives
A = 1, then A = 2 and J = 5. The last jerk uses the current 0.20 s interval,
not the preceding 0.10 s interval.

The implementation uses no smoothing/filtering and no physical clipping/clamping.
Finite extreme values remain data rather than being forced into an assumed
vehicle envelope. The raw causal baseline exposes timestamp, sign, numerical,
and state-transition behavior directly. Differentiation amplifies measurement
noise, especially in jerk and at small sampling intervals. Filtering would introduce additional
history, delay, and parameter effects. First verify numerical correctness and
measure noise sensitivity; only then evaluate filtering against the baseline,
declaring causality or explicit fixed latency. No filtering constants or measured
noise/performance claims are approved here.

## 6. State machine

```text
EMPTY
  ↓ first accepted velocity
HAVE_VELOCITY
  ↓ next continuous sample with finite acceleration
HAVE_ACCEL
```

These are conceptual states derived from stored data, not a separate mutable
state enum:

| Stored representation | Conceptual state |
| --- | --- |
| No history | `EMPTY` |
| History + no acceleration | `HAVE_VELOCITY` |
| History + valid acceleration | `HAVE_ACCEL` |

The class contains one `std::optional<History>`. A present `History` contains
the accepted timestamp, velocity, and an optional valid acceleration.
Independent validity booleans or a separate mutable state enum could disagree
with that data; deriving state from presence avoids that inconsistency.

| Update | Output | Next state |
| --- | --- | --- |
| First valid sample | V only | `HAVE_VELOCITY` |
| Next continuous sample, finite A | V/A | `HAVE_ACCEL` |
| Further continuous sample, finite A/J | V/A/J | `HAVE_ACCEL` |
| Rejection | None | Unchanged |
| Gap or non-finite A | V only | `HAVE_VELOCITY` |
| Finite A, non-finite J | V/A | `HAVE_ACCEL` |

No previous jerk is stored: the next jerk requires current and previous valid
acceleration. `HAVE_ACCEL` describes available history, not whether the last jerk
was valid. After a reset, a continuous finite-A sample restores V/A; the following
continuous finite-A/J sample restores V/A/J.

## 7. Rejection, reset and partial degradation

| Category | Trigger | Output and history |
| --- | --- | --- |
| Rejection | `dt <= 0` or non-finite input velocity | No output; all history unchanged |
| Node-boundary rejection | Wrong/missing child frame or malformed timestamp components | No publication; estimator not called; history unchanged |
| Accepted reset | Long gap or non-finite calculated acceleration | Retain current valid velocity/time as new baseline; A/J unavailable; clear acceleration history |
| Accepted partial degradation | Valid acceleration but non-finite jerk | Retain current velocity/time and valid acceleration; J unavailable |

For duplicate/decreasing timestamps, the node warns and publishes nothing.
It does not sort inputs, overwrite history, or restart automatically. Later
samples are still compared with the last accepted sample. NaN, positive infinity,
and negative infinity in input velocity likewise cannot contaminate history.

Finite input velocity and a positive interval do not guarantee finite derivatives:
subtraction or division can overflow. Non-finite acceleration is an accepted
derivative reset, not a rejected input. Non-finite jerk is an accepted V/A result,
not a reset of valid acceleration. Both produce one output through the node,
with unavailable fields mapped to the v1 placeholders and false flags.
Non-finite derivatives are never marked valid or stored as valid acceleration
history.

The implemented `UpdateOutcome` distinguishes `Accepted`,
`AcceptedResetGap`, `AcceptedResetNonFiniteAcceleration`,
`AcceptedNonFiniteJerk`, `RejectedNonMonotonicTimestamp`, and
`RejectedNonFiniteVelocity`. `UpdateResult.sample` is absent for rejection
and present for every accepted outcome. The estimator does no logging;
the node uses these reasons for concise diagnostics.

The approved design freezes these behaviors and diagnostic distinctions, not
unnecessary private method names or exact enum identifiers.

## 8. Gap handling

```text
max_sample_gap_s default = 0.25
default integer threshold = 250000000 ns

dt_ns > threshold_ns  → discontinuity
dt_ns == threshold_ns → continuous
```

A large positive gap is not an ordering failure. Accept the current velocity,
clear derivative history, publish V only, and begin warm-up again from this
sample. This avoids presenting a difference over a long missing interval as a
continuous local derivative.

The ROS parameter must be finite and strictly positive. Zero, negative, NaN,
and infinite startup values are rejected. The node declares it read-only so
runtime parameter changes cannot disagree with the already constructed estimator.
Frame semantics and derivative equations are fixed contracts, not parameters.
The gap limit is a continuity decision, not an event threshold.

The estimator constructor converts seconds to the integer nanosecond threshold
once, rather than per sample. Ordering and the strict gap comparison are then
exact integer decisions. At the default, 250000000 ns remains continuous;
250000001 ns triggers reset.

The implementation truncates a positive fractional nanosecond threshold:
for integer deltas, `dt_ns > threshold` is equivalent to
`dt_ns > floor(threshold)`. A positive sub-nanosecond limit therefore makes
every positive integer delta discontinuous. A finite configured limit whose
converted magnitude reaches or exceeds `2^64` uses `UINT64_MAX`, preserving
no gap for every supported signed-timestamp span. This is threshold
representation handling, not physical clamping of velocity or derivatives.

## 9. Timestamp handling

`Odometry.header.stamp` is measurement/source time. Callback time is when the
executor handles the message; transport delay, queueing, and scheduling can
change callback intervals without changing measurement intervals. Derivatives
therefore use source stamps, never arrival spacing or the node's current clock.
Accepted input stamps are copied exactly into `EgoState.header.stamp`.

Timestamp zero is valid as the first simulation/replay sample. History absence
is represented separately, so zero is not an uninitialized sentinel. Repeating
an accepted zero is a duplicate and is rejected. The M1 subscriber's nonzero
check belongs to its synthetic fixture and does not redefine M2 or change M1
evidence.

The node validates timestamp representation before conversion. Standard ROS Time
uses signed 32-bit `sec` and a `nanosec` component in `[0, 999999999]`;
see the [ROS Time message](https://github.com/ros2/rcl_interfaces/blob/jazzy/builtin_interfaces/msg/Time.msg).
The wire types already constrain component types. The node rejects
`nanosec >= 1000000000` instead of carrying it into another second,
with no publication or estimator call. Canonical negative seconds are not
rejected merely for being negative.

The node forms `int64_t` nanoseconds as `sec * 1000000000 + nanosec`.
The estimator first compares signed timestamps. Duplicate/decreasing timestamps
are rejected before subtraction. For increasing timestamps it computes:

```cpp
static_cast<uint64_t>(timestamp_ns) -
static_cast<uint64_t>(history_->timestamp_ns)
```

The full positive span between two ordered `int64_t` timestamps can exceed
`INT64_MAX`. Unsigned subtraction gives the exact positive delta without signed
overflow, including across zero and from `INT64_MIN` to `INT64_MAX`.

Only after a positive delta has passed the integer gap check does the estimator
convert it to floating-point seconds for derivative calculation. Subtracting
large floating-point absolute timestamps could erase a 1 ns interval or change
ordering; integer ordering avoids that loss. Numeric test tolerance does not
apply to timestamps or gap decisions.

With identical source samples and arrival order, different callback delays should
yield identical numerical results. ROS callback-delay invariance remains a
pending integration check, not a measured result. Best Effort loss can change
the delivered sequence and must be assessed separately.

## 10. Package architecture

```text
sts_ws/src/sts_ego_state_cpp/
├── CMakeLists.txt
├── package.xml
├── include/sts_ego_state_cpp/ego_kinematics_estimator.hpp
├── src/ego_kinematics_estimator.cpp
├── src/ego_state_node.cpp
└── test/test_ego_kinematics_estimator.cpp
```

```text
EgoStateNode                  = ROS adapter
       ↓ scalar timestamp + velocity
EgoKinematicsEstimator        = deterministic domain logic
       ↓ outcome + optional sample
EgoStateNode                  = diagnostics + EgoState mapping/publication
```

| Name | Implemented value |
| --- | --- |
| Package | `sts_ego_state_cpp` |
| Estimator class | `EgoKinematicsEstimator` |
| ROS adapter class | `EgoStateNode` |
| Executable | `ego_state_node` |
| ROS node name | `ego_state` |

The node owns subscriptions/publications, frame validation, timestamp component
validation, startup parameter validation, source-stamp preservation, frame
assignment, message mapping, and logging. It forwards only
`int64_t timestamp_ns` and `double velocity_mps` to the estimator.

The estimator owns ordering, continuity, finite-difference calculations,
numerical validation, and history. It depends on no ROS messages, DDS,
executors, logging, frame names, or ROS parameters. Frame interpretation is
already validated at the node boundary.

`EgoStateNode` owns its estimator as a member value, with no global/static
estimator storage. Independent node instances and test fixtures therefore
start with independent history.

Middleware handles transport, discovery, callbacks, and publication; domain
logic handles the kinematic rules. Separating them lets tests call the exact
production estimator directly without ROS graph startup, and permits reuse
with other scalar-input adapters. GoogleTest/ament supplies test/build
integration without making middleware a dependency of the estimator.
ROS integration must separately verify the adapter and communication.

M2.2A froze this layout and responsibility boundary as documentation only.
The actual package files were created under the separately authorized M2.2B work.

## 11. C++ concepts used

| Concept | Meaning in the M2 implementation |
| --- | --- |
| `.hpp` versus `.cpp` | The header declares API/types shared by the node and tests. The estimator `.cpp` defines calculations compiled into a static library; the node `.cpp` defines adaptation and `main`; the test `.cpp` defines GoogleTest cases. |
| Namespace | `sts_ego_state_cpp` groups project names such as `EgoKinematicsEstimator`, avoiding collisions with other libraries. |
| Anonymous namespace | The unnamed namespace in the estimator `.cpp` keeps `gap_threshold_ns` local to that translation unit. Test helpers are similarly local to the test file. |
| `enum class` | `UpdateOutcome` names result reasons with scoped values such as `UpdateOutcome::AcceptedResetGap`. It describes outcomes, not a separately mutable estimator state. |
| `struct` | `KinematicSample`, `UpdateResult`, and `History` group related data. Struct members default to public, while the nested `History` type itself is private to the estimator. |
| `std::optional` | Holds an owned value or no value. Absent history means `EMPTY`; absent A/J means unavailable, while present `0.0` is valid zero. |
| `std::nullopt` | Explicitly constructs an empty optional, used for rejected outputs and unavailable derivatives. It is not a numeric sentinel. |
| Constructor | `EgoKinematicsEstimator(double max_sample_gap_s = 0.25)` validates configuration, establishes the gap threshold, and starts with absent history. It consumes no sample or ROS peer. |
| Constructor initializer list | `: max_sample_gap_ns_(gap_threshold_ns(max_sample_gap_s))` initializes the const threshold before the constructor body. The node similarly initializes its ROS base and owned estimator before creating endpoints. |
| `explicit` | Prevents a gap scalar from silently converting into an estimator object. Direct construction such as `EgoKinematicsEstimator estimator{0.25};` remains allowed; validation still happens in constructor logic. |
| `const` | Makes `max_sample_gap_ns_` fixed after construction. Const local deltas and derivative values cannot be reassigned within an update. |
| `int64_t` / `uint64_t` | Signed 64-bit timestamps support negative and positive source times; unsigned 64-bit deltas can represent the entire positive span between ordered signed timestamps. |
| `std::isfinite` | Rejects NaN/infinity in configuration or velocity and detects non-finite calculated A/J before they can be marked valid. |
| `std::numeric_limits` | Supplies type limits such as the maximum unsigned threshold and reproducible NaN/infinity fixtures in tests. |
| Dereferencing an optional with `*` | After confirming presence, `*result.sample` accesses its sample and `*history_->acceleration_mps2` accesses valid previous acceleration. Dereferencing an absent optional is not allowed. |
| Member access through `->` | `history_->timestamp_ns` accesses the contained history after a presence check. `publisher_->publish(...)` accesses the ROS publisher through its shared pointer. |
| Aggregate initialization | `History{timestamp_ns, velocity_mps, std::nullopt}` fills fields in declaration order and replaces history with a velocity-only baseline. `KinematicSample{...}` similarly initializes output data. |
| `auto` | Deduces a type from its initializer: `auto outcome = UpdateOutcome::Accepted` remains an `UpdateOutcome`; `const auto & sample = *result.sample` names a sample by reference without copying. |
| Private versus public members | The public constructor and `update` form the small estimator API. Private history and threshold cannot be changed directly by callers. |
| Ownership | The node's `EgoKinematicsEstimator estimator_;` member lives with that node. Optional history lives inside the estimator; no raw owning pointer or shared global history is needed. |

For language details, see the existing
[C++ explicit specifier rules](https://eel.is/c++draft/dcl.fct.spec) and
[C++ optional specification](https://eel.is/c++draft/optional.optional).
Constructor purpose, optional representation, and ownership are approved design
principles; unnecessary helper names are implementation details.

## 12. Important estimator implementation flow

```text
receive timestamp + velocity

if velocity non-finite:
    reject with no output; history unchanged

if history exists and timestamp <= previous:
    reject with no output; history unchanged

if first sample:
    store timestamp + V, no acceleration
    output V only; return accepted

compute positive dt_ns using unsigned subtraction

if dt_ns exceeds gap threshold:
    replace history with current timestamp + V, no acceleration
    output V only; return accepted-reset

convert dt_ns to seconds
compute acceleration

if acceleration non-finite:
    replace history with current timestamp + V, no acceleration
    output V only; return accepted-reset

if previous acceleration exists:
    compute jerk
    if jerk finite:
        include jerk
    otherwise:
        leave jerk unavailable; report accepted partial degradation

store current timestamp, velocity, valid acceleration
return accepted result with V/A and optional J
```

Ordering rejection precedes gap handling; gap handling precedes derivatives.
All rejection paths return before history mutation. Accepted reset paths replace
the complete history object, clearing old acceleration. Partial degradation
stores the valid current acceleration, allowing the next sample to recover jerk.
The node rejects frame/component failures before this flow starts.

## 13. Test philosophy and coverage

GoogleTest exercises the ROS-independent production estimator directly using
GoogleTest/ament, with no DDS, executor, ROS messages, or graph startup.
Assertions check numerical outputs and history effects, not merely startup or
return status. Deterministic computed values use absolute tolerance `1e-9`;
timestamps, optional presence, and outcomes use exact checks. Node-boundary
acceptance additionally requires exact flags, placeholders, frames, source
stamps, and publication counts; those ROS checks remain pending.
M1's exactly representable canonical fixture does not justify exact floating-point
equality for ordinary calculated derivatives.

Verified local evidence after hardening, **2026-10-02, macOS arm64**:

| Evidence | Result |
| --- | --- |
| Estimator GoogleTest cases | **32 passed**, zero errors/failures/skips |
| Five ament lint tools | Copyright, cpplint, CMake lint, uncrustify, and XML lint executed successfully |
| cppcheck | Four file checks skipped by installed cppcheck 2.22.0 tooling because of known performance issues |
| Package-only result report | **57 records**, zero errors/failures, four cppcheck-related skips |
| Selected-package build | Two packages finished, exit status zero |
| Package test run | One package finished, exit status zero; seven CTest entries reported passed |

The 32 estimator cases comprise 23 ordinary cases and nine parameterized cases
(three rejection/history scenarios repeated for NaN/+Inf/-Inf). The seven CTest
entries are one estimator executable and six lint runners. A passed cppcheck
runner does not mean its four skipped file checks executed.

The 57 package-only records comprise seven CTest wrapper records, 32 GoogleTest
cases, and 18 lint file/check records. Wrappers are not extra numerical tests.
Before hardening, 30 estimator cases passed and five lint tools executed
successfully, with the same four cppcheck skips.

The earlier workspace aggregate of 101 result records was **not 101 M2 tests**:
55 records were M2 (seven wrappers + 30 estimator cases + 18 lint records);
46 were historical M1 records from the default build tree and separate
`build/m1_4` tree. Four skips were M2 cppcheck and two were historical M1
cppcheck. The scoped report excludes those M1 files. The CTest parser uses
the latest `Testing/TAG` selection; historical workspace result trees still
contribute to an unscoped aggregate.

| Coverage | What the tests demonstrate |
| --- | --- |
| Startup and first timestamp zero | V only, then V/A, then V/A/J without treating zero as absent history |
| Constant velocity / acceleration | Zero derivatives for constant V; A = 2 and J = 0 for constant positive A |
| Braking onset | Complete approved reference including J = -20 at onset |
| Irregular dt | Actual source intervals used; both constant-A and nonzero-J fixtures |
| Signed reverse motion | Negative velocity and acceleration retained |
| Exact gap boundary | 0.25 s remains continuous |
| Gap reset/recovery | One nanosecond beyond the limit resets old A and proves V → V/A → V/A/J recovery |
| Duplicate/decreasing timestamps | Rejection followed by correct later A/J from accepted history |
| NaN/+Inf/-Inf inputs | Rejection in EMPTY, velocity-only, and acceleration-history states |
| Rejected-sample history preservation | Subsequent outputs prove that rejected timestamps/velocities did not become baselines |
| Acceleration overflow/reset | Both subtraction and division overflow; recovery from current velocity/time |
| Overflow while HAVE_ACCEL | Previously valid nonzero A is cleared, then ordinary samples recover A and J |
| Jerk overflow/partial degradation | Valid current A retained; subsequent J recovers to zero |
| Nanosecond ordering | One-nanosecond steps remain distinct at a large epoch |
| Extreme timestamp span | Full signed range handled without signed subtraction overflow |
| Configuration and ownership | Invalid gap values rejected; custom, sub-nanosecond and very large limits handled; instances have independent histories |

Overflow fixtures use extreme finite float64 powers of two and 0.125 s intervals
for reproducible finite references and overflow. The HAVE_ACCEL fixture first
establishes A = `2^1006`, then overflows A with finite velocity zero. Later
ordinary samples prove that the overflow sample's timestamp and zero velocity
became the baseline, that old A was discarded, and that warm-up resumes.
The irregular fixture verifies A = 1, A = 2, J = 5; the division-overflow
fixture checks final valid zero jerk. No existing test was weakened by hardening.

The complete [M2.1 deterministic matrix](../tasks/M2_STREAMING_VEHICLE_CORE.md#m21-deterministic-test-matrix)
also requires wrong/missing child-frame rejection, malformed ROS timestamp
validation, parameter/message mapping, source-stamp preservation, publication
counts, callback-delay invariance, and reception from both Best Effort and
Reliable publishers. **ROS boundary/runtime/integration tests remain pending.**
Rejection verification should cover applicable states and compare later outputs
with a control stream omitting rejected samples. Integration must distinguish
transport loss from delay while holding delivered samples and order identical.

Recorded verification commands, from `sts_ws` in the existing Pixi environment:

```bash
pixi run --locked colcon build --packages-select sts_interfaces sts_ego_state_cpp
pixi run --locked colcon test --packages-select sts_ego_state_cpp
pixi run --locked colcon test-result --test-result-base build/sts_ego_state_cpp --verbose --all
```

These are previously executed implementation/hardening results, not new runtime
evidence from this documentation consolidation. M2 Linux runtime verification,
noise sensitivity, and performance measurement remain pending.

## 14. macOS/RoboStack engineering notes

The node's CMake link options include `LINKER:-dead_strip_dylibs` under
`if(APPLE)`. As in M1, this lets the Apple linker omit unused RoboStack Python
generator dylibs from the C++ executable's dependencies. It does not change the
estimator or ROS message behavior.

The discovered GoogleTest workaround is guarded by
`if(APPLE AND GTEST_FROM_SOURCE_FOUND)`:

```cmake
target_compile_options(gtest PUBLIC "-iquote${GTEST_FROM_SOURCE_INCLUDE_DIRS}")
```

RoboStack Clang supplies the environment prefix's include directory as a regular
include path, which can take precedence over ament's SYSTEM vendored GoogleTest
headers. The result was older ament-selected GoogleTest sources encountering
headers from the separately installed GoogleTest version.

The `-iquote` option prioritizes the discovered matching source-tree headers
for quoted includes such as `"gtest/gtest.h"`. Making it PUBLIC propagates
the option to consumers such as `gtest_main` and the estimator test target.
The path comes from CMake discovery; no machine-specific absolute path is
hard-coded. Both Apple guards leave Linux build logic unaffected; that is a
CMake-scope observation, not a new Linux M2 test result.

Known incremental interface installation on RoboStack/macOS emitted
`install_name_tool` diagnostics about removing absent RPATH entries and
regenerating signatures, while colcon still returned build success.
Record these as environment observations, not functional estimator failures.
Neither a successful build nor pure estimator tests establish ROS runtime
acceptance.

## 15. File-by-file future lookup guide

Paths below are relative to `sts_ws/src/sts_ego_state_cpp/`.

| File | Look here when you want to understand... |
| --- | --- |
| [ego_kinematics_estimator.hpp](../../sts_ws/src/sts_ego_state_cpp/include/sts_ego_state_cpp/ego_kinematics_estimator.hpp) | API, result types, optional history, and class state |
| [ego_kinematics_estimator.cpp](../../sts_ws/src/sts_ego_state_cpp/src/ego_kinematics_estimator.cpp) | Timestamp logic, gap handling, acceleration/jerk, and state mutation |
| [ego_state_node.cpp](../../sts_ws/src/sts_ego_state_cpp/src/ego_state_node.cpp) | ROS subscription/publication, frame validation, parameter handling, and message mapping |
| [test_ego_kinematics_estimator.cpp](../../sts_ws/src/sts_ego_state_cpp/test/test_ego_kinematics_estimator.cpp) | Deterministic mathematics and state-machine verification |
| [CMakeLists.txt](../../sts_ws/src/sts_ego_state_cpp/CMakeLists.txt) | Compilation, dependencies, testing, and Apple workarounds |
| [package.xml](../../sts_ws/src/sts_ego_state_cpp/package.xml) | ROS package dependency metadata |

## 16. Interview recall

| Question | Concise answer |
| --- | --- |
| Why separate ROS from the estimator? | Scalar domain logic can be tested and reused without transport, executor timing, or a running ROS graph. |
| Why source timestamp instead of callback time? | It records measurement time; queueing and callback delays must not change the physical interval used for derivatives. |
| Why integer nanoseconds? | Exact ordering and gap boundaries survive large absolute timestamps and 1 ns intervals; unsigned deltas avoid signed overflow. |
| Why Best Effort input but Reliable output? | Input prioritizes fresh streaming measurements; output preserves the approved downstream delivery contract. Reliable output cannot restore lost inputs. |
| Why no filtering initially? | Verify the deterministic raw baseline and measure noise sensitivity before adding filter history, delay, and parameters. |
| Why not use pose like the thesis? | ROS already supplies frame-qualified velocity through twist; pose-derived parity is deferred to an explicit later comparison. |
| Difference between rejection and reset? | Rejection has no output or state change; accepted reset retains current velocity/time, clears derivatives, and restarts warm-up. |
| Why `std::optional`? | It distinguishes unavailable data from valid zero and makes conceptual state follow the stored history rather than separate flags. |
| How was the estimator tested? | Direct GoogleTest calls verify numeric outputs, optional validity, rejection history, reset recovery, and overflow; 32 cases passed locally, with ROS integration pending. |

## 17. Current status

| M2 item | Current status |
| --- | --- |
| M2.0 architecture baseline | Complete documentation baseline |
| M2.1 numerical/runtime contract | Complete specification; runtime verification is separate |
| M2.2A package/API design | Complete |
| M2.2B estimator + node | Implemented locally and estimator unit-tested; node runtime verification pending |
| M2.2D ROS runtime integration | Pending |
| Event detection | Pending / Planned |
| Rolling temporal state | Pending / Planned |
| M2 / v0.2 Streaming Vehicle Core | **In Progress** |

Documentation-only work through M2.2A created no runtime files. M2.2B was
separately authorized and now supplies the package and unit evidence.
This reference consolidation changes documentation only: approved production
behavior, `EgoState v1`, and M1 evidence are unchanged. Later release milestones
remain Planned; M2 completion is not claimed.
