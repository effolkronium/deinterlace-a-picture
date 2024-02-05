project("deinterlace-a-picture")
add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)