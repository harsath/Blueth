#include "concurrency/internal/EventLoopBase.hpp"
#include "net/Socket.hpp"
#include <asm-generic/socket.h>
#include <concurrency/AsyncEventLoop.hpp>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace blueth;
class PeerStateImpl {
	char *buffer;
	size_t buffer_len;
	char *cursor;
};

static concurrency::FDStatus on_read(concurrency::PeerStateHolderBase<PeerStateImpl> *peer_state){
	printf("Ready read\n");
	exit(1);
	return concurrency::WantNoReadWrite;
}

static concurrency::FDStatus on_write(concurrency::PeerStateHolderBase<PeerStateImpl> *peer_state){
	printf("Ready write\n");
	exit(1);
	return concurrency::WantRead;
}

static concurrency::FDStatus on_accept(concurrency::PeerStateHolderBase<PeerStateImpl> *peer_state){
	printf("accept connection\n");
	return concurrency::WantWrite;
}

int main(int argc, const char *argv[]){
	int sock_fd = ::socket(AF_INET, SOCK_STREAM, 0);
	short port = 9999;
	if(sock_fd < 0){
		std::perror("socket");
		::exit(EXIT_FAILURE);
	}
	int opt = 1;
	if(::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		std::perror("setsockopt");
		::exit(EXIT_FAILURE);
	}
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);
	if(::bind(sock_fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		std::perror("bind");
		::exit(EXIT_FAILURE);
	}
	if(::listen(sock_fd, 10)){
		std::perror("listen");
		::exit(EXIT_FAILURE);
	}
	net::makeSocketNonBlocking(sock_fd);

	// concurrency::EventLoopBase<PeerStateImpl>* evl = new concurrency::AsyncEpollEventLoop<PeerStateImpl>(sock_fd, 1000);
	// evl->registerCallbackForEvent(on_accept, concurrency::EventType::AcceptEvent);
	// evl->registerCallbackForEvent(on_write, concurrency::EventType::WriteEvent);
	// evl->registerCallbackForEvent(on_read, concurrency::EventType::ReadEvent);
	// evl->startEventloop();
	
	return 0;
}
