// Copyright 2026 Hariharan Chandrasekaran
//
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sts_interfaces/msg/ego_state.hpp"

using namespace std::chrono_literals;

class EgoStateTestPublisher : public rclcpp::Node
{
public:
  EgoStateTestPublisher()
  : Node("ego_state_test_publisher")
  {
    auto qos = rclcpp::QoS(rclcpp::KeepLast(10));
    qos.reliable();
    qos.durability_volatile();

    publisher_ = create_publisher<sts_interfaces::msg::EgoState>(
      "/sts/ego/state", qos);
    timer_ = create_wall_timer(1s, [this]() {publish_canonical_payload();});
  }

private:
  void publish_canonical_payload()
  {
    sts_interfaces::msg::EgoState message;
    message.header.stamp = get_clock()->now();
    message.header.frame_id = "base_link";
    message.longitudinal_velocity_mps = 13.5;
    message.longitudinal_acceleration_mps2 = -2.25;
    message.longitudinal_jerk_mps3 = -4.0;
    message.velocity_valid = true;
    message.acceleration_valid = true;
    message.jerk_valid = true;

    publisher_->publish(message);
    RCLCPP_INFO(get_logger(), "Published canonical EgoState payload");
  }

  rclcpp::Publisher<sts_interfaces::msg::EgoState>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<EgoStateTestPublisher>());
  rclcpp::shutdown();
  return 0;
}
