file(GLOB SRC_FILES *.c)

idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS "./include"
        REQUIRES vfs util json esp_http_server)