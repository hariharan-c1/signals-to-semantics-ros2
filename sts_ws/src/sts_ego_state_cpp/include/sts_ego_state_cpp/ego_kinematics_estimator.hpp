// Copyright 2026 Hariharan Chandrasekaran
//
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#ifndef STS_EGO_STATE_CPP__EGO_KINEMATICS_ESTIMATOR_HPP_
#define STS_EGO_STATE_CPP__EGO_KINEMATICS_ESTIMATOR_HPP_

#include <cstdint>
#include <optional>

namespace sts_ego_state_cpp
{

enum class UpdateOutcome
{
  Accepted,
  AcceptedResetGap,
  AcceptedResetNonFiniteAcceleration,
  AcceptedNonFiniteJerk,
  RejectedNonMonotonicTimestamp,
  RejectedNonFiniteVelocity
};

struct KinematicSample
{
  int64_t timestamp_ns;
  double velocity_mps;
  std::optional<double> acceleration_mps2;
  std::optional<double> jerk_mps3;
};

struct UpdateResult
{
  UpdateOutcome outcome;
  // No sample means rejection. Every accepted outcome supplies current velocity.
  std::optional<KinematicSample> sample;
};

// ROS-independent causal estimator; signed longitudinal quantities use SI units.
class EgoKinematicsEstimator
{
public:
  // Converts a finite positive gap to integer nanoseconds once at construction.
  explicit EgoKinematicsEstimator(double max_sample_gap_s = 0.25);

  // Zero is a valid initial timestamp. Rejections leave all history unchanged.
  UpdateResult update(int64_t timestamp_ns, double velocity_mps);

private:
  struct History
  {
    int64_t timestamp_ns;
    double velocity_mps;
    std::optional<double> acceleration_mps2;
  };

  const uint64_t max_sample_gap_ns_;
  // EMPTY / HAVE_VELOCITY / HAVE_ACCEL derive from these two optional presences.
  std::optional<History> history_;
};

}  // namespace sts_ego_state_cpp

#endif  // STS_EGO_STATE_CPP__EGO_KINEMATICS_ESTIMATOR_HPP_
