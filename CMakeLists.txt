cmake_minimum_required(VERSION 3.22)
project(irradiancemapping LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options("/utf-8")
endif()

set(SOURCES
    main.cpp
    Window.cpp
    gg.cpp
)

set(HEADERS
    Window.h
    gg.h
)

set(SHADER_FILES
)

set(RESOURCE_FILES
    "bunny.mtl"
    "bunny.obj"
    "envmap0.tga"
    "envmap1.tga"
    "envmap2.tga"
    "envmap3.tga"
    "envmap4.tga"
    "envmap5.tga"
    "envmap6.tga"
    "envmap7.tga"
    "envmap8.tga"
    "envmap9.tga"
    "envmapa.tga"
    "envmapb.tga"
    "envmapc.tga"
    "envmapd.tga"
    "envmape.tga"
    "envmapf.tga"
    "envmapg.tga"
    "envmaph.tga"
    "irr00000.tga"
    "irrmap0.tga"
    "irrmap1.tga"
    "irrmap2.tga"
    "irrmap3.tga"
    "irrmap4.tga"
    "irrmap5.tga"
    "irrmap6.tga"
    "irrmap7.tga"
    "irrmap8.tga"
    "irrmap9.tga"
    "irrmapa.tga"
    "irrmapb.tga"
    "irrmapc.tga"
    "irrmapd.tga"
    "irrmape.tga"
    "irrmapf.tga"
    "irrmapg.tga"
    "irrmaph.tga"
    "skymap0.tga"
    "skymap1.tga"
    "skymap2.tga"
    "skymap3.tga"
    "skymap4.tga"
    "skymap5.tga"
    "skymap6.tga"
    "skymap7.tga"
    "skymap8.tga"
    "skymap9.tga"
)

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})
source_group("Resource Files" FILES ${RESOURCE_FILES})

set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")
set(INCLUDE_DIR "${FETCHCONTENT_BASE_DIR}/include")
set(GL_DIR "${INCLUDE_DIR}/GL")
set(KHR_DIR "${INCLUDE_DIR}/KHR")

file(MAKE_DIRECTORY "${INCLUDE_DIR}")
file(MAKE_DIRECTORY "${GL_DIR}")
file(MAKE_DIRECTORY "${KHR_DIR}")

if(NOT EXISTS "${GL_DIR}/glcorearb.h")
    message(STATUS "Downloading OpenGL & KHR headers...")
    set(UA_HEADER "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/glcorearb.h" "${GL_DIR}/glcorearb.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/glext.h" "${GL_DIR}/glext.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/wgl.h" "${GL_DIR}/wgl.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/wglext.h" "${GL_DIR}/wglext.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/glxext.h" "${GL_DIR}/glxext.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/EGL/api/KHR/khrplatform.h" "${KHR_DIR}/khrplatform.h" HTTPHEADER "${UA_HEADER}")
endif()

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

if(APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${SHADER_FILES} ${RESOURCE_FILES})
    find_library(OPENGL_LIBRARY OpenGL)
    target_include_directories(${PROJECT_NAME} PRIVATE "${INCLUDE_DIR}")
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw ${OPENGL_LIBRARY} "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2015-2026 Kohe Tokoi. All rights reserved."
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.yourcompany.${PROJECT_NAME}"
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    set_source_files_properties(${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY XCODE_GENERATE_SCHEME ON)

elseif(WIN32)
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADER_FILES} ${RESOURCE_FILES})
    find_package(OpenGL REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE
        "${INCLUDE_DIR}"
        "${glfw_SOURCE_DIR}/include"
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL)
    foreach(RESOURCE_FILE IN LISTS SHADER_FILES RESOURCE_FILES)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_FILE}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_FILE}"
                "${CMAKE_CURRENT_BINARY_DIR}"
        )
    endforeach()
    set_source_files_properties(${SHADER_FILES} ${RESOURCE_FILES} PROPERTIES
        HEADER_FILE_ONLY TRUE
        VS_COPY_TO_OUT_DIR PreserveNewest
    )
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

else()
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADER_FILES} ${RESOURCE_FILES})
    find_package(OpenGL REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE "${INCLUDE_DIR}")
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL m pthread dl)
    foreach(RESOURCE_FILE IN LISTS SHADER_FILES RESOURCE_FILES)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_FILE}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )
    endforeach()
endif()
