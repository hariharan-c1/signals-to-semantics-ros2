# Copyright 2026 Hariharan Chandrasekaran
#
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

"""Subscribe to EgoState and validate the canonical M1 payload."""

from typing import List

import rclpy
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy
from rclpy.qos import HistoryPolicy
from rclpy.qos import QoSProfile
from rclpy.qos import ReliabilityPolicy
from sts_interfaces.msg import EgoState


CANONICAL_FIELDS = {
    'header.frame_id': 'base_link',
    'longitudinal_velocity_mps': 13.5,
    'longitudinal_acceleration_mps2': -2.25,
    'longitudinal_jerk_mps3': -4.0,
    'velocity_valid': True,
    'acceleration_valid': True,
    'jerk_valid': True,
}


def validate_message(message: EgoState) -> List[str]:
    """Return field-level mismatches from the canonical EgoState payload."""
    mismatches = []

    stamp = message.header.stamp
    if stamp.sec == 0 and stamp.nanosec == 0:
        mismatches.append(
            'header.stamp: expected non-zero ROS clock time, received 0.0'
        )

    actual_fields = {
        'header.frame_id': message.header.frame_id,
        'longitudinal_velocity_mps': message.longitudinal_velocity_mps,
        'longitudinal_acceleration_mps2': (
            message.longitudinal_acceleration_mps2
        ),
        'longitudinal_jerk_mps3': message.longitudinal_jerk_mps3,
        'velocity_valid': message.velocity_valid,
        'acceleration_valid': message.acceleration_valid,
        'jerk_valid': message.jerk_valid,
    }

    for field_name, expected in CANONICAL_FIELDS.items():
        actual = actual_fields[field_name]
        if actual != expected:
            mismatches.append(
                f'{field_name}: expected {expected!r}, received {actual!r}'
            )

    return mismatches


class EgoStateTestSubscriber(Node):
    """Validate every EgoState received on the approved M1 topic."""

    def __init__(self) -> None:
        """Create the subscriber with the approved M1 QoS profile."""
        super().__init__('ego_state_test_subscriber')
        qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=10,
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.VOLATILE,
        )
        self.subscription = self.create_subscription(
            EgoState,
            '/sts/ego/state',
            self._on_message,
            qos,
        )

    def _on_message(self, message: EgoState) -> None:
        """Log an explicit result for every received payload."""
        mismatches = validate_message(message)
        if mismatches:
            details = '\n  - '.join(mismatches)
            self.get_logger().error(
                f'FAIL canonical EgoState payload:\n  - {details}'
            )
            return

        self.get_logger().info('PASS canonical EgoState payload')


def main(args=None) -> None:
    """Run the M1 EgoState contract-verification subscriber."""
    rclpy.init(args=args)
    node = EgoStateTestSubscriber()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
