# M1 ROS 2 Foundation — Engineering Reference

**Coverage:** Completed work through M1.3B; functional verification accepted.
**Overall milestone:** v0.1 remains In Progress; CI and automated cross-process
integration testing are not established by this work.

References: [approved architecture](../DESIGN_SESSION_0_V2.md),
[M1 task](../tasks/M1_ROS_FOUNDATION.md),
[interface contract](../ROS_INTERFACE_SPEC.md), and [QoS policy](../QOS.md).
Those documents retain some earlier implementation-status labels; this note records
the accepted M1.3B evidence without changing their contracts or architecture.

## 1. Milestone purpose

Prove that one approved custom ROS message can be generated, built, published in
C++, and received and validated in Python on the local Apple M1 Pro. The completed
path is `sts_contract_publisher_cpp` → `/sts/ego/state` →
`sts_contract_subscriber_py`, using `sts_interfaces/EgoState`.

All three packages built. Package checks reported **17 tests, 0 errors, 0 failures,
1 skipped**; cppcheck was skipped by the installed tooling because that version has
known performance issues. Python tests cover canonical acceptance and rejection of
incorrect fields. The separate manual runtime check produced repeated subscriber
`PASS canonical EgoState payload` logs and showed one publisher and one subscription.
These are contract-verification support packages. Production `sts_ego_state_cpp`,
odometry processing, kinematic derivation, TF2, and scenario logic remain outside
this completed scope.

## 2. M1.0 ROS 2 fundamentals

| Concept | Meaning in this foundation |
| --- | --- |
| ROS 2 | Libraries, middleware integration, and tools for communicating software components. |
| Node | A named participant in the ROS graph; a process can host one or several nodes. |
| Topic | A named channel for typed publish/subscribe communication, here `/sts/ego/state`. |
| Message | The typed data schema and its generated language representations. |
| Publisher | A node-owned endpoint that sends messages on a topic. |
| Subscriber | A node-owned subscription endpoint that receives compatible messages. |
| Callback | A function an executor invokes when work is ready, such as a timer or received message. |
| DDS/RMW | DDS supplies discovery/data transport in this environment; RMW is ROS's middleware abstraction. The build selected `rmw_fastrtps_cpp`. |
| `rclcpp` / `rclpy` | C++ / Python ROS client libraries, providing nodes, endpoints, clocks, and executors. |
| QoS | Communication policies governing reliability, retained history, durability, and compatibility. |
| Package | A discoverable unit of source, dependencies, build metadata, and installed resources. |
| Workspace | The `sts_ws` collection of packages in `src/`, with generated `build/`, `install/`, and `log/`. |
| `colcon` | Discovers packages and orchestrates their build/test order from dependencies. |
| `package.xml` | Package identity, maintainership, license, dependency categories, and build type. |
| `CMakeLists.txt` | CMake instructions for finding dependencies, generating interfaces or compiling targets, testing, and installation. The Python package instead uses setuptools. |

AUTOSAR comparisons are conceptual aids, not interchangeable technical contracts:

| AUTOSAR concept | Approximate ROS concept |
| --- | --- |
| SWC | Node |
| P-Port | Publisher |
| R-Port | Subscriber |
| Sender-Receiver | Topic communication |
| Interface/DataElement | ROS message schema/fields |
| Runnable | Callback |
| TimingEvent | Timer callback |
| RTE | Conceptually the ROS client/middleware abstraction; **not technically equivalent** to AUTOSAR RTE. |

ROS discovery, QoS, executors, and deployment do not reproduce AUTOSAR's generated
RTE APIs or scheduling guarantees.

## 3. M1.1 Apple Silicon ROS environment

Pixi manages the native **RoboStack ROS 2 Jazzy** environment. `pixi.toml` selects
the `robostack-jazzy` channel, `ros-jazzy-desktop`, `ros-dev-tools`, and platform
`osx-arm64`; the machine/compiler target is arm64. This is the local macOS stage.

Talker/listener verification demonstrated actual `Hello World` publication and
reception. RViz verification reached OpenGL initialization (OpenGL 2.1/GLSL 1.2);
the log's “Stereo is NOT SUPPORTED” message does not establish a startup failure.
This verifies RViz startup, not project-specific visualization or TF behavior.

| Environment artifact | Role |
| --- | --- |
| `pixi.toml` | Human-maintained dependency constraints, channels, and supported platform. |
| `pixi.lock` | Resolved dependency versions/builds for reproducing the environment; tracked. |
| `.pixi/` | Local installed environment and generated state; ignored by Git. |

`feat/ros-foundation` holds M1 environment, contract, interface, and node commits.
At this note's creation, `main`/`origin/main` still point to the architecture
foundation, while the feature branch and its remote-tracking reference contain
the M1.3B commit. This keeps milestone work reviewable before integration into main.

## 4. M1.2 EgoState v1 contract

The custom interface expresses normalized longitudinal ego quantities and explicit
per-field validity needed by the scenario contract. Standard pose/twist messages
remain appropriate for generic geometry; this schema avoids adding unused pose,
yaw, lateral, or 3D fields.

Velocity is a **signed longitudinal scalar**, not unsigned speed or a 3D vector:
the ego X axis is positive forward, and the sign preserves direction. Velocity,
acceleration, and jerk use SI units m/s, m/s², and m/s³ respectively. Negative
longitudinal acceleration represents deceleration along that axis.

`velocity_valid`, `acceleration_valid`, and `jerk_valid` each govern their matching
numeric field. When a flag is false, consumers must not interpret that quantity;
the producer uses `0.0` only as an invalid/unavailable transport placeholder.
The M1 fixture requires all three flags true.

`std_msgs/Header` supplies `stamp` and `frame_id`. The timestamp means sample/source
time; this synthetic publisher assigns its node ROS clock time during message
creation. `frame_id = "base_link"` identifies the ego body frame without publishing
any transform. The timestamp is variable, not a frozen fixture constant.

Both endpoints use **Reliable, Keep Last depth 10, Volatile**: request reliable
delivery, keep up to the latest ten samples in endpoint history, and do not retain
historical samples for late joiners. Live Fast DDS introspection reported
Reliable/Volatile but `History (Depth): UNKNOWN`; depth 10 is explicit in both
sources, not independently established by that CLI output.

Canonical payload on `/sts/ego/state`:

```ini
frame_id = "base_link"
longitudinal_velocity_mps = 13.5
longitudinal_acceleration_mps2 = -2.25
longitudinal_jerk_mps3 = -4.0
velocity_valid = true
acceleration_valid = true
jerk_valid = true
```

## 5. M1.3A Interface package

Source: [`sts_interfaces`](../../sts_ws/src/sts_interfaces/).

| Item | Role |
| --- | --- |
| `msg/EgoState.msg` | Single source schema: Header, three `float64` quantities, three `bool` validity flags. |
| `package.xml` | Declares `ament_cmake`, generator tooling, `std_msgs`, runtime support, lint dependencies, and membership in `rosidl_interface_packages`. |
| `CMakeLists.txt` | Finds dependencies, registers the message generation, exports runtime dependencies, registers lint checks, and finalizes the package. |
| `ament_cmake` | ROS integration for CMake packages, including package registration/export and testing helpers. |
| ROSIDL | ROS interface description/adaptation and language/type-support generation infrastructure. |
| `rosidl_default_generators` | Build-time default interface generators. |
| `rosidl_default_runtime` | Runtime dependency set used by generated interfaces and their consumers. |
| `rosidl_generate_interfaces` | Registers `msg/EgoState.msg` and its `std_msgs` dependency with the generators. |
| `ament_package` | Finalizes package metadata and CMake discovery/export files. |
| `colcon build` | Builds packages in dependency order and installs their generated outputs. |

One `.msg` yields the C++ header/type `sts_interfaces::msg::EgoState` and Python
import `from sts_interfaces.msg import EgoState`, plus serialization/type-support
code. The processes exchange serialized data through middleware; Python does not
call the publisher's C++ class directly.

The Pixi environment is the underlay. Sourcing `sts_ws/install/setup.zsh` adds the
built workspace overlay to package/library/Python discovery in that shell. Source
files in `src/` alone do not make a package runnable; build, install, and overlay
activation establish that path. Generated outputs remain ignored by Git.

## 6. M1.3B C++ publisher

Source: [`ego_state_test_publisher.cpp`](../../sts_ws/src/sts_contract_publisher_cpp/src/ego_state_test_publisher.cpp).

- Includes provide duration literals (`<chrono>`), shared ownership (`<memory>`),
  the ROS API, and the generated message. `std::chrono_literals` makes `1s` a typed
  one-second duration.
- `EgoStateTestPublisher : public rclcpp::Node` inherits ROS node functionality.
  Its constructor initializer calls `Node("ego_state_test_publisher")` before
  creating the endpoints and timer.
- QoS starts with `KeepLast(10)` and explicitly sets reliable and volatile.
  `create_publisher<EgoState>` binds that type and QoS to `/sts/ego/state`.
- `create_wall_timer(1s, ...)` schedules periodic work. The lambda `[this]` captures
  the current object's pointer so it can call `publish_canonical_payload()`.
  The wall timer's cadence is distinct from the ROS clock used for the stamp;
  it does not promise hard real-time scheduling.
- Each callback constructs a new message, assigns `get_clock()->now()`, the frame,
  all canonical quantities, and all validity flags. `publish()` submits it to ROS;
  `RCLCPP_INFO` logs the send. A send log alone cannot prove reception.
- `SharedPtr` members retain the publisher and timer for the node's lifetime.
  `std::make_shared` creates the node with shared ownership.
- `rclcpp::init` initializes the context and ROS arguments. `spin` runs an executor
  that dispatches ready work; it is what lets the timer callback execute.
  `shutdown` closes the ROS context after spinning ends.

The `ament_cmake` package links `rclcpp::rclcpp` and the generated C++ typesupport
target, then installs `ego_state_test_publisher` under `lib/<package>` for `ros2 run`.

## 7. M1.3B Python subscriber

Source: [`ego_state_test_subscriber.py`](../../sts_ws/src/sts_contract_subscriber_py/sts_contract_subscriber_py/ego_state_test_subscriber.py).

- Imports supply typing, the `rclpy` context/API, `Node`, explicit QoS policies,
  and the generated `EgoState` class.
- `EgoStateTestSubscriber(Node)` calls `super().__init__` with
  `ego_state_test_subscriber`. `QoSProfile` specifies Keep Last 10, Reliable,
  Volatile. `create_subscription` binds the message, topic, callback, and QoS;
  `self.subscription` retains the endpoint.
- `CANONICAL_FIELDS` records expected values. `validate_message` constructs the
  actual field mapping and returns all field-level mismatches. It also rejects a
  zero timestamp. This check does not prove freshness or the source clock's origin;
  those semantics are supplied by the publisher implementation.
- `_on_message` is dispatched for each received message. The logger emits explicit
  `PASS` when no mismatch exists, otherwise an error-level `FAIL` listing each
  expected/received value. A failed payload receives no PASS; the node keeps running
  so subsequent messages can also be checked.
- `rclpy.init` initializes ROS; `spin` dispatches callbacks. `finally` calls
  `destroy_node`; the `rclpy.ok()` guard avoids calling `shutdown` again after the
  context has already been shut down by signal handling.

Exact float equality is appropriate here: **13.5, -2.25, and -4.0 are exactly
representable binary floating-point values**, directly assigned and serialized as
`float64`, with no numerical calculation. Later calculated sensor quantities need
explicit, justified absolute/relative tolerances rather than copying this rule.

The `ament_python` package uses `setup.py` for installation and the console entry
point, an ament resource marker for discovery, and `setup.cfg` to install the script
under `lib/<package>`. Its five Python checks use `unittest` discovery in the current
environment; they test validation locally, not the live C++ process.

## 8. macOS linker issue

The first executable built but aborted at startup with
`dyld: symbol not found in flat namespace '_PyBool_Type'`. RoboStack's transitive
exported CMake dependencies pulled Python generator dylibs into the native C++
executable. Those unused libraries reference Python runtime symbols; the C++ process
does not provide the Python interpreter's `_PyBool_Type` symbol.

Linking the specific C++ typesupport target reduced the unnecessary dependencies,
but transitive Python dylibs still appeared through the imported ROS targets.
`target_link_options(... PRIVATE "LINKER:-dead_strip_dylibs")` then instructed the
Apple linker to omit unused dylib dependencies from this executable. Inspection
showed the unwanted Python generator dependencies removed, and startup succeeded.
This fixes this executable's observed startup problem; it does not change ROSIDL
generation or the message contract.

The option is inside `if(APPLE)`, so Linux does not receive the Apple linker flag.
Linux builds are unaffected by this option; a Linux build was not verified in M1.3B.

## 9. Git concepts learned

| Concept/command | Meaning |
| --- | --- |
| `git init` | Creates repository metadata; the initial architecture commit starts this history. |
| Branch | A movable reference to a commit; work advances the checked-out branch. |
| `git switch -c <name>` | Creates and switches to a new branch. The recorded feature branch is `feat/ros-foundation`. |
| `git add` | Copies selected working-tree changes into the index (staging area). |
| `git commit` | Records the staged snapshot locally; does not publish it remotely. |
| `git push` | Publishes local commits/references to a remote. |
| `origin` | This repository's remote name for its GitHub URL. |
| `main` vs `origin/main` | Local branch vs locally stored last-known remote-tracking reference; neither automatically follows feature-branch commits. |
| `git diff` | Tracked working-tree changes relative to the index; untracked file contents are omitted. |
| `git diff --cached` | Staged changes relative to HEAD. |
| `git status` | Branch/staging/working-tree state, including untracked paths. |
| `git log` | Commit history; `--oneline --decorate` makes commits and references easy to scan. |

Environment, interface design, generation, and node work appear as separate commits
on the feature branch. A clean status means there are no pending reported changes;
it does not prove runtime correctness.

## 10. Interview recall

> I built a minimal ROS 2 Jazzy foundation on Apple Silicon using Pixi and
> RoboStack. I first specified an EgoState contract with signed longitudinal
> velocity, acceleration, jerk, a source timestamp, the base_link frame, and
> explicit validity flags. ROSIDL generates C++ and Python types from one message
> definition. A C++ node publishes the fixed canonical payload once per second;
> a Python node validates every field and reports PASS or detailed mismatches.
> Both endpoints use Reliable, Keep Last depth 10, Volatile QoS. All three packages
> build, and package tests report zero failures with one tooling skip. I also
> verified real C++ to Python reception and inspected the ROS graph. I diagnosed
> a macOS transitive Python dylib startup issue and fixed it with a guarded linker
> option. This establishes interface compatibility; vehicle processing and the
> later scenario-intelligence pipeline remain planned.

## 11. Command cheat sheet

These commands were used and verified in the foundation/review workflow. Run the
build/test commands from `sts_ws/`; Pixi discovers the parent project manifest.

```sh
pixi run colcon build --packages-select sts_interfaces sts_contract_publisher_cpp sts_contract_subscriber_py --event-handlers console_stderr-
pixi run colcon test --packages-select sts_interfaces sts_contract_publisher_cpp sts_contract_subscriber_py --event-handlers console_stderr- --return-code-on-test-failure
pixi run colcon test-result --verbose
```

For the runtime check, both terminals used ROS domain 73 and a writable temporary
log directory. These invocations were verified from `sts_ws/`:

```sh
mkdir -p /private/tmp/sts_m1_ros_logs
env ROS_DOMAIN_ID=73 ROS_LOG_DIR=/private/tmp/sts_m1_ros_logs pixi run zsh -c 'source install/setup.zsh && exec ros2 run sts_contract_publisher_cpp ego_state_test_publisher'
env ROS_DOMAIN_ID=73 ROS_LOG_DIR=/private/tmp/sts_m1_ros_logs pixi run zsh -c 'source install/setup.zsh && exec ros2 run sts_contract_subscriber_py ego_state_test_subscriber'
```

In the same environment, live graph commands were verified using `--no-daemon`:

```sh
ros2 node list --no-daemon
ros2 topic list --no-daemon
ros2 topic info /sts/ego/state --verbose --no-daemon
```

DDS needs local network permission; the sandboxed run did not discover the peer
nodes. The permitted runtime check did. Graph results were the two requested nodes,
`/sts/ego/state`, `/parameter_events`, and `/rosout`, with one publisher/subscription.

Repository inspection commands used during this workflow:

```sh
git status --short --untracked-files=all
git log -12 --oneline --decorate
git diff --stat
git diff --check
```
