# Copyright 2026 Hariharan Chandrasekaran
#
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

"""Check Python source docstrings with pydocstyle."""

import unittest

from ament_pep257.main import main


class TestPep257(unittest.TestCase):
    """Run the ament PEP 257 linter."""

    def test_pep257(self) -> None:
        """Check all Python source files for docstring violations."""
        rc = main(argv=['.', 'test'])
        self.assertEqual(rc, 0, 'Found code style errors / warnings')
