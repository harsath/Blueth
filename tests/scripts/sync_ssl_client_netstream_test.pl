#!/usr/bin/env perl
use warnings;
use strict;
use IO::Socket::SSL;

use constant {
	SSL_CERT => "cert.pem",
	SSL_KEY => "key.pem"
};

my $ssl_key_location = "/tmp/key.pem";
my $ssl_cert_location = "/tmp/cert.pem";

`openssl req -nodes -newkey rsa:2048 -keyout $ssl_key_location -out $ssl_cert_locatio -subj "/C=DE/ST=NRW/L=Berlin/O=Foo/OU=Foo/CN=www.foo.com/emailAddress=foo\@www.foo.com"`;

my $io_class = IO::Socket::SSL->can_ipv6 || 'IO::Socket::INET';
my $ssl_server = $io_class->new(
	Listen => 3,
	LocalAddr => '127.0.0.1',
	LocalPort => 9876,
	Reuse => 1,
) or die "io_class->new() failed.";

my $ssl_ctx = IO::Socket::SSL::SSL_Context->new(
	SSL_Server => 1,
	SSL_cert_file => SSL_CERT,
	SSL_key_file => SSL_KEY
) or die "cannot create SSL Context";

my $response = "Hello, from Perl";
my $request;
my $expected_request = "Hello, from C++ Tester";
while(1){
	my $cl = $ssl_server->accept or do {
		warn "failed to accept\n";
		next;
	};
	IO::Socket::SSL->start_SSL($cl, SSL_server => 1, SSL_reuse_ctx => $ssl_ctx) or do {
		warn "ssl handshake failed\n";
		next;
	};
	$request = <$cl>;
	unless($request eq $expected_request){
		print "Unmatched, falied\n";
		last;
	}
	print $cl $response;
	last;
}

END {
	`rm key.pem cert.pem`;
}
