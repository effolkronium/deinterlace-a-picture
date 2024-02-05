cmake_minimum_required(VERSION 3.12) # FindJPEG
project("deinterlace-a-picture")
find_package(JPEG REQUIRED)
add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)
target_link_libraries(${PROJECT_NAME} JPEG::JPEG)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
