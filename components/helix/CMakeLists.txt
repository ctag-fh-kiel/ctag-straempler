file(GLOB SRC_FILES *.c)

idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS "./include"
        REQUIRES )

target_compile_definitions(${COMPONENT_LIB} PRIVATE MIPS)
target_compile_options(${COMPONENT_LIB} PRIVATE -mlongcalls)
