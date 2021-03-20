set(LIB_NAME libblueth)
set(
	CONTAINER_DIR_FILES
	./containers/matrix.hpp
	./containers/vector.hpp
	./containers/stack_array.hpp
	)
set(
	IO_DIR_FILES
	./io/IOBuffer.hpp
)
set(
	NET_DIR_FILES
	./net/Socket.hpp
	./net/TransportHelpers.hpp
	)
set(
	OTHER_UTIL
	./simd.hpp
	)
add_library(
	${LIB_NAME} 
	STATIC ${CONTAINER_DIR_FILES}
	${NET_DIR_FILES} 
	${IO_DIR_FILES}
	${OTHER_UTIL}
	)
set_target_properties(
	${LIB_NAME}
	PROPERTIES LINKER_LANGUAGE CXX
	)
