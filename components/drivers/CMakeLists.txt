file(GLOB SRC_FILES *.c)

idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS "./include"
        REQUIRES ulp)

set(ulp_app_name ulp_${COMPONENT_NAME})

set(ulp_s_sources ulp/stack.S ulp/spi-bb.S)

#set(ulp_exp_dep_srcs "adc.cpp")

ulp_embed_binary(${ulp_app_name} ${ulp_s_sources} ${ulp_exp_dep_srcs})



