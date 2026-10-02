# Contributing

Signals-to-Semantics ROS 2 is in **Active Development**. The current milestone is
**v0.2 Streaming Vehicle Core — In Progress**; v0.1 ROS 2 Foundation is
**Implemented** and later milestones are **Planned**. Contributions must preserve
the traceability and staged delivery defined by the approved architecture.

## Before making a change

1. Read [`docs/DESIGN_SESSION_0_V2.md`](docs/DESIGN_SESSION_0_V2.md) in full. It is
   the authoritative architecture.
2. Read [`AGENTS.md`](AGENTS.md), the relevant ADRs, and
   [`docs/ROS_INTERFACE_SPEC.md`](docs/ROS_INTERFACE_SPEC.md).
3. Read the applicable task specification under [`docs/tasks/`](docs/tasks/).
4. Confirm that the work belongs to the current milestone and does not present
   planned behavior as implemented.

## Change workflow

Significant features follow this sequence:

```text
UNDERSTAND -> DESIGN -> SPECIFY CONTRACT -> LEARN REQUIRED CONCEPTS
-> WRITE PSEUDOCODE -> IMPLEMENT -> BUILD -> TEST -> REVIEW -> EXPLAIN -> COMMIT
```

Keep commits focused. Update tests and documentation with behavior changes. Record
architecture-level decisions as ADRs. If implementation exposes a conflict or a
better design, stop and request architectural review instead of silently changing a
contract.

## Status language

Use these labels consistently:

- **Implemented:** exists and has evidence appropriate to its scope.
- **In Progress:** actively being built but not yet accepted.
- **Planned:** approved direction without a delivered implementation.

Documentation, README claims, demos, and releases must not imply that planned
packages or capabilities already work.

## Interfaces and compatibility

- Prefer established ROS messages over custom messages.
- Add a custom interface only for a scenario-specific concept.
- Define frame ownership, units, validity semantics, and QoS before consumers rely
  on an interface.
- Do not change message schemas, topic names, frames, or mode semantics silently.
- Keep CARLA-specific types behind an adapter; downstream components consume common
  ROS interfaces.
- Keep GT and perception providers compatible with the same `ActorStateArray`
  contract.

## Testing expectations

Test behavior, not merely process startup. A numerical component should be checked
against known inputs and tolerances. Add the narrowest useful layer first, then use
ROS integration, rosbag regression, AI regression, CARLA, and end-to-end tests as
those capabilities enter scope.

## Repository hygiene

Do not commit credentials, local overrides, raw datasets, large rosbag recordings,
database volumes, caches, generated ROS build trees, or private/large model
artifacts. Public sanitized configuration belongs in Git. The repository may later
publish deliberately selected small model metadata or safe artifacts; the entire
`models/` directory must not be ignored by default.

Use short, selected, compressed recordings with storage limits when rosbag2 enters
scope. Avoid unnecessary duplicate environments and large generated artifacts on
the constrained local development machine.
