# Kernel library
add_library(kernel SHARED
    src/main.cpp
    include/kernel/PluginInterface.hpp
)

target_include_directories(kernel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Kernel executable
add_executable(microkernel src/main.cpp)
target_link_libraries(microkernel kernel)

# Install target for kernel
install(TARGETS microkernel DESTINATION bin)
install(TARGETS kernel DESTINATION lib)
install(DIRECTORY include/kernel DESTINATION include)