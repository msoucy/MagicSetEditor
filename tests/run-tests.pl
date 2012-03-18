#+----------------------------------------------------------------------------+
#| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
#| Copyright:    (C) 2001 - 2011 Twan van Laarhoven and Sean Hunt             |
#| License:      GNU General Public License 2 or later (see file COPYING)     |
#+----------------------------------------------------------------------------+

use strict;

# -----------------------------------------------------------------------------
# Subdirectories
# -----------------------------------------------------------------------------

sub test_subdir {
	my $dir = shift;
	chdir($dir);
	require "../$dir/run-tests.pl";
}

test_subdir('script');
test_subdir('stylesheets');
test_subdir('export-templates');

