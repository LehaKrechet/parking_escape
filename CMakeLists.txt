cmake_minimum_required(VERSION 3.10)
project(ParkingGame)

# Поиск библиотек SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Добавление исполняемого файла
add_executable(parking_game main_file.cpp)

# Линковка библиотек
target_link_libraries(parking_game 
    SDL2::SDL2 
    SDL2_image::SDL2_image 
    SDL2_ttf::SDL2_ttf
)

# Копирование DLL (для Windows)
if(WIN32)
    add_custom_command(TARGET parking_game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${SDL2_LIBRARY_DIR}/SDL2.dll"
        "${CMAKE_BINARY_DIR}/SDL2.dll"
    )
    add_custom_command(TARGET parking_game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${SDL2_IMAGE_LIBRARY_DIR}/SDL2_image.dll"
        "${CMAKE_BINARY_DIR}/SDL2_image.dll"
    )
    add_custom_command(TARGET parking_game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${SDL2_TTF_LIBRARY_DIR}/SDL2_ttf.dll"
        "${CMAKE_BINARY_DIR}/SDL2_ttf.dll"
    )
endif()