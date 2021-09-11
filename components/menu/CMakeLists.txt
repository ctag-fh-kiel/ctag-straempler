file(GLOB SRC_FILES *.c)

idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS "./include"
        REQUIRES json util tft audio rest-api)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-function -Wno-implicit-fallthrough -Wno-unused-but-set-variable -Wno-unused-variable)

