cmake_minimum_required(VERSION 3.10)
project(ParkingGame)

set(CMAKE_CXX_STANDARD 17)

# Пути к библиотекам SDL2
set(SDL2_DIR "H:/SDL2")
set(SDL2_IMAGE_DIR "H:/SDL2_image")
set(SDL2_TTF_DIR "H:/SDL2_ttf")

# Поиск библиотек
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Добавление исполняемого файла
add_executable(ParkingGame main_file.cpp)

# Линковка библиотек
target_include_directories(ParkingGame PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

target_link_libraries(ParkingGame
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
)

# Копирование DLL в папку сборки
add_custom_command(TARGET ParkingGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${SDL2_DIR}/lib/x64/SDL2.dll"
    "${CMAKE_BINARY_DIR}/Debug/"
)