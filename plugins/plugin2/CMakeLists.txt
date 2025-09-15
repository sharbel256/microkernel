add_library(plugin2 SHARED plugin2.cpp)
target_include_directories(plugin2 PUBLIC ${CMAKE_SOURCE_DIR}/kernel/include)

install(TARGETS plugin2 DESTINATION lib/plugins)