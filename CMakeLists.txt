cmake_minimum_required(VERSION 3.10.0)
project(AmazingPrinterConverter VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Collect all source files
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Prefer modern target-based usage: include project's header dir for the target
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Find Boost (we use boost::system for asio's runtime components)
find_package(Boost REQUIRED COMPONENTS asio)
if(Boost_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE Boost::boost)
else()
    message(FATAL_ERROR "Boost not found. Please install Boost or ensure vcpkg is set up correctly.")
endif()


# Place executable in bin/
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)