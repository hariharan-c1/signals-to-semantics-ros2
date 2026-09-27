# Online vs Offline-Parity Modes

**Status:** Architectural behavior approved; mode implementations are planned.

The original Signals-to-Semantics research pipeline primarily operates offline on
completed scenario windows. The ROS 2 extension must preserve a reproducible path to
that behavior while separately supporting causally valid streaming operation.

## Mode definitions

### Offline-parity mode

Offline-parity mode may use a complete recorded scenario window, including samples
that occur after an earlier point in the scenario. Its purposes are:

- regression against thesis behavior;
- algorithm comparison; and
- reproducibility.

Access to a complete window is an explicit evaluation condition. Offline-parity
results must not be represented as evidence of online performance.

### Online mode

Online mode may use only information available at the current time and in the past.
It must never use future ground-truth trajectory samples. Estimates of future
interaction must come from an explicit prediction model such as:

- constant velocity;
- constant acceleration; or
- CTRV where appropriate.

An algorithm that smooths using future samples must be replaced by causal filtering
or declared and implemented as fixed-latency processing.

## Behavioral comparison

| Concern | Offline-parity | Online |
| --- | --- | --- |
| Input horizon | Complete recorded window permitted | Present and past only |
| Future GT trajectory samples | Permitted for parity where the thesis behavior uses the completed window | Prohibited |
| Future interaction | May be evaluated from the completed record for comparison | Must be predicted |
| Smoothing | May reproduce non-causal offline behavior | Must be causal or explicitly fixed-latency |
| Primary purpose | Reproducibility and thesis regression | Streaming operation |

## Shared behavior

Both modes should use the same approved ROS interfaces and preserve traceability
through the processing stages. The difference lies in information availability and
algorithm behavior, not in silently changing message meaning.

## Evaluation requirements

When these modes are implemented, documentation and evaluation must identify:

1. Which mode produced each reported result.
2. The information horizon available to every compared algorithm.
3. The prediction model used online.
4. Whether filtering is causal or has a declared fixed latency.
5. The effect of online constraints on physical metrics, events, S2/S3 ranking, and
   downstream semantics.

Comparisons must not leak future ground truth into the online path. Differences
between offline-parity and online outputs are expected to be measured and explained,
not hidden.
