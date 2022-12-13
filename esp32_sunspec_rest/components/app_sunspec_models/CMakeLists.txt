idf_component_register(SRCS "model_1.c" 
                            "model_307.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES sunspec_models sht31_reader bmp280_reader)