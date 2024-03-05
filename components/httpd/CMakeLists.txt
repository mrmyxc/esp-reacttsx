idf_component_register(SRCS "httpd.cpp"
                    PRIV_REQUIRES nvs_flash
                        esp_http_server
                        vfs
                        json
                    INCLUDE_DIRS ".")
