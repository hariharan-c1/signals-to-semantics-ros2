// Copyright 2026 Hariharan Chandrasekaran
//
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <cmath>
#include <cstdint>
#include <exception>
#include <memory>
#include <stdexcept>

#include "nav_msgs/msg/odometry.hpp"
#include "rcl_interfaces/msg/parameter_descriptor.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sts_ego_state_cpp/ego_kinematics_estimator.hpp"
#include "sts_interfaces/msg/ego_state.hpp"

namespace sts_ego_state_cpp
{

class EgoStateNode : public rclcpp::Node
{
public:
  EgoStateNode()
  : Node("ego_state"), estimator_(declare_gap_parameter())
  {
    auto output_qos = rclcpp::QoS(rclcpp::KeepLast(10));
    output_qos.reliable();
    output_qos.durability_volatile();
    publisher_ = create_publisher<sts_interfaces::msg::EgoState>(
      "/sts/ego/state", output_qos);
    subscription_ = create_subscription<nav_msgs::msg::Odometry>(
      "/vehicle/odometry", rclcpp::SensorDataQoS(),
      [this](nav_msgs::msg::Odometry::ConstSharedPtr message) {on_odometry(*message);});
  }

private:
  double declare_gap_parameter()
  {
    rcl_interfaces::msg::ParameterDescriptor descriptor;
    descriptor.description = "Maximum continuous source sample interval in seconds";
    descriptor.additional_constraints = "Finite and strictly positive; startup configuration";
    // Avoid accepting runtime changes that would disagree with the constructed estimator.
    descriptor.read_only = true;
    const double seconds = declare_parameter<double>("max_sample_gap_s", 0.25, descriptor);
    if (!std::isfinite(seconds) || seconds <= 0.0) {
      throw std::invalid_argument("max_sample_gap_s must be finite and positive");
    }
    return seconds;
  }

  void on_odometry(const nav_msgs::msg::Odometry & input)
  {
    if (input.child_frame_id != "base_link") {
      RCLCPP_WARN(get_logger(), "Rejected odometry: child_frame_id must be base_link");
      return;
    }
    // sec is already int32_t on the wire. Check nanosec before any normalization.
    if (input.header.stamp.nanosec >= 1000000000u) {
      RCLCPP_WARN(get_logger(), "Rejected odometry: malformed source timestamp");
      return;
    }
    const int64_t timestamp_ns =
      static_cast<int64_t>(input.header.stamp.sec) * 1000000000LL +
      static_cast<int64_t>(input.header.stamp.nanosec);
    const auto result = estimator_.update(timestamp_ns, input.twist.twist.linear.x);
    switch (result.outcome) {
      case UpdateOutcome::RejectedNonMonotonicTimestamp:
        RCLCPP_WARN(get_logger(), "Rejected odometry: source timestamp did not increase");
        break;
      case UpdateOutcome::RejectedNonFiniteVelocity:
        RCLCPP_WARN(get_logger(), "Rejected odometry: non-finite longitudinal velocity");
        break;
      case UpdateOutcome::AcceptedResetGap:
        RCLCPP_WARN(get_logger(), "Reset ego derivative history: source sample gap");
        break;
      case UpdateOutcome::AcceptedResetNonFiniteAcceleration:
        RCLCPP_WARN(get_logger(), "Reset ego derivative history: non-finite acceleration");
        break;
      case UpdateOutcome::AcceptedNonFiniteJerk:
        RCLCPP_WARN(get_logger(), "Ego jerk unavailable: non-finite calculated jerk");
        break;
      case UpdateOutcome::Accepted:
        break;
    }
    if (!result.sample) {
      return;
    }

    const auto & sample = *result.sample;
    sts_interfaces::msg::EgoState output;
    output.header.stamp = input.header.stamp;
    output.header.frame_id = "base_link";
    output.longitudinal_velocity_mps = sample.velocity_mps;
    output.longitudinal_acceleration_mps2 = sample.acceleration_mps2.value_or(0.0);
    output.longitudinal_jerk_mps3 = sample.jerk_mps3.value_or(0.0);
    output.velocity_valid = true;
    output.acceleration_valid = sample.acceleration_mps2.has_value();
    output.jerk_valid = sample.jerk_mps3.has_value();
    publisher_->publish(output);
  }

  EgoKinematicsEstimator estimator_;
  rclcpp::Publisher<sts_interfaces::msg::EgoState>::SharedPtr publisher_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr subscription_;
};

}  // namespace sts_ego_state_cpp

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  int exit_code = 0;
  try {
    rclcpp::spin(std::make_shared<sts_ego_state_cpp::EgoStateNode>());
  } catch (const std::exception & error) {
    RCLCPP_ERROR(rclcpp::get_logger("ego_state"), "Ego state node failed: %s", error.what());
    exit_code = 1;
  }
  rclcpp::shutdown();
  return exit_code;
}
