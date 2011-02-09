#!/usr/bin/perl

# For each mse-script file:
# 1. Invoke magicseteditor
# 2. Ensure that there are no errors

use strict;
use File::Find;
use File::Basename;
use lib "../util/";
use MseTestUtils;

# -----------------------------------------------------------------------------
# Utility functions
# -----------------------------------------------------------------------------

# Invoke a script
sub run_script_test {
	my $script   = shift;
	my $args     = shift;
	my $expected = basename($script,".mse-script") . ".out.expected";
	my $outfile  = basename($script,".mse-script") . ".out";
	my $command  = "$MseTestUtils::MAGICSETEDITOR --cli --quiet --script $script $args > $outfile";
	print "$command\n";
	`$command`;
	# TODO: diff against expected output
}

sub write_dummy_set {
	my $setname = shift;
	my $contents = shift;
	mkdir($setname);
	open SET,"> $setname/set";
	print SET "mse version: 2.0.0\n";
	print SET $contents;
	close SET;
}

# -----------------------------------------------------------------------------
# The tests
# -----------------------------------------------------------------------------

run_script_test("test-builtin1.mse-script", "");
write_dummy_set("dummy-magic-set.mse-set", "game: magic\nstylesheet: new\n");
run_script_test("test-magic.mse-script", "dummy-magic-set");
