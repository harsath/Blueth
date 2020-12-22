set(LIB_NAME libblueth)
add_subdirectory(./containers)
set(
	CONTAINER_DIR_FILES
	./containers/matrix.h
	)
add_library(
	${LIB_NAME} 
	STATIC ${CONTAINER_DIR_FILES}
	)
