#+----------------------------------------------------------------------------+
#| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
#| Copyright:    (C) 2001 - 2011 Twan van Laarhoven and Sean Hunt             |
#| License:      GNU General Public License 2 or later (see file COPYING)     |
#+----------------------------------------------------------------------------+

use strict;
use File::Basename;

# -----------------------------------------------------------------------------
# Invoking Magic Set Editor
# -----------------------------------------------------------------------------

# Find magicseteditor executable

our $MAGICSETEDITOR;
my $is_windows = $^O =~ /win/i || -e "C:";
if ($is_windows) {
	$MAGICSETEDITOR = "\"../../build/Release Unicode/mse.exe\"";
} else {
	$MAGICSETEDITOR = "../../magicseteditor";
}

# Invoke a script
sub run_script_test {
	my $script   = shift;
	my $args     = shift;
	my $outfile  = basename($script,".mse-script") . ".out";
	my $command  = "$MAGICSETEDITOR --cli --quiet --script $script $args > $outfile";
	print "$command\n";
	`$command`;
	
	# Check for errors / warnings
	open FILE,"< $outfile";
	my $in_error = 0;
	foreach (<FILE>) {
		if (/^(WARNING|ERROR)/) {
			print $_;
			$in_error = 1;
		} elsif ($in_error) {
			if (/^    /) {
				print $_;
			} else {
				$in_error = 0;
			}
		}
	}
	close FILE;
	
	# TODO: diff against expected output?
	#my $expected = basename($script,".mse-script") . ".out.expected";
}

# -----------------------------------------------------------------------------
# Invoking Magic Set Editor
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Dummy sets
# -----------------------------------------------------------------------------

sub file_set_contents {
	my $filename = shift;
	my $contents = shift;
	open FILE,"> $filename";
	print FILE $contents;
	close FILE;
}

sub write_dummy_set {
	my $setname = shift;
	my $contents = shift;
	mkdir($setname);
	file_set_contents("$setname/set", "mse version: 2.0.0\n$contents");
}

sub remove_dummy_set {
	my $setname = shift;
	unlink("$setname/set");
	rmdir($setname);
}

# -----------------------------------------------------------------------------
1;
