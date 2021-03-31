#!/bin/perl
use warnings;
use strict;
use constant {
	COMPILER_FLAGS => "-Wall -O2 -g",
	CC => "gcc-9",
	CXX => "g++-9",
	USE_CMAKE => 0
};

unless ($#ARGV+1 == 1){ 
	print "Usage: ./script.pl <BUILD_DIR>"; exit(1); 
}
my $BUILD_DIR = $ARGV[0];
my $TEST_BINS = {
	container => "./tests/test-containers/test_container",
	io => "./tests/test-io/test_io",
	http => "./tests/test-http/test_http"
};
if(-d $BUILD_DIR){
	print "Build dir already exists, remove that first\n"; exit(1);
}

sub build_binary {
	my $project_root = $_[0];
	unless($project_root){ print "You must provide project root\n"; exit(1); }
	mkdir($BUILD_DIR);
	chdir($BUILD_DIR);
	my $compiler_cmd = "CC=".CC." && CXX=".CXX." cmake -D CMAKE_CXX_FLAGS=\"".COMPILER_FLAGS."\"";
	if(!USE_CMAKE){ $compiler_cmd .= " -GNinja $project_root && ninja "; }
	else{ $compiler_cmd .= " $project_root && make"; }
	my $exit_code = system($compiler_cmd);
	return $exit_code;
}

sub run_tests {
	my $test_cmd = ${$TEST_BINS}{container}." && ".${$TEST_BINS}{io}." && ".${$TEST_BINS}{http};
	my $exit_code = system($test_cmd);
	return $exit_code;
}

my $build_status = build_binary("..");
if($build_status != 0){
	print "[Error] Build has failed\n";
	exit($build_status);
}
my $test_status = run_tests();
if($test_status != 0){
	print "[Error] Build has failed\n";
	exit($build_status);
}
