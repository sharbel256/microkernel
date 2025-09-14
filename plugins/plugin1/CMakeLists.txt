add_library(plugin1 SHARED plugin1.cpp)
target_include_directories(plugin1 PUBLIC ${CMAKE_SOURCE_DIR}/kernel/include)

# Install plugin
install(TARGETS plugin1 DESTINATION lib/plugins)