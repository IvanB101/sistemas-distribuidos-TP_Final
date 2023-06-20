cmake_minimum_required(VERSION 3.22)

project(tp_final C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
# For MPI and OpenMP
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp -mcmodel=large")
# For MPI
find_package(MPI REQUIRED)
# Following line only if using OpenMPI
add_definitions(-DOMPI_SKIP_MPICXX)

add_subdirectory(src/common)

add_executable(test
    src/test/main.c)
target_link_libraries(test
    common)

add_executable(secuencial
    src/secuencial/main.c)
target_link_libraries(secuencial
    common)

add_executable(mpi
    src/mpi/main.c)
target_link_libraries(mpi PRIVATE
    common)

add_executable(openmp
    src/openmp/main.c)
target_link_libraries(openmp 
    common)

add_executable(mixed 
    src/mixed/main.c)
target_link_libraries(mixed PRIVATE
    common)

# Linking MPI libraries
target_link_libraries(mpi PRIVATE
    ${MPI_C_LIBRARIES})
target_include_directories(mpi PRIVATE
    ${MPI_C_INCLUDE_PATH}) 

target_link_libraries(mixed PRIVATE
    ${MPI_C_LIBRARIES})
target_include_directories(mixed PRIVATE
    ${MPI_C_INCLUDE_PATH}) 

