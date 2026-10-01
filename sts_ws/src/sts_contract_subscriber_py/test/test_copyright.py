# Copyright 2026 Hariharan Chandrasekaran
#
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.

"""Check source files for copyright notices."""

import unittest

from ament_copyright.main import main


class TestCopyright(unittest.TestCase):
    """Run the ament copyright linter."""

    def test_copyright(self) -> None:
        """Check all Python source files for a recognized license header."""
        rc = main(argv=['.', 'test'])
        self.assertEqual(rc, 0, 'Found errors')
