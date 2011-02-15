#!/usr/bin/perl

# For each stylesheet:
# 1. Create a dummy set for that stylesheet
# 2. Invoke magicseteditor with that set
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

sub test_stylesheet {
	my $path = shift;
	(my $x,my $y,my $package) = File::Spec->splitpath($path);
	my $basename = basename($package,".mse-style");
	
	test_case("stylesheets/$basename/Blank card",sub{
	
	# Determine game for this set
	my $game;
	open STYLE, "< $path/style";
	while (<STYLE>) {
		$game = $1 if /^game:\s*(\S*)/;
	}
	close STYLE;
	die ("No game found for $package") if !$game;
	
	# Stylesheet suffix
	my $suffix;
	if ($package =~ /$game-(.+).mse-style$/) {
		$suffix = $1;
	} else {
		die ("Stylesheet filename doesn't match game ($game)!\n");
	}
	
	print "game: $game\n";
	print "stylesheet: $suffix\n";
	
	# Create dummy set
	my $set;
	my $tempname = "_dummy-$basename";
	my $setname = "$tempname-set.mse-set";
	$set .= "game: $game\n";
	$set .= "stylesheet: $suffix\n";
	$set .= "card:\n";
	write_dummy_set($setname, $set);
	
	# Write script
	my $script = "$tempname.mse-script";
	mkdir("cards-out");
	file_set_contents($script, "write_image_file(set.cards[0],file:\"cards-out/blank-$basename.png\");1");
	
	# Run!
	run_script_test($script, set => $setname, cleanup => 1);
	
	# Cleanup
	remove_dummy_set($setname);
	unlink($script);
	
	# TODO: Compare the card against the expected output?
	
	});
}

my $package_dir = "../../data";
my @packages = glob "$package_dir/*.mse-style";

foreach (@packages) {
	test_stylesheet($_);
}

1;
