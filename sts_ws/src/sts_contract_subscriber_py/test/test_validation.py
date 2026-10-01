# Copyright 2026 Hariharan Chandrasekaran
#
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

"""Test canonical EgoState payload validation."""

import unittest

from sts_contract_subscriber_py.ego_state_test_subscriber import validate_message
from sts_interfaces.msg import EgoState


def make_canonical_message() -> EgoState:
    """Construct the documented M1 canonical payload for validation tests."""
    message = EgoState()
    message.header.stamp.sec = 1
    message.header.frame_id = 'base_link'
    message.longitudinal_velocity_mps = 13.5
    message.longitudinal_acceleration_mps2 = -2.25
    message.longitudinal_jerk_mps3 = -4.0
    message.velocity_valid = True
    message.acceleration_valid = True
    message.jerk_valid = True
    return message


class TestValidation(unittest.TestCase):
    """Verify strict canonical payload validation."""

    def test_canonical_message_passes(self) -> None:
        """Accept the complete documented canonical payload."""
        self.assertEqual(validate_message(make_canonical_message()), [])

    def test_every_incorrect_field_is_reported(self) -> None:
        """Report every invalid documented field rather than tolerating it."""
        message = EgoState()
        message.header.frame_id = 'odom'
        message.longitudinal_velocity_mps = 12.0
        message.longitudinal_acceleration_mps2 = 1.0
        message.longitudinal_jerk_mps3 = 2.0

        mismatches = '\n'.join(validate_message(message))

        self.assertIn('header.stamp', mismatches)
        self.assertIn('header.frame_id', mismatches)
        self.assertIn('longitudinal_velocity_mps', mismatches)
        self.assertIn('longitudinal_acceleration_mps2', mismatches)
        self.assertIn('longitudinal_jerk_mps3', mismatches)
        self.assertIn('velocity_valid', mismatches)
        self.assertIn('acceleration_valid', mismatches)
        self.assertIn('jerk_valid', mismatches)
