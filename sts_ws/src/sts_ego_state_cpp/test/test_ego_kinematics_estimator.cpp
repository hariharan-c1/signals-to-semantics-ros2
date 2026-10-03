// Copyright 2026 Hariharan Chandrasekaran
//
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>

#include "gtest/gtest.h"
#include "sts_ego_state_cpp/ego_kinematics_estimator.hpp"

namespace sts_ego_state_cpp
{
namespace
{

constexpr double kTolerance = 1e-9;

void expect_sample(
  const UpdateResult & result, UpdateOutcome outcome, int64_t timestamp_ns,
  double velocity_mps, std::optional<double> acceleration_mps2,
  std::optional<double> jerk_mps3)
{
  EXPECT_EQ(result.outcome, outcome);
  ASSERT_TRUE(result.sample.has_value());
  const auto & sample = *result.sample;
  EXPECT_EQ(sample.timestamp_ns, timestamp_ns);
  EXPECT_NEAR(sample.velocity_mps, velocity_mps, kTolerance);
  ASSERT_EQ(sample.acceleration_mps2.has_value(), acceleration_mps2.has_value());
  if (acceleration_mps2) {
    EXPECT_NEAR(*sample.acceleration_mps2, *acceleration_mps2, kTolerance);
  }
  ASSERT_EQ(sample.jerk_mps3.has_value(), jerk_mps3.has_value());
  if (jerk_mps3) {
    EXPECT_NEAR(*sample.jerk_mps3, *jerk_mps3, kTolerance);
  }
}

void expect_rejection(const UpdateResult & result, UpdateOutcome outcome)
{
  EXPECT_EQ(result.outcome, outcome);
  EXPECT_FALSE(result.sample.has_value());
}

TEST(EgoKinematicsEstimator, FirstSampleAllowsZeroTimestampAndOnlyVelocity)
{
  EgoKinematicsEstimator estimator;
  expect_sample(
    estimator.update(0, 10.0), UpdateOutcome::Accepted, 0, 10.0,
    std::nullopt, std::nullopt);
}

TEST(EgoKinematicsEstimator, ConstantVelocityWarmsUpAccelerationThenJerk)
{
  EgoKinematicsEstimator estimator;
  expect_sample(estimator.update(0, 10.0), UpdateOutcome::Accepted, 0, 10.0, {}, {});
  expect_sample(
    estimator.update(100000000, 10.0), UpdateOutcome::Accepted,
    100000000, 10.0, 0.0, {});
  expect_sample(
    estimator.update(200000000, 10.0), UpdateOutcome::Accepted,
    200000000, 10.0, 0.0, 0.0);
  expect_sample(
    estimator.update(300000000, 10.0), UpdateOutcome::Accepted,
    300000000, 10.0, 0.0, 0.0);
}

TEST(EgoKinematicsEstimator, ConstantPositiveAcceleration)
{
  EgoKinematicsEstimator estimator;
  expect_sample(estimator.update(0, 1.0), UpdateOutcome::Accepted, 0, 1.0, {}, {});
  expect_sample(
    estimator.update(100000000, 1.2), UpdateOutcome::Accepted,
    100000000, 1.2, 2.0, {});
  expect_sample(
    estimator.update(200000000, 1.4), UpdateOutcome::Accepted,
    200000000, 1.4, 2.0, 0.0);
  expect_sample(
    estimator.update(300000000, 1.6), UpdateOutcome::Accepted,
    300000000, 1.6, 2.0, 0.0);
}

TEST(EgoKinematicsEstimator, ApprovedBrakingOnsetReferenceSequence)
{
  EgoKinematicsEstimator estimator;
  expect_sample(estimator.update(0, 10.0), UpdateOutcome::Accepted, 0, 10.0, {}, {});
  expect_sample(
    estimator.update(100000000, 10.0), UpdateOutcome::Accepted,
    100000000, 10.0, 0.0, {});
  expect_sample(
    estimator.update(200000000, 9.8), UpdateOutcome::Accepted,
    200000000, 9.8, -2.0, -20.0);
  expect_sample(
    estimator.update(300000000, 9.6), UpdateOutcome::Accepted,
    300000000, 9.6, -2.0, 0.0);
  expect_sample(
    estimator.update(400000000, 9.4), UpdateOutcome::Accepted,
    400000000, 9.4, -2.0, 0.0);
}

TEST(EgoKinematicsEstimator, IrregularIntervalsUseSourceTime)
{
  EgoKinematicsEstimator estimator;
  expect_sample(estimator.update(0, 1.0), UpdateOutcome::Accepted, 0, 1.0, {}, {});
  expect_sample(
    estimator.update(50000000, 1.1), UpdateOutcome::Accepted,
    50000000, 1.1, 2.0, {});
  expect_sample(
    estimator.update(200000000, 1.4), UpdateOutcome::Accepted,
    200000000, 1.4, 2.0, 0.0);
  expect_sample(
    estimator.update(400000000, 1.8), UpdateOutcome::Accepted,
    400000000, 1.8, 2.0, 0.0);
}

TEST(EgoKinematicsEstimator, ReverseMotionPreservesSignedVelocityAndAcceleration)
{
  EgoKinematicsEstimator estimator;
  expect_sample(estimator.update(0, -1.0), UpdateOutcome::Accepted, 0, -1.0, {}, {});
  expect_sample(
    estimator.update(100000000, -1.2), UpdateOutcome::Accepted,
    100000000, -1.2, -2.0, {});
  expect_sample(
    estimator.update(200000000, -1.4), UpdateOutcome::Accepted,
    200000000, -1.4, -2.0, 0.0);
}

TEST(EgoKinematicsEstimator, IrregularIntervalsUseCurrentIntervalForNonzeroJerk)
{
  EgoKinematicsEstimator estimator;
  expect_sample(estimator.update(0, 0.0), UpdateOutcome::Accepted, 0, 0.0, {}, {});
  expect_sample(
    estimator.update(100000000, 0.1), UpdateOutcome::Accepted,
    100000000, 0.1, 1.0, {});
  expect_sample(
    estimator.update(300000000, 0.5), UpdateOutcome::Accepted,
    300000000, 0.5, 2.0, 5.0);
}

TEST(EgoKinematicsEstimator, DuplicateTimestampRejectsWithoutChangingHistory)
{
  EgoKinematicsEstimator estimator;
  estimator.update(0, 10.0);
  estimator.update(100000000, 10.0);
  expect_rejection(
    estimator.update(100000000, 99.0), UpdateOutcome::RejectedNonMonotonicTimestamp);
  expect_sample(
    estimator.update(200000000, 9.8), UpdateOutcome::Accepted,
    200000000, 9.8, -2.0, -20.0);
}

TEST(EgoKinematicsEstimator, DecreasingTimestampRejectsWithoutChangingHistory)
{
  EgoKinematicsEstimator estimator;
  estimator.update(0, 10.0);
  estimator.update(100000000, 10.0);
  expect_rejection(
    estimator.update(50000000, 99.0), UpdateOutcome::RejectedNonMonotonicTimestamp);
  expect_sample(
    estimator.update(200000000, 9.8), UpdateOutcome::Accepted,
    200000000, 9.8, -2.0, -20.0);
}

TEST(EgoKinematicsEstimator, ExactQuarterSecondGapRemainsContinuous)
{
  EgoKinematicsEstimator estimator;
  estimator.update(0, 0.0);
  expect_sample(
    estimator.update(250000000, 0.5), UpdateOutcome::Accepted,
    250000000, 0.5, 2.0, {});
  expect_sample(
    estimator.update(500000000, 1.0), UpdateOutcome::Accepted,
    500000000, 1.0, 2.0, 0.0);
}

TEST(EgoKinematicsEstimator, GapOneNanosecondOverLimitResetsAndWarmsUpAgain)
{
  EgoKinematicsEstimator estimator;
  estimator.update(0, 10.0);
  estimator.update(100000000, 9.8);
  expect_sample(
    estimator.update(350000001, 20.0), UpdateOutcome::AcceptedResetGap,
    350000001, 20.0, {}, {});
  expect_sample(
    estimator.update(450000001, 20.0), UpdateOutcome::Accepted,
    450000001, 20.0, 0.0, {});
  expect_sample(
    estimator.update(550000001, 20.0), UpdateOutcome::Accepted,
    550000001, 20.0, 0.0, 0.0);
}

class NonFiniteVelocity : public ::testing::TestWithParam<double>
{
};

TEST_P(NonFiniteVelocity, RejectsInEmptyState)
{
  EgoKinematicsEstimator estimator;
  expect_rejection(estimator.update(0, GetParam()), UpdateOutcome::RejectedNonFiniteVelocity);
  expect_sample(estimator.update(0, 10.0), UpdateOutcome::Accepted, 0, 10.0, {}, {});
}

TEST_P(NonFiniteVelocity, PreservesVelocityOnlyHistory)
{
  EgoKinematicsEstimator estimator;
  estimator.update(0, 10.0);
  expect_rejection(
    estimator.update(50000000, GetParam()), UpdateOutcome::RejectedNonFiniteVelocity);
  expect_sample(
    estimator.update(100000000, 9.8), UpdateOutcome::Accepted,
    100000000, 9.8, -2.0, {});
}

TEST_P(NonFiniteVelocity, PreservesVelocityAndAccelerationHistory)
{
  EgoKinematicsEstimator estimator;
  estimator.update(0, 10.0);
  estimator.update(100000000, 10.0);
  expect_rejection(
    estimator.update(150000000, GetParam()), UpdateOutcome::RejectedNonFiniteVelocity);
  expect_sample(
    estimator.update(200000000, 9.8), UpdateOutcome::Accepted,
    200000000, 9.8, -2.0, -20.0);
}

INSTANTIATE_TEST_SUITE_P(
  NaNAndBothInfinities, NonFiniteVelocity,
  ::testing::Values(
    std::numeric_limits<double>::quiet_NaN(),
    std::numeric_limits<double>::infinity(),
    -std::numeric_limits<double>::infinity()));

TEST(EgoKinematicsEstimator, TimestampRejectionPreservesVelocityOnlyHistory)
{
  EgoKinematicsEstimator estimator;
  estimator.update(0, 10.0);
  expect_rejection(estimator.update(0, 99.0), UpdateOutcome::RejectedNonMonotonicTimestamp);
  expect_rejection(estimator.update(-1, 99.0), UpdateOutcome::RejectedNonMonotonicTimestamp);
  expect_sample(
    estimator.update(100000000, 9.8), UpdateOutcome::Accepted,
    100000000, 9.8, -2.0, {});
}

TEST(EgoKinematicsEstimator, AccelerationSubtractionOverflowRestartsFromCurrentVelocity)
{
  const double velocity = std::ldexp(1.0, 1023);
  ASSERT_TRUE(std::isfinite(velocity));
  EgoKinematicsEstimator estimator;
  estimator.update(0, -velocity);
  expect_sample(
    estimator.update(125000000, velocity), UpdateOutcome::AcceptedResetNonFiniteAcceleration,
    125000000, velocity, {}, {});
  expect_sample(
    estimator.update(250000000, velocity), UpdateOutcome::Accepted,
    250000000, velocity, 0.0, {});
  expect_sample(
    estimator.update(375000000, velocity), UpdateOutcome::Accepted,
    375000000, velocity, 0.0, 0.0);
}

TEST(EgoKinematicsEstimator, AccelerationDivisionOverflowRestartsFromCurrentVelocity)
{
  const double velocity = std::ldexp(1.0, 1023);
  EgoKinematicsEstimator estimator;
  estimator.update(0, 0.0);
  expect_sample(
    estimator.update(125000000, velocity), UpdateOutcome::AcceptedResetNonFiniteAcceleration,
    125000000, velocity, {}, {});
  expect_sample(
    estimator.update(250000000, velocity), UpdateOutcome::Accepted,
    250000000, velocity, 0.0, {});
  expect_sample(
    estimator.update(375000000, velocity), UpdateOutcome::Accepted,
    375000000, velocity, 0.0, 0.0);
}

TEST(EgoKinematicsEstimator, AccelerationOverflowClearsExistingAccelerationHistory)
{
  const double velocity = std::ldexp(1.0, 1023);
  const double increment = std::ldexp(1.0, 1003);
  const double prior_acceleration = std::ldexp(1.0, 1006);
  ASSERT_TRUE(std::isfinite(velocity));
  ASSERT_TRUE(std::isfinite(prior_acceleration));
  EgoKinematicsEstimator estimator;
  expect_sample(
    estimator.update(0, velocity - increment), UpdateOutcome::Accepted,
    0, velocity - increment, {}, {});
  // Establish HAVE_ACCEL with a finite, nonzero acceleration before overflowing.
  expect_sample(
    estimator.update(125000000, velocity), UpdateOutcome::Accepted,
    125000000, velocity, prior_acceleration, {});
  expect_sample(
    estimator.update(250000000, 0.0), UpdateOutcome::AcceptedResetNonFiniteAcceleration,
    250000000, 0.0, {}, {});
  // Ordinary velocities must use the overflow sample's zero-velocity baseline.
  // The old finite acceleration would produce a finite unwanted jerk here.
  expect_sample(
    estimator.update(375000000, 0.125), UpdateOutcome::Accepted,
    375000000, 0.125, 1.0, {});
  expect_sample(
    estimator.update(500000000, 0.375), UpdateOutcome::Accepted,
    500000000, 0.375, 2.0, 8.0);
}

TEST(EgoKinematicsEstimator, NonFiniteJerkRetainsValidAccelerationHistory)
{
  const double velocity = std::ldexp(1.0, 1019);
  const double acceleration = std::ldexp(1.0, 1022);
  EgoKinematicsEstimator estimator;
  estimator.update(0, 0.0);
  expect_sample(
    estimator.update(125000000, -velocity), UpdateOutcome::Accepted,
    125000000, -velocity, -acceleration, {});
  expect_sample(
    estimator.update(250000000, 0.0), UpdateOutcome::AcceptedNonFiniteJerk,
    250000000, 0.0, acceleration, {});
  expect_sample(
    estimator.update(375000000, velocity), UpdateOutcome::Accepted,
    375000000, velocity, acceleration, 0.0);
}

TEST(EgoKinematicsEstimator, NanosecondOrderingSurvivesLargeAbsoluteTimestamps)
{
  constexpr int64_t epoch = 2000000000000000000LL;
  EgoKinematicsEstimator estimator;
  estimator.update(epoch, 1.0);
  expect_sample(
    estimator.update(epoch + 1, 1.0), UpdateOutcome::Accepted,
    epoch + 1, 1.0, 0.0, {});
  expect_rejection(
    estimator.update(epoch, 99.0), UpdateOutcome::RejectedNonMonotonicTimestamp);
  expect_sample(
    estimator.update(epoch + 2, 1.0), UpdateOutcome::Accepted,
    epoch + 2, 1.0, 0.0, 0.0);
}

TEST(EgoKinematicsEstimator, ExtremeTimestampSpanDoesNotOverflow)
{
  EgoKinematicsEstimator estimator;
  estimator.update(std::numeric_limits<int64_t>::min(), 1.0);
  expect_sample(
    estimator.update(std::numeric_limits<int64_t>::max(), 2.0),
    UpdateOutcome::AcceptedResetGap, std::numeric_limits<int64_t>::max(), 2.0, {}, {});
}

TEST(EgoKinematicsEstimator, GapParameterMustBeFiniteAndPositive)
{
  EXPECT_THROW(EgoKinematicsEstimator{0.0}, std::invalid_argument);
  EXPECT_THROW(EgoKinematicsEstimator{-1.0}, std::invalid_argument);
  EXPECT_THROW(
    EgoKinematicsEstimator{std::numeric_limits<double>::quiet_NaN()}, std::invalid_argument);
  EXPECT_THROW(
    EgoKinematicsEstimator{std::numeric_limits<double>::infinity()}, std::invalid_argument);
  EXPECT_THROW(
    EgoKinematicsEstimator{-std::numeric_limits<double>::infinity()}, std::invalid_argument);
}

TEST(EgoKinematicsEstimator, CustomGapParameterUsesStrictIntegerBoundary)
{
  EgoKinematicsEstimator estimator{0.1};
  estimator.update(0, 1.0);
  expect_sample(
    estimator.update(100000000, 1.0), UpdateOutcome::Accepted,
    100000000, 1.0, 0.0, {});
  expect_sample(
    estimator.update(200000001, 1.0), UpdateOutcome::AcceptedResetGap,
    200000001, 1.0, {}, {});
}

TEST(EgoKinematicsEstimator, PositiveSubNanosecondGapMakesEveryPositiveDeltaDiscontinuous)
{
  EgoKinematicsEstimator estimator{0.5e-9};
  estimator.update(0, 1.0);
  expect_sample(
    estimator.update(1, 1.0), UpdateOutcome::AcceptedResetGap, 1, 1.0, {}, {});
}

TEST(EgoKinematicsEstimator, LargeFiniteGapParameterRemainsSupported)
{
  EgoKinematicsEstimator estimator{std::numeric_limits<double>::max()};
  estimator.update(std::numeric_limits<int64_t>::min(), 1.0);
  expect_sample(
    estimator.update(std::numeric_limits<int64_t>::max(), 1.0), UpdateOutcome::Accepted,
    std::numeric_limits<int64_t>::max(), 1.0, 0.0, {});
}

TEST(EgoKinematicsEstimator, EachInstanceOwnsIndependentHistory)
{
  EgoKinematicsEstimator first;
  EgoKinematicsEstimator second;
  first.update(0, 10.0);
  first.update(100000000, 9.8);
  expect_sample(second.update(0, -1.0), UpdateOutcome::Accepted, 0, -1.0, {}, {});
  expect_sample(
    first.update(200000000, 9.6), UpdateOutcome::Accepted,
    200000000, 9.6, -2.0, 0.0);
  expect_sample(
    second.update(100000000, -1.2), UpdateOutcome::Accepted,
    100000000, -1.2, -2.0, {});
}

}  // namespace
}  // namespace sts_ego_state_cpp
