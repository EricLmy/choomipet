idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES "hardware" "audio" "display" "input" "network" "system" "common")