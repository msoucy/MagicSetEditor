#!/usr/bin/perl

# For each export-template:
# 1. Create a dummy set for its game, with the first stylesheet we encounter (?)
# 2. Invoke magicseteditor with that set & export-template
# 3. Ensure that there are no errors

use strict;
use lib "../util/";
use MseTestUtils;
use TestFramework;
use File::Spec;
use File::Basename;

# -----------------------------------------------------------------------------
# The tests
# -----------------------------------------------------------------------------

my $package_dir = "../../data";
our %default_stylesheets = ('magic' => 'new');
my @packages = glob "$package_dir/*.mse-export-template";

sub test_export_template {
	my $path = shift;
	(my $x,my $y,my $package) = File::Spec->splitpath($path);
	my $basename = basename($package,".mse-export-template");
	
	test_case("export-template/$basename",sub{
	
	# Determine game for this export-template
	my $game;
	open TEMPLATE, "< $path/export-template";
	while (<TEMPLATE>) {
		$game = $1 if /^game:\s*(\S*)/;
	}
	close TEMPLATE;
	die ("No game found for $package") if !$game;
	
	# Check filename
	my $suffix;
	if ($package =~ /$game-(.+).mse-export-template$/) {
		$suffix = $1;
	} else {
		die("Export-template filename doesn't match game ($game)!\n");
	}
	
	# Find stylesheet
	my $stylesheet = $default_stylesheets{$game} // 'standard';
	
	print "game: $game\n";
	print "stylesheet: $stylesheet\n";
	print "export-template: $suffix\n";
	
	# Create dummy set
	my $set;
	my $tempname = "_dummy-$basename";
	my $setname = "$tempname-set.mse-set";
	$set .= "game: $game\n";
	$set .= "stylesheet: $stylesheet\n";
	$set .= "card:\n";
	write_dummy_set($setname, $set);
	
	# Run!
	mkdir("out");
	run_export_test($package, $setname, "out/$basename.out", cleanup => 1);
	
	# Cleanup
	remove_dummy_set($setname);
	
	# TODO: Compare the output against the expected output?
	
	});
}

foreach (@packages) {
	test_export_template($_) ;
}

1;
