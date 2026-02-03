idf_component_register(SRCS "param_validation.c" "nv_params.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash config)