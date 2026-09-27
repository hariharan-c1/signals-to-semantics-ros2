# Package Map

**Status:** Planned package architecture. No ROS packages are implemented yet.  
**Current delivery:** v0.1 introduces only the packages required by the approved
M1 task after their contracts are finalized.

The package names come from the approved architecture. “Primary implementation” is
derived from the approved C++/Python boundary; entries marked “To be decided” do not
yet have an approved language or executable layout.

| Package | Intended responsibility | Primary implementation | Earliest roadmap stage | Status |
| --- | --- | --- | --- | --- |
| `sts_interfaces` | Scenario-specific ROS message definitions | ROS interface definitions | v0.1 | In Progress |
| `sts_bringup` | System launch and integration entry points | To be decided | v0.1 foundation, expanded later | Planned |
| `sts_replay` | Recorded-scenario and rosbag2 replay support | To be decided | v0.4 | Planned |
| `sts_carla_adapter` | Convert CARLA data, including GT actors, to common ROS interfaces | To be decided | v0.6 | Planned |
| `sts_perception` | Detection and spatial perception | Python / `rclpy` | v0.5 | Planned |
| `sts_tracking` | Initial actor tracking and velocity estimation | Python / `rclpy` initially | v0.5 | Planned |
| `sts_ego_state_cpp` | Deterministic streaming ego-state processing | C++ / `rclcpp` | v0.2 | Planned |
| `sts_actor_transform_cpp` | Frame-aware actor normalization where appropriate | C++ / `rclcpp` | v0.4 | Planned |
| `sts_risk_engine_cpp` | Relative geometry, TTC/THW/DCA, interaction, and risk | C++ / `rclcpp` | v0.3 | Planned |
| `sts_event_detector_cpp` | Safety-relevant event detection | C++ / `rclcpp` | v0.2 | Planned |
| `sts_window_manager_cpp` | Rolling history and frozen scenario-window management | C++ / `rclcpp` | v0.2 and later integration | Planned |
| `sts_feature_adapter` | Adapt ROS scenario windows to thesis features | Python / `rclpy` | v0.7 | Planned |
| `sts_actor_ranker` | S2 representation and S3 GATv2 inference | Python / `rclpy` | v0.7 | Planned |
| `sts_evidence` | S4 deterministic evidence construction | Python / `rclpy` | v0.8 | Planned |
| `sts_semantic_reasoner` | Asynchronous S5 constrained semantic reasoning | Python / `rclpy` | v0.8 | Planned |
| `sts_database` | S6 PostgreSQL/pgvector integration | Python / `rclpy` | v0.9 | Planned |
| `sts_retrieval` | S7 semantic retrieval and reranking | Python / `rclpy` | v0.9 | Planned |
| `sts_visualization_cpp` | RViz2 engineering and scenario visualization | C++ / `rclcpp` | v0.3, expanded later | Planned |

## Dependency direction

The intended dependency direction is:

```text
source adapters / perception / tracking
                  -> sts_interfaces
                  -> deterministic C++ streaming packages
                  -> feature adapter and S2-S5 packages
                  -> S6/S7 packages and visualization
```

Shared message contracts should prevent downstream packages from importing CARLA
types or binding directly to a specific actor provider. Exact package dependencies,
node names, executables, and launch files remain unapproved until their milestone
task specifies them.

## Package creation rule

Create packages only when their release task enters scope. An empty placeholder
package does not count as progress and should not be added to make the target map
appear implemented.
