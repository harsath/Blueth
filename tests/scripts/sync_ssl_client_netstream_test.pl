#!/usr/bin/env perl
use warnings;
use strict;
use IO::Socket::SSL;

my $ssl_key_location = "key.pem";
my $ssl_cert_location = "cert.pem";

my $io_class = IO::Socket::SSL->can_ipv6 || 'IO::Socket::INET';
my $ssl_server = $io_class->new(
	Listen => 3,
	LocalAddr => '127.0.0.1',
	LocalPort => 9876,
	Reuse => 1,
) or die "io_class->new() failed";

my $ssl_ctx = IO::Socket::SSL::SSL_Context->new(
	SSL_Server => 1,
	SSL_cert_file => $ssl_cert_location,
	SSL_key_file => $ssl_key_location
) or die "cannot create SSL Context: $SSL_ERROR";

my $response = "Hello, from Perl";
my $request;
my $expected_request = "Hello, from C++ Tester";
while(1){
	my $cl = $ssl_server->accept or do {
		warn "failed to accept ".$SSL_ERROR if defined $SSL_ERROR."\n";
		next;
	};
	IO::Socket::SSL->start_SSL($cl, SSL_server => 1, SSL_reuse_ctx => $ssl_ctx) or do {
		warn "ssl handshake failed\n";
		next;
	};
	chomp($request = <$cl>);
	unless($request eq $expected_request){
		print "Unmatched, failed\n";
		last;
	}
	print $cl $response;
	last;
}
