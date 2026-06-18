add_library(sample_plugin SHARED SamplePlugin.cpp)

# We need the headers from the main app to implement the interface
target_include_directories(sample_plugin PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/src/core 
    ${CMAKE_SOURCE_DIR}/include
)

# Output directly to the plugins folder of the build directory
set_target_properties(sample_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    PREFIX "" # No "lib" prefix
)
