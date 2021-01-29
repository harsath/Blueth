#!/bin/php
<?php

if($argc != 2){ echo "Usage: ./script.php <BUILD DIR NAME> "; exit(1); }

const COMPILER_FLAGS = "-Wall -O2 -g";
const CC = "gcc-9";
const CXX = "g++-9";
const USE_CMAKE = false;
$BUILD_DIR = $argv[1];
const TEST_BINS = array(
	"container" => "./tests/test-containers/test_container",
	"io" => "./tests/test-io/test_io"
);

if(is_dir($BUILD_DIR)){ echo "Build dir already exists, remove first"; exit(1); }

function build_binary(string $project_root) : int {
	global $BUILD_DIR;
	mkdir($BUILD_DIR);
	chdir($BUILD_DIR);
	$compiler_cmd = "CC=".CC." && CXX=".CXX." cmake -D CMAKE_CXX_FLAGS=\"".
			COMPILER_FLAGS."\"";	
	if(!USE_CMAKE){ $compiler_cmd .= " -GNinja ".$project_root." && ninja "; }
	else{ $compiler_cmd .= " ".$project_root." && make"; }
	system($compiler_cmd, $build_status);	
	return $build_status;
}
function run_tests() : int {
	$test_cmd = TEST_BINS["container"]." && ".TEST_BINS["io"];
	system($test_cmd, $test_status);		
	return $test_status;
}

$build_status = build_binary("..");
if($build_status != 0){ echo "[Error] Build has failed"; exit($build_status); }

$test_status = run_tests();
if($test_status != 0){ echo "[Error] Test has failed"; exit($test_status); }

?>
