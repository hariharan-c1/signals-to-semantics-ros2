# ADR-005: Keep LLM Reasoning Asynchronous

- **Status:** Accepted
- **Decision scope:** Semantic-reasoning boundary
- **Implementation status:** Planned for v0.8
- **Authority:** [`../DESIGN_SESSION_0_V2.md`](../DESIGN_SESSION_0_V2.md)

## Context

S5 constrained semantic reasoning enriches a detected scenario, but LLM latency and
availability are unsuitable dependencies for physical or safety-relevant streaming
processing.

## Decision

The LLM never belongs to the physical or safety-critical path. When an event is
detected, physical results proceed immediately to visualization and storage while a
separate asynchronous semantic job invokes S5. Semantic output enriches the scenario
when available.

## Consequences

- Risk, event, and scenario-window processing complete without waiting for S5.
- LLM timeout, latency, or failure cannot stop physical processing.
- The eventual semantic job boundary needs explicit status and failure handling.
- Physical evidence must remain available even when semantic enrichment is absent.
- S5 latency is measured separately from deterministic processing latency.
