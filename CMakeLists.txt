cmake_minimum_required(VERSION 3.15)
project(CrowBackend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Essential Crow flags
add_definitions(-DCROW_USE_BOOST)

# 2. Map your local folders
# We include the root of your local boost and the include folder of crow
set(LOCAL_BOOST "${CMAKE_CURRENT_SOURCE_DIR}/boost_1_90_0")
set(LOCAL_CROW "${CMAKE_CURRENT_SOURCE_DIR}/crow/include")

# 3. Find System Dependencies (for linking)
find_package(Threads REQUIRED)

add_executable(CrowBackend main.cpp)

# 4. Tell the compiler where to find headers
target_include_directories(CrowBackend PUBLIC 
    ${LOCAL_BOOST} 
    ${LOCAL_CROW}
)

# 5. Tell the linker to join the actual library files
target_link_libraries(CrowBackend 
    PRIVATE 
    Threads::Threads
)