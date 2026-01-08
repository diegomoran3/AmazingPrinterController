cmake_minimum_required(VERSION 3.10.0)
project(AmazingPrinterConverter VERSION 0.1.0 LANGUAGES C CXX)

# 1. Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 2. Find Dependencies (MUST be done before using them)
# Find Boost
find_package(Boost REQUIRED COMPONENTS asio system)

# Find wxWidgets (Required for the log window)
find_package(wxWidgets REQUIRED COMPONENTS core base)
# This includes the "hidden" setup.h path you were missing earlier
include(${wxWidgets_USE_FILE})

# 3. Collect Source Files
# Ensure your main.cpp (with the log code) is inside the src/ folder
# Include both .cpp AND .h files so VS Code sees them as part of the project
file(GLOB SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/include/*.h"
)

# 4. Create Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# 5. Configure Includes
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

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