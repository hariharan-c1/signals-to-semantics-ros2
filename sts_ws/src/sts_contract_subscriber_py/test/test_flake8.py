# Copyright 2026 Hariharan Chandrasekaran
#
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

"""Check Python source files with flake8."""

import unittest

from ament_flake8.main import main_with_errors


class TestFlake8(unittest.TestCase):
    """Run the ament flake8 linter."""

    def test_flake8(self) -> None:
        """Check all Python source files for flake8 violations."""
        rc, errors = main_with_errors(argv=[])
        self.assertEqual(rc, 0, '\n'.join(errors))
