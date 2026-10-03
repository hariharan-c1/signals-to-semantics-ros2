# Design Decisions

**Status:** The decisions below are accepted because they derive from the approved
architecture. Implementation evidence does not yet exist unless separately marked.

[`DESIGN_SESSION_0_V2.md`](DESIGN_SESSION_0_V2.md) is authoritative. ADRs explain
individual decisions and their consequences; they do not override that document.

## Decision index

| ADR | Decision | Status |
| --- | --- | --- |
| [ADR-001](adr/ADR-001-ros2-jazzy.md) | Use ROS 2 Jazzy as the foundation | Accepted; v0.1 Implemented |
| [ADR-002](adr/ADR-002-cpp-python-boundary.md) | Split deterministic streaming work from ML/data work across C++ and Python | Accepted; M1 proof Implemented; M2.2B C++ ego-state package locally unit-tested; ROS end-to-end verification pending; broader boundary Planned |
| [ADR-003](adr/ADR-003-gt-vs-perception.md) | Support GT and perception actor providers through one common interface | Accepted; implementation Planned |
| [ADR-004](adr/ADR-004-online-vs-offline.md) | Separate offline-parity and causally valid online modes | Accepted; implementation Planned |
| [ADR-005](adr/ADR-005-async-llm.md) | Keep LLM reasoning asynchronous and outside physical processing | Accepted; implementation Planned |
| [ADR-006](adr/ADR-006-remote-carla.md) | Develop on macOS first and run CARLA later in Linux/NVIDIA | Accepted; implementation Planned |
| [ADR-007](adr/ADR-007-standard-ros-interfaces.md) | Prefer standard ROS interfaces and limit custom messages | Accepted; M1 custom interface Implemented; M2.2B Odometry subscription implemented locally; ROS end-to-end verification pending; other source integrations Planned |

## Other approved architecture constraints

- The repository is a systems extension of the existing Signals-to-Semantics
  research pipeline and preserves its traceability.
- Downstream risk and AI components remain independent of CARLA-specific types.
- Spatial values own explicit frames and are normalized before physical risk
  calculations.
- Sensor/risk processing is continuous; S2/S3 processing is event-triggered on a
  frozen scenario window.
- Existing trained models are reused initially; model retraining is not required for
  the first integrated system.
- Native implementation and testing precede Dockerization.
- rosbag2 is a core replay, regression, and demonstration capability, but large bags
  are not committed.
- Packages are introduced incrementally instead of all at once.

## Change procedure

If implementation reveals a conflict, limitation, or materially better design:

1. Record the evidence and affected contract.
2. Stop any implementation that would silently diverge.
3. Propose a new or superseding ADR.
4. Review the impact on interfaces, packages, tests, roadmap, and documentation.
5. Obtain architecture approval before changing the contract or code.

Convenience alone is not a reason to bypass this procedure.
