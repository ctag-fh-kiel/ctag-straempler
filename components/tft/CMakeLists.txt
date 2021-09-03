file(GLOB SRC_FILES
        ESP32_TFT_library/components/tft/*.c
        ESP32_TFT_library/components/spidriver/*.c
        )

idf_component_register(SRCS ${SRC_FILES}
        INCLUDE_DIRS ESP32_TFT_library/components/tft ESP32_TFT_library/components/spidriver
        REQUIRES )