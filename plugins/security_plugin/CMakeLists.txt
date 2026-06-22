add_library(security_plugin SHARED
    SecurityPlugin.cpp
)

target_include_directories(security_plugin PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

# Export symbols
if(WIN32)
    set_target_properties(security_plugin PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

# Output directly to the plugins folder of the build directory
set_target_properties(security_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    PREFIX "" # No "lib" prefix
)
