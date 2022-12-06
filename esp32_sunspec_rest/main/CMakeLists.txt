idf_component_register(SRCS "main copy.c" "main.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES "certs/servercert.pem"
                                   "certs/prvtkey.pem")
