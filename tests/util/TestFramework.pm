#+----------------------------------------------------------------------------+
#| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
#| Copyright:    (C) 2001 - 2011 Twan van Laarhoven and Sean Hunt             |
#| License:      GNU General Public License 2 or later (see file COPYING)     |
#+----------------------------------------------------------------------------+

package TestFramework;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(test_case fail_current_test); 

use strict;
use warnings;

# -----------------------------------------------------------------------------
# Test cases
# -----------------------------------------------------------------------------

# Initialization

$|=1;
my $tests_passed = 0;
my $tests_failed = 0;
our $current_test_passed;

# Evaluate a testcase, catch any exceptions
# Usage:  test_case("name",sub{code});
sub test_case {
	my $name = shift;
	my $sub  = shift;
	print "----\n";
	print "test: $name\n";
	$current_test_passed = 1;
	eval { &$sub(); };
	if ($@ || !$current_test_passed) {
		$tests_failed++;
		print $@;
		print "FAIL\n";
	} else {
		$tests_passed++;
		print "OK\n";
	}
}

sub fail_current_test {
	$current_test_passed = 0;
}

sub test_summary {
	print "====\n";
	print "tests passed: $tests_passed\n";
	print "tests failed: $tests_failed\n" if ($tests_failed);
	exit $tests_failed ? 1 : 0;
}
END { test_summary(); }

# -----------------------------------------------------------------------------
1;
