#!/bin/perl
use warnings;
use strict;

my @apt_dependencies = ("cmake", "ninja-build", "gcc-9", "g++-9");
`sudo apt-get update -y`;
`sudo add-apt-repository ppa:ubuntu-toolchain-r/test`;
my $apt_cmd = "sudo apt-get install ";
for (@apt_dependencies){
	$apt_cmd .= "$_ ";	
}
$apt_cmd .= " -y";
system $apt_cmd;

`cpan install IO::Socket`;
`cpan install IO::Socket::INET`;
`cpan install Term::ANSIColor`;

print "\n\n#### CMake Version: ####\n\n";
system "cmake --version";
print "#### GCC Version: ####\n\n";
system "gcc --version";
