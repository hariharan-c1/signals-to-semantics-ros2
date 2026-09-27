# ADR-004: Separate Offline-Parity from Online Operation

- **Status:** Accepted
- **Decision scope:** Temporal information boundary
- **Implementation status:** Planned
- **Authority:** [`../DESIGN_SESSION_0_V2.md`](../DESIGN_SESSION_0_V2.md)

## Context

The original thesis pipeline primarily processes completed scenario windows. A
streaming ROS system cannot claim online behavior if it uses samples that were not
available at the time of a decision.

## Decision

Provide two explicitly distinguished modes:

- **Offline-parity mode** may use complete recorded windows for thesis regression,
  algorithm comparison, and reproducibility.
- **Online mode** may use only current and past information and must never use future
  ground-truth trajectory samples.

Online future-interaction estimates use prediction approaches such as constant
velocity, constant acceleration, or CTRV where appropriate. Non-causal smoothing is
replaced by causal filtering or declared fixed-latency processing.

## Consequences

- Results and evaluations must identify which mode and information horizon they use.
- Offline-parity performance cannot be presented as online performance.
- Online algorithms need future-prediction and causal-filter tests.
- The effects of causal constraints and fixed latency must be documented and
  evaluated.
- Future-data leakage is a correctness failure in online mode.

See [`../ONLINE_VS_OFFLINE.md`](../ONLINE_VS_OFFLINE.md) for the derived operating
contract.
