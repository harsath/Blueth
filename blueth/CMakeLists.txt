set(LIB_NAME libblueth)
set(
	CONTAINER_DIR_FILES
	./containers/matrix.hpp
	./containers/vector.hpp
	)
add_library(
	${LIB_NAME} 
	STATIC ${CONTAINER_DIR_FILES}
	)
set_target_properties(
	${LIB_NAME}
	PROPERTIES LINKER_LANGUAGE CXX
	)
