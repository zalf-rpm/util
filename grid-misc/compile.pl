# /usr/bin/perl
#
# This script compiles the dbb-library.
# Use --install if make install should be called.
#

use strict;
use Getopt::Long;	# long command line parameters
use Cwd;			# determining pathname and current working directory
use Cwd 'abs_path';
use File::Path;		# removing a not empty directory

my $no_install = 0;

#############################################################

# run main routine
main();

##############################################################

#
# Main routine
#
sub main
{
	ParseCommandLineParameters();
		
	# run qmake
	print "-- qmake --\n";
	print `qmake grid.pro 2>&1`;
	CheckForErrors($?);
	
	# make
	print "-- make --\n";
	print `make 2>&1`;
	CheckForErrors($?);
	
	# make install
	if (! $no_install) {
		print "-- make install --\n";
		print `make install 2>&1`;
		CheckForErrors($?);
	}

	# run qmake
print "-- qmake --\n";
print `qmake grid-swig.pro 2>&1`;
CheckForErrors($?);
	
	# make
print "-- make --\n";
print `make 2>&1`;
CheckForErrors($?);
	
	# make install
if (! $no_install) {
	print "-- make install --\n";
	print `make install 2>&1`;
	CheckForErrors($?);
}
	
	# remove obj-directory
	print "-- Removing \"obj\" directory --\n";
	rmtree("obj");
}

#############################################################

#
# Checks command line parameters.
#
sub ParseCommandLineParameters
{
	my $result = GetOptions("no-install" => \$no_install);  # flag
}

#############################################################

sub CheckForErrors
{
	my $param = $_[0];	
	$param = $param>>8;
		
	if ($param !=0 ) {		
		print "\n-- Error $param: \"".abs_path($0)."\" is aborted --\n";
		exit 11;
	}	
}
