file(GLOB SOURCES *.c)

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS . ../../main
                       REQUIRES clock lepton sys json app_update mdns)

