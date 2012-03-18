#+----------------------------------------------------------------------------+
#| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
#| Copyright:    (C) 2001 - 2011 Twan van Laarhoven and Sean Hunt             |
#| License:      GNU General Public License 2 or later (see file COPYING)     |
#+----------------------------------------------------------------------------+

package MseTestUtils;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(run_script_test run_export_test file_set_contents write_dummy_set remove_dummy_set compare_files); 

use strict;
use File::Basename;
use TestFramework;

# -----------------------------------------------------------------------------
# Invoking Magic Set Editor
# -----------------------------------------------------------------------------

# Find magicseteditor executable

our $MAGICSETEDITOR;
my $is_windows = $^O =~ /win/i || -e "C:";
if ($is_windows) {
	$MAGICSETEDITOR = "\"../../build/Release Unicode/mse.com\"";
} else {
	$MAGICSETEDITOR = "../../magicseteditor";
}

# Invoke a script
sub run_script_test {
	my $script   = shift;
	my %opts     = @_;
	my $args     = defined($opts{set}) ? "\"$opts{set}\"" : '';
	my $ignore_locale_errors = $opts{ignore_locale_errors} // 1;
	my $cleanup  = $opts{cleanup} // 0;
	my $outfile  = basename($script,".mse-script") . ".out";
	my $errfile  = basename($script,".mse-script") . ".err";
	my $command  = "$MAGICSETEDITOR --cli --quiet --script \"$script\" $args > \"$outfile\" 2> \"$errfile\"";
	print "$command\n";
	#`$command`;
	my $errcode = system($command);
	if ($errcode != 0) {
		print "Invoking Magic Set Editor failed\n";
		fail_current_test();
	}
	
	# Check for errors / warnings
	check_for_errors($errfile, $ignore_locale_errors);
	
	# TODO: diff against expected output?
	#my $expected = basename($script,".mse-script") . ".out.expected";
	
	if ($cleanup) {
		unlink($outfile);
		unlink($errfile);
	}
}

# Invoke an export template
sub run_export_test {
	my $template = shift;
	my $set      = shift;
	my $outfile  = shift;
	my %opts     = @_;
	my $ignore_locale_errors = $opts{ignore_locale_errors} // 1;
	my $cleanup  = $opts{cleanup} // 0;
	my $errfile  = basename($set,".mse-set") . ".err";
	my $command  = "$MAGICSETEDITOR --export \"$template\" \"$set\" \"$outfile\" 2> \"$errfile\"";
	print "$command\n";
	my $errcode = system($command);
	if ($errcode != 0) {
		print "Invoking Magic Set Editor failed\n";
		fail_current_test();
	}
	
	# Check for errors / warnings
	check_for_errors($errfile, $ignore_locale_errors);

	# TODO: diff against expected output?
	#my $expected = basename($script,".mse-script") . ".out.expected";
	
	if ($cleanup) {
		unlink($errfile);
	}
}

sub check_for_errors {
	my $errfile = shift;
	my $ignore_locale_errors = shift;
	open FILE,"< $errfile";
	my $in_error = 0;
	foreach (<FILE>) {
		if (/^(WARNING|ERROR)/) {
			print $_;
			$in_error = 1;
			fail_current_test() unless (/in locale file/ && $ignore_locale_errors);
		} elsif ($in_error) {
			if (/^    /) {
				print $_;
			} else {
				$in_error = 0;
			}
		}
	}
	close FILE;
}

# -----------------------------------------------------------------------------
# Comparing files
# -----------------------------------------------------------------------------

sub compare_files {
	my $out_file = shift;
	my $expected_file = shift;
	if (!-f $out_file) {
		die("Output file missing: $out_file");
	}
	if (!-f $expected_file) {
		die("File with expected output is missing: $expected_file");
	}
	if (system("diff -q \"$out_file\" \"$expected_file\"") != 0) {
		die("Files differ: $out_file $expected_file");
	}
}

sub compare_image_files {
}

# -----------------------------------------------------------------------------
# Information on the machine running tests
# -----------------------------------------------------------------------------

use POSIX qw/strftime/;
use Sys::Hostname;

sub svn_revision {
	my $rev = `svn info ../..`;
	if ($rev =~ /Revision:\s*(\d+)/) {
		return $1;
	} else {
		return "???";
	}
}

sub print_system_info {
	print "host: ", getlogin()," @ ",hostname, "\n";
	print "platform: ",($is_windows ? "Windows" : "not-windows"),"\n";
	print "date: ", strftime('%Y-%m-%d %H:%M:%S (%z)',localtime), "\n";
	print "revision: ", svn_revision(), "\n";
}

print_system_info();

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
