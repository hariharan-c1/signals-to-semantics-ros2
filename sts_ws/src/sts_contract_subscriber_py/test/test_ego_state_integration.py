# Copyright 2026 Hariharan Chandrasekaran
#
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

"""Verify the installed C++ to Python EgoState contract exchange."""

import os
from pathlib import Path
import random
import sys

from ament_index_python.packages import get_package_prefix
from launch import LaunchDescription
import launch_pytest
from launch_pytest.tools.process import wait_for_stderr_sync
from launch_ros.actions import Node
import pytest


PASS_TIMEOUT_SECONDS = 30.0


@launch_pytest.fixture
def ego_state_launch(tmp_path):
    """Launch both installed executables with a shared test-only ROS domain."""
    node_environment = {
        'ROS_DOMAIN_ID': str(random.SystemRandom().randrange(1, 100)),
        'ROS_LOG_DIR': str(tmp_path),
        # ROS logs go to stderr; capture the subscriber's output explicitly.
        'RCUTILS_LOGGING_USE_STDOUT': '0',
    }
    if sys.platform == 'darwin':
        # macOS system shells can strip DYLD_* during colcon test activation.
        # Restore the installed interface library path for these child processes.
        interface_lib = str(Path(get_package_prefix('sts_interfaces')) / 'lib')
        library_paths = [interface_lib]
        if os.environ.get('DYLD_LIBRARY_PATH'):
            library_paths.append(os.environ['DYLD_LIBRARY_PATH'])
        node_environment['DYLD_LIBRARY_PATH'] = os.pathsep.join(library_paths)
    publisher = Node(
        package='sts_contract_publisher_cpp',
        executable='ego_state_test_publisher',
        output='screen',
        additional_env=node_environment,
        sigterm_timeout='2',
        sigkill_timeout='2',
    )
    subscriber = Node(
        package='sts_contract_subscriber_py',
        executable='ego_state_test_subscriber',
        output='screen',
        cached_output=True,
        additional_env=node_environment,
        sigterm_timeout='2',
        sigkill_timeout='2',
    )
    return LaunchDescription([publisher, subscriber]), subscriber


@pytest.mark.launch(fixture=ego_state_launch)
def test_canonical_payload_reaches_python(ego_state_launch, launch_context):
    """Require the real subscriber's canonical PASS within a bounded timeout."""
    subscriber = ego_state_launch[1]
    observed_pass = wait_for_stderr_sync(
        launch_context,
        subscriber,
        lambda output: 'PASS canonical EgoState payload' in output,
        timeout=PASS_TIMEOUT_SECONDS,
    )
    assert observed_pass, (
        f'Subscriber did not emit PASS within {PASS_TIMEOUT_SECONDS} seconds. '
        f'Subscriber stderr: {subscriber.get_stderr()!r}'
    )
