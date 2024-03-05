idf_component_register(SRCS "ufs.cpp"
                    PRIV_REQUIRES nvs_flash 
                        vfs
                        fatfs
                        spiffs
                    INCLUDE_DIRS ".")
