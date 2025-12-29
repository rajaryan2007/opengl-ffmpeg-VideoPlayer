cmake_minimum_required(VERSION 3.10)

project(video_rendering
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VCPKG_INSTALLED_DIR "C:/Users/rion/vcpkg/installed")
# Add libraries
add_subdirectory(../lib/GLFW ../build/GLFW)

# FFmpeg usually should NOT be added via add_subdirectory unless you built it with CMake



add_executable(video_rendering
    main.cc
    video_rendering.cc
    ../lib/glad/src/glad.c
)
target_include_directories(video_rendering PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/glad/include
    ${VCPKG_INSTALLED_DIR}/x64-windows/include
)
# Copy assets after build
add_custom_command(TARGET video_rendering POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${VCPKG_INSTALLED_DIR}/x64-windows/bin
    $<TARGET_FILE_DIR:video_rendering>
)

if (WIN32)
    target_link_libraries(video_rendering PRIVATE
    glfw
    opengl32
    ${VCPKG_INSTALLED_DIR}/x64-windows/lib/avcodec.lib
    ${VCPKG_INSTALLED_DIR}/x64-windows/lib/avformat.lib
    ${VCPKG_INSTALLED_DIR}/x64-windows/lib/avutil.lib
    ${VCPKG_INSTALLED_DIR}/x64-windows/lib/swscale.lib
    ${VCPKG_INSTALLED_DIR}/x64-windows/lib/swresample.lib
)

    set_target_properties(video_rendering PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()
