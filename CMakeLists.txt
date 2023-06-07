cmake_minimum_required(VERSION 3.22)

project(tp_final C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
# For MPI and OpenMP
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
# For MPI
find_package(MPI REQUIRED)
# Following line only if using OpenMPI
add_definitions(-DOMPI_SKIP_MPICXX)

add_executable(secuencial
    src/secuencial/main.c)

add_executable(MPI
    src/MPI/main.c)

add_executable(OpenMP
    src/OpenMP/main.c)

add_executable(Mixed
    src/Mixed/main.c)

# Linking MPI libraries
target_link_libraries(MPI PRIVATE
    ${MPI_C_LIBRARIES})
target_include_directories(MPI PRIVATE
    ${MPI_C_INCLUDE_PATH}) 

target_link_libraries(Mixed PRIVATE
    ${MPI_C_LIBRARIES})
target_include_directories(Mixed PRIVATE
    ${MPI_C_INCLUDE_PATH}) 

# add_library(c_procedures STATIC)
# target_include_directories(c_procedures PUBLIC
#     ${CMAKE_CURRENT_LIST_DIR})
