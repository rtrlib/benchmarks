include_directories(.)
add_subdirectory(src)

#Copy scripts in build tree
add_custom_target(
	copy_scripts ALL
	COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/*.sh ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/*.py ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Copy script files to build tree"
)