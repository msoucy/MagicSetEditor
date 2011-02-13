#!/usr/bin/perl

# For each mse-script file:
# 1. Invoke magicseteditor
# 2. Ensure that there are no errors

use strict;
use lib "../util/";
use MseTestUtils;
use TestFramework;

# -----------------------------------------------------------------------------
# The tests
# -----------------------------------------------------------------------------

test_case("script/Builtin Funcions", sub{
	run_script_test("test-builtin.mse-script", "");
});

test_case("script/Magic Funcions", sub{
	write_dummy_set("_dummy-magic-set.mse-set", "game: magic\nstylesheet: new\n");
	run_script_test("test-magic.mse-script", "_dummy-magic-set.mse-set");
	remove_dummy_set("_dummy-magic-set.mse-set");
});

1;
