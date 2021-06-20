#!/usr/bin/env perl
use warnings;
use strict;

my $server_cert_domain = "localtester.net";
my $server_cert_name = "$server_cert_domain.crt";
my $server_csr_name = "$server_cert_domain.csr";
my $server_key_name = "$server_cert_domain.key";
my $key_size = 4096;

system "openssl genrsa -out $server_key_name $key_size";
system "openssl req -subj '/C=CA/ST=Toronto/L=Toronto/O=LOL' -new -key $server_key_name -out $server_csr_name";
system "openssl genrsa -out ca.key $key_size";
system "openssl req -subj '/C=CA/ST=Toronto/L=Toronto/O=LOL' -new -x509 -key ca.key -out ca.crt";
system "openssl x509 -req -in $server_csr_name -CA ca.crt -CAkey ca.key -CAcreateserial -out $server_cert_name";
