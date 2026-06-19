# Sample C Plugin — Pure C implementation using the Severance C Plugin SDK

add_library(sample_c_plugin SHARED SampleCPlugin.c)

# Force C compilation (not C++)
set_target_properties(sample_c_plugin PROPERTIES
    LINKER_LANGUAGE C
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
)

# Plugin only needs the public SDK header
target_include_directories(sample_c_plugin PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Output to the plugins folder of the build directory
set_target_properties(sample_c_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    PREFIX "" # No "lib" prefix
)
