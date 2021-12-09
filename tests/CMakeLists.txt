project(tests CXX)

# Setup testing
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

add_executable(${PROJECT_NAME} main.cpp)

list(APPEND EXTRA_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/../lib/includes")
list(APPEND EXTRA_LINKED_SOURCES Prom_Exporter)

target_include_directories(${PROJECT_NAME}
                    PUBLIC ${EXTRA_INCLUDE_DIRECTORIES}
                          )

target_link_libraries(${PROJECT_NAME} 
                      gtest 
                      gtest_main
                      ${EXTRA_LINKED_SOURCES}
                      )