#!/bin/perl
use warnings;
use strict;
use IO::Socket::INET;
use Term::ANSIColor;

print "Running test for SyncClientNetworkStream from $0: ";

# Server configuration
use constant {
	Port => 9876,
	ListenHost => "localhost",
	Protocol => "tcp",
};

# Test configuration
use constant {
	ExpectedInput => "Hey, this is client",
	OutputToWrite => "Hello, it's from Perl"
};

my $serverSock = IO::Socket::INET->new(
	Listen	  => 1,
	LocalPort => Port,
	Proto	  => Protocol,
	ReuseAddr => 1
) or die "IO::Socket::INET->new() $!\n";

my $clientReader = $serverSock->accept;
my $inputBuffer;

while(1){
	my $bufferLength = sysread($clientReader, $inputBuffer, 100);
	die "sysread() failed: $!\n" unless defined $bufferLength;
	last;
}

chomp($inputBuffer);
if($inputBuffer eq ExpectedInput){
	print color("green"), "Passed", color("reset"), "\n";
	print $clientReader OutputToWrite;
}else{
	print color("red"), "Failed", color("reset"), "\n";
}
close $clientReader;
