# ADR-003: Use One Actor Contract for GT and Perception

- **Status:** Accepted
- **Decision scope:** Actor-state providers
- **Implementation status:** Planned for the CARLA/perception stages
- **Authority:** [`../DESIGN_SESSION_0_V2.md`](../DESIGN_SESSION_0_V2.md)

## Context

Downstream risk and actor-ranking behavior must be testable independently of
perception quality, while the complete perception-to-semantics pipeline must also be
evaluated. Direct dependence on CARLA types would couple downstream intelligence to
one simulator and prevent a controlled provider comparison.

## Decision

Support two actor-state providers:

- **Ground-truth mode:** adapt CARLA ground-truth actors to the common
  `ActorStateArray` representation.
- **Perception mode:** detect, localize, track, and estimate actor motion from camera
  and LiDAR/depth before publishing the same `ActorStateArray` representation.

Downstream risk and AI components consume the common interface and do not depend
directly on CARLA types.

## Consequences

- The `ActorStateArray` contract must cover the needs of both providers without
  changing meaning by mode.
- GT mode can isolate downstream algorithms and establish reference results.
- Perception mode evaluates the complete pipeline and its uncertainty.
- GT-vs-perception comparison becomes a required future test layer.
- Frame, timestamp, identity, validity, and uncertainty semantics must be explicit
  before the interface is implemented.
