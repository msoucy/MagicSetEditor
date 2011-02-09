#+----------------------------------------------------------------------------+
#| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
#| Copyright:    (C) 2001 - 2011 Twan van Laarhoven and Sean Hunt             |
#| License:      GNU General Public License 2 or later (see file COPYING)     |
#+----------------------------------------------------------------------------+

package MseTestUtils;

use strict;

# -----------------------------------------------------------------------------
# Utilities for testing scripts
# -----------------------------------------------------------------------------

# Find magicseteditor executable

our $MAGICSETEDITOR;
my $is_windows = $^O =~ /win/i;
if ($is_windows) {
	$MAGICSETEDITOR = "\"../../build/Release Unicode/mse.exe\"";
} else {
	$MAGICSETEDITOR = "../../magicseteditor";
}


# -----------------------------------------------------------------------------
1;
