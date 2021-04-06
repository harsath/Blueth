set(LIB_NAME libblueth)
set(
	CONTAINER_DIR_FILES
	containers/matrix.hpp
	containers/vector.hpp
	containers/stack_array.hpp
	)
set(
	IO_DIR_FILES
	io/IOBuffer.hpp
	)
set(
	NET_DIR_FILES
	net/Socket.hpp
	net/TransportHelpers.hpp
	net/NetworkStream.hpp
	)
set(
	UTILS
	utils/simd.hpp
	utils/UnixTime.hpp
	)
set(
	HTTP
	http/http_message.hpp
	http/http_headers.hpp
	http/http_constants.hpp
	)
set(
	CONCURRENCY
	concurrency/AsyncEventLoop.hpp
)
add_library(
	${LIB_NAME} 
	STATIC ${CONTAINER_DIR_FILES}
	${NET_DIR_FILES} 
	${IO_DIR_FILES}
	${CONCURRENCY}
	${UTILS}
	)
set_target_properties(
	${LIB_NAME}
	PROPERTIES LINKER_LANGUAGE CXX
	)
target_include_directories(
	${LIB_NAME}
	PUBLIC ./
)
