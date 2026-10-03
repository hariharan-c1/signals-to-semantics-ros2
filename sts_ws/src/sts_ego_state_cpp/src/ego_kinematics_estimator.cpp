// Copyright 2026 Hariharan Chandrasekaran
//
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include "sts_ego_state_cpp/ego_kinematics_estimator.hpp"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace sts_ego_state_cpp
{
namespace
{

uint64_t gap_threshold_ns(double seconds)
{
  if (!std::isfinite(seconds) || seconds <= 0.0) {
    throw std::invalid_argument("max_sample_gap_s must be finite and positive");
  }

  const double nanoseconds = seconds * 1e9;
  if (nanoseconds >= std::ldexp(1.0, 64)) {
    // Such a limit exceeds every possible delta between int64_t timestamps.
    // This representation therefore preserves 'no gap' for every supported input.
    return std::numeric_limits<uint64_t>::max();
  }
  // For integer deltas, dt > a positive threshold is equivalent to dt > floor(threshold).
  // A sub-nanosecond positive limit correctly makes every positive delta a gap.
  return static_cast<uint64_t>(nanoseconds);
}

}  // namespace

EgoKinematicsEstimator::EgoKinematicsEstimator(double max_sample_gap_s)
: max_sample_gap_ns_(gap_threshold_ns(max_sample_gap_s))
{
}

UpdateResult EgoKinematicsEstimator::update(int64_t timestamp_ns, double velocity_mps)
{
  if (!std::isfinite(velocity_mps)) {
    return {UpdateOutcome::RejectedNonFiniteVelocity, std::nullopt};
  }
  if (history_ && timestamp_ns <= history_->timestamp_ns) {
    return {UpdateOutcome::RejectedNonMonotonicTimestamp, std::nullopt};
  }

  KinematicSample sample{timestamp_ns, velocity_mps, std::nullopt, std::nullopt};
  if (!history_) {
    history_ = History{timestamp_ns, velocity_mps, std::nullopt};
    return {UpdateOutcome::Accepted, sample};
  }

  // Ordered signed timestamps can span more than INT64_MAX. Unsigned subtraction
  // gives their exact positive difference without signed overflow, even across zero.
  const uint64_t dt_ns =
    static_cast<uint64_t>(timestamp_ns) - static_cast<uint64_t>(history_->timestamp_ns);
  if (dt_ns > max_sample_gap_ns_) {
    history_ = History{timestamp_ns, velocity_mps, std::nullopt};
    return {UpdateOutcome::AcceptedResetGap, sample};
  }

  const double dt_s = static_cast<double>(dt_ns) * 1e-9;
  const double acceleration = (velocity_mps - history_->velocity_mps) / dt_s;
  if (!std::isfinite(acceleration)) {
    history_ = History{timestamp_ns, velocity_mps, std::nullopt};
    return {UpdateOutcome::AcceptedResetNonFiniteAcceleration, sample};
  }

  sample.acceleration_mps2 = acceleration;
  auto outcome = UpdateOutcome::Accepted;
  if (history_->acceleration_mps2) {
    const double jerk = (acceleration - *history_->acceleration_mps2) / dt_s;
    if (std::isfinite(jerk)) {
      sample.jerk_mps3 = jerk;
    } else {
      outcome = UpdateOutcome::AcceptedNonFiniteJerk;
    }
  }

  history_ = History{timestamp_ns, velocity_mps, acceleration};
  return {outcome, sample};
}

}  // namespace sts_ego_state_cpp
