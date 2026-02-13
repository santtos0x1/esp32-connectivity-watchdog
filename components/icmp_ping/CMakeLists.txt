idf_component_register(SRCS "net_ping.c"
                    INCLUDE_DIRS "include"
                    REQUIRES lwip icmp_ping)