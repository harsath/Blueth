#pragma once
#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace blueth::net {
inline void err_check(int returner, const std::string &err_str) {
	if (returner < 0) {
		perror(err_str.c_str());
		exit(EXIT_FAILURE);
	}
}
inline void read_data(int net_fd, char *read_buffer,
		      std::size_t read_buffer_size, int recv_flag) {
	int recv_ret = recv(net_fd, read_buffer, read_buffer_size, recv_flag);
	err_check(recv_ret, "linux recv()");
}

inline void write_data(int new_fd, const char *buffer, std::size_t buffer_size,
		       int send_flag) {
	int send_ret = send(new_fd, buffer, buffer_size, send_flag);
	err_check(send_ret, "linux send()");
}

inline void close_connection(int client_fd) {
	int close_ret = ::close(client_fd);
	err_check(close_ret, "linux close()");
}
} // namespace blueth::net
