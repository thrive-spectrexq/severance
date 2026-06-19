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

# We only need headers for the API
set_target_properties(security_plugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/$<CONFIG>"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/$<CONFIG>"
)
