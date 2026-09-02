cmake_minimum_required(VERSION 3.21)
project(Experiment-Calculator-test VERSION 1.0.0 LANGUAGES C)

# Set C23 standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

find_package(OpenGL REQUIRED)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/CMakeLists.txt")
    message(FATAL_ERROR "Missing submodule not found. Run: git submodule update --init --recursive")
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

add_executable(Experiment-Calculator-test 
    src/main.c
    src/calc.c
    src/renderer.c
    src/input_handler.c
)

add_library(stb INTERFACE)

target_include_directories(Experiment-Calculator-test PRIVATE 
    external/glfw/include
    external/stb
    src    
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(Experiment-Calculator-test PRIVATE 
    glfw
    stb
    OpenGL::GL
)

if(PLATFORM_LINUX)
    target_link_libraries(Experiment-Calculator-test PRIVATE 
        m 
        dl 
        pthread
    )

    find_package(X11 REQUIRED)
    target_link_libraries(Experiment-Calculator-test PRIVATE 
        ${X11_LIBRARIES}
        ${X11_Xrandr_LIB}
        ${X11_Xinerama_LIB}
        ${X11_Xcursor_LIB}
        ${X11_Xi_LIB}
    )
elseif(PLATFORM_WINDOWS)
    target_compile_definitions(Experiment-Calculator-test PRIVATE
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )

    target_link_libraries(Experiment-Calculator-test PRIVATE
        opengl32
        gdi32
        shell32
    )
endif()

if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE MATCHES "Debug")
    if(NOT MSVC)
        target_compile_options(Experiment-Calculator-test PRIVATE 
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )

        target_link_options(Experiment-Calculator-test PRIVATE 
            -fsanitize=address,undefined
        )
    endif()
endif()

add_custom_command(TARGET Experiment-Calculator-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        $<TARGET_FILE_DIR:Experiment-Calculator-test>/assets
)