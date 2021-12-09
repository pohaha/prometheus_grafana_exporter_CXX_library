project(Prom_Exporter CXX)

add_library(${PROJECT_NAME} "src/prom_exporter.cpp")

list(APPEND EXTRA_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/includes")
target_link_libraries(Prom_Exporter curl)
target_include_directories(${PROJECT_NAME}
                    PUBLIC ${EXTRA_INCLUDE_DIRECTORIES}
                          )