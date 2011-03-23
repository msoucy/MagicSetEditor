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

test_case("script/Locale Errors?", sub{
	run_script_test("test-blank.mse-script", ignore_locale_errors => 0);
	compare_files("test-blank.out", "expected-out/test-blank.out");
});

test_case("script/Builtin Funcions", sub{
	run_script_test("test-builtin.mse-script");
	compare_files("test-builtin.out", "expected-out/test-builtin.out");
	compare_files("textfile1.out.txt", "expected-out/textfile1.out.txt");
});

test_case("script/Magic Funcions", sub{
	write_dummy_set("_dummy-magic-set.mse-set", "game: magic\nstylesheet: new\n");
	run_script_test("test-magic.mse-script", set => "_dummy-magic-set.mse-set");
	remove_dummy_set("_dummy-magic-set.mse-set");
	compare_files("test-magic.out", "expected-out/test-magic.out");
});

test_case("compatability/2.0.0", sub{
	mkdir("out");
	run_export_test("magic-forum", "simple-magic-2.0.0.mse-set", "out/simple-magic-2.0.0.txt", cleanup => 1);
	compare_files("out/simple-magic-2.0.0.txt", "expected-out/simple-magic-2.0.0.txt");
});

1;
