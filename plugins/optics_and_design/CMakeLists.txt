add_library(optics_and_design MODULE OpticsAndDesignPlugin.c)

set_target_properties(optics_and_design PROPERTIES
    LINKER_LANGUAGE C
    C_STANDARD 17
    C_STANDARD_REQUIRED ON
)

target_include_directories(optics_and_design PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

set_target_properties(optics_and_design PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    PREFIX "" 
)
