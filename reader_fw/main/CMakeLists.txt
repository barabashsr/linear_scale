idf_component_register(
    SRCS "main.c" "scale.c" "buttons.c" "i2c_slave.c"
    INCLUDE_DIRS "." "../../shared"
    REQUIRES driver freertos esp_timer
)
