#pragma once
#include "NetworkStream.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

namespace blueth::net {

static std::string
DnsResolveIPv4FromName(std::string dest_ip,
		       StreamProtocol protocol) noexcept(false) {
	addrinfo hints, *results, *temp;
	char ipv4_buffer[INET_ADDRSTRLEN] = {0};
	hints.ai_family = AF_UNSPEC;
	if (protocol == StreamProtocol::TCP) {
		hints.ai_socktype = SOCK_STREAM;
	} else if (protocol == StreamProtocol::UDP) {
		hints.ai_socktype = SOCK_DGRAM;
	}
	int status;
	if ((status = ::getaddrinfo(dest_ip.c_str(), nullptr, &hints,
				    &results)) != 0) {
		std::perror("getaddrinfo");
		throw std::runtime_error{
		    "Error from getaddrinfo during name resolution"};
	}
	temp = results;
	for (; temp != nullptr; temp = temp->ai_next)
		if (temp->ai_family == AF_INET) {
			sockaddr_in *ipv4 =
			    reinterpret_cast<sockaddr_in *>(temp->ai_addr);
			::inet_ntop(temp->ai_family, &ipv4->sin_addr,
				    ipv4_buffer, INET_ADDRSTRLEN);
		}
	freeaddrinfo(results);
	return ipv4_buffer;
}

} // namespace blueth::net
