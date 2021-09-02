set(SRC_FILES main.c)

idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS "."
        REQUIRES)

