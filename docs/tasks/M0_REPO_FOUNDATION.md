# M0.1 — Repository Foundation

**Status:** Implemented  
**Project status:** Active Development  
**Next milestone:** v0.1 ROS 2 Foundation

## Objective

Create a truthful, architecture-aligned public repository foundation before any ROS
implementation begins. This task produces documentation and repository policy only.

## Deliverables

Repository root:

- `README.md`
- `ROADMAP.md`
- `CONTRIBUTING.md`
- `LICENSE`
- `AGENTS.md`
- `.gitignore`

Architecture and project documentation:

- `docs/PROJECT_CHARTER.md`
- `docs/ARCHITECTURE.md`
- `docs/PACKAGE_MAP.md`
- `docs/ROS_INTERFACE_SPEC.md`
- `docs/ROS_GRAPH.md`
- `docs/TF_TREE.md`
- `docs/QOS.md`
- `docs/ONLINE_VS_OFFLINE.md`
- `docs/DESIGN_DECISIONS.md`
- `docs/TESTING.md`
- `docs/MILESTONES.md`
- `docs/LEARNING_ROADMAP.md`
- `docs/INTERVIEW_NOTES.md`

Task and decision records:

- `docs/tasks/M0_REPO_FOUNDATION.md`
- `docs/tasks/M1_ROS_FOUNDATION.md`
- ADR-001 through ADR-007 under `docs/adr/`

## Exact acceptance criteria

M0.1 is accepted only when all of the following are true:

1. Every listed deliverable exists, is non-empty, and is readable as Markdown or,
   for `LICENSE` and `.gitignore`, its appropriate plain-text format.
2. `docs/DESIGN_SESSION_0_V2.md` remains unmodified by M0.1 and is identified as the
   authoritative approved architecture.
3. All derived documents are consistent with that architecture and reference no
   unapproved implemented functionality.
4. Project status is **Active Development** and the current milestone is
   **v0.1 ROS 2 Foundation** wherever current status is stated.
5. The repository clearly distinguishes **Implemented**, **In Progress**, and
   **Planned** work.
6. The README addresses both recruiters and engineers, links to the original
   [Signals-to-Semantics repository](https://github.com/hariharan-c1/Signals_to_Semantics),
   and presents this repository as its systems extension.
7. The Apple M1 Pro/macOS development constraint and future Linux/NVIDIA CARLA/GPU
   phase are represented accurately.
8. `AGENTS.md` requires future coding agents to read the authoritative architecture,
   relevant ADRs, interface contracts, and task specification before implementation.
9. `AGENTS.md` prohibits silent architecture and interface changes.
10. `LICENSE` contains the MIT License.
11. `.gitignore` covers macOS artifacts, Python caches and environments, colcon
    `build/`, `install/`, and `log/`, IDE state, credentials/local overrides, local
    database data, raw/local datasets, and rosbag recordings.
12. `.gitignore` does not ignore `models/` as a whole and does not ignore public
    `config/` as a whole; it targets only private/local model paths and secret/local
    configuration overrides.
13. No secrets or machine-specific absolute paths are added.
14. No production source code, ROS packages, placeholder implementations, Docker
    files, model checkpoints, datasets, or dependency installations are added.
15. `docs/tasks/M1_ROS_FOUNDATION.md` is limited to ROS 2 Jazzy, a colcon workspace,
    `sts_interfaces`, one custom message, one C++ ROS node, one Python ROS node,
    cross-language communication, basic tests, and a CI baseline.
16. Repository links and requested filenames are checked for consistency.
17. The complete repository tree and final `git status` are reviewed.
18. No Git commit is created as part of M0.1.

## Explicit exclusions

M0.1 does not create or install a ROS environment, initialize a colcon workspace,
create packages, define an unapproved message schema, add CI workflow code, add
Docker configuration, download data/models, or implement any runtime behavior.

## Verification evidence

Acceptance should be supported by a file inventory, empty-file check, link/contract
review, architecture-file integrity check, prohibited-artifact scan, and `git
status`. These checks do not constitute a commit.
