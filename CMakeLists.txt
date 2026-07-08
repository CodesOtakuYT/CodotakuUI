cmake_minimum_required(VERSION 4.2)
project(CodotakuUI)

set(CMAKE_CXX_STANDARD 26)

add_subdirectory(thirdparty/SDL EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/SDL_ttf EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/yoga EXCLUDE_FROM_ALL)

add_library(CodotakuUI STATIC src/codotaku_ui.cpp)
target_include_directories(CodotakuUI PUBLIC include)
target_link_libraries(CodotakuUI PUBLIC SDL3::SDL3 SDL3_ttf::SDL3_ttf yogacore)

if (PROJECT_IS_TOP_LEVEL)
    add_executable(CodotakuUI_demo examples/demo.cpp)
    target_link_libraries(CodotakuUI_demo PRIVATE CodotakuUI)

    add_custom_command(
            TARGET CodotakuUI_demo POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:CodotakuUI_demo> $<TARGET_RUNTIME_DLLS:CodotakuUI_demo>
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:CodotakuUI_demo>/assets"
            COMMAND_EXPAND_LISTS
    )
endif ()

