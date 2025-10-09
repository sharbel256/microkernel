find_package(nlohmann_json 3.2.0 REQUIRED)

add_library(kernel SHARED
    include/kernel/config_manager.hpp
    include/kernel/plugin_interface.hpp
    include/kernel/ring_buffer.hpp
    include/kernel/model.hpp
    include/kernel/kernel.hpp
    src/config_manager.cpp
    src/main.cpp
)

target_link_libraries(kernel PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(kernel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(microkernel src/main.cpp)
target_link_libraries(microkernel kernel)

install(TARGETS microkernel DESTINATION bin)
install(TARGETS kernel DESTINATION lib)
install(DIRECTORY include/kernel DESTINATION include)