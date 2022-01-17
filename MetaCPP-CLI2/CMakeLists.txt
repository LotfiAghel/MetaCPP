cmake_minimum_required (VERSION 3.1)
project(MetaCPP-CLI2)

set (CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../CMake")

add_subdirectory(../MetaCPP MetaCPP)

find_package(LLVM REQUIRED)
find_Package(Clang REQUIRED)

# LLVM
link_directories(${LLVM_LIB_DIR})
add_definitions(${LLVM_COMPILE_FLAGS})

set(MetaCPPCLI_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE MetaCPPCLI_SRC ${MetaCPPCLI_SRC_DIR}/*.hpp
								 ${MetaCPPCLI_SRC_DIR}/*.cpp)

file(GLOB_RECURSE MetaCPPCLI_SRC2 ${MetaCPPCLI_SRC_DIR}/JsonS/*.hpp
                                                              ${MetaCPPCLI_SRC_DIR}/JsonS/*.cpp)
set(APPEND MetaCPPCLI_SRC ${MetaCPPCLI_SRC2})
add_executable(MetaCPPCLI2 ${MetaCPPCLI_SRC})

target_link_libraries(MetaCPPCLI2 PUBLIC MetaCPP)

# LLVM and Clang
target_link_libraries(MetaCPPCLI2 PUBLIC ${CLANG_LIBS} ${LLVM_LIBS_CORE} ${LLVM_LDFLAGS})
target_include_directories(MetaCPPCLI2 PRIVATE ${LLVM_INCLUDE_DIR} ${CLANG_INCLUDE_DIRS})
target_include_directories(MetaCPPCLI2 PUBLIC ../External/Mustache)
if(WIN32)
	# Annoying lib in Windows
	target_link_libraries(MetaCPPCLI2 PRIVATE version.lib)
endif()
