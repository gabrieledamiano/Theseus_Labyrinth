cmake_minimum_required(VERSION 3.20)

project(Theseus LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(INCLUDE_DIR "${PROJECT_ROOT}/include")
set(LIB_DIR "${PROJECT_ROOT}/lib/x64/release")
set(GLFW_DIR "${PROJECT_ROOT}/../packages/glfw.3.3.3/build/native")
file(GLOB SHADER_FILES "${PROJECT_ROOT}/*.vs" "${PROJECT_ROOT}/*.fs")

add_executable(Theseus
    animation.cpp
    glad.c
    main.cpp
    particleEmitter.cpp
)

target_include_directories(Theseus PRIVATE
    "${INCLUDE_DIR}"
    "${GLFW_DIR}/include"
)

target_compile_definitions(Theseus PRIVATE GLM_ENABLE_EXPERIMENTAL)

target_link_libraries(Theseus PRIVATE
    "${LIB_DIR}/assimp-vc142-mt.lib"
    "${LIB_DIR}/freetype.lib"
    "${LIB_DIR}/glfw3.lib"
    "${LIB_DIR}/irrKlang.lib"
    opengl32
)

add_custom_command(TARGET Theseus POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIB_DIR}/assimp-vc142-mt.dll"
        "$<TARGET_FILE_DIR:Theseus>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIB_DIR}/glfw3.dll"
        "$<TARGET_FILE_DIR:Theseus>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_ROOT}/irrKlang.dll"
        "$<TARGET_FILE_DIR:Theseus>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_ROOT}/ikpMP3.dll"
        "$<TARGET_FILE_DIR:Theseus>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_ROOT}/ikpFlac.dll"
        "$<TARGET_FILE_DIR:Theseus>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_ROOT}/resources"
        "$<TARGET_FILE_DIR:Theseus>/resources"
)

foreach(SHADER_FILE IN LISTS SHADER_FILES)
    add_custom_command(TARGET Theseus POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SHADER_FILE}"
            "$<TARGET_FILE_DIR:Theseus>"
    )
endforeach()
