cmake_minimum_required(VERSION 3.12.4)

# *******************************************
# ************* CMake Content ***************
# *******************************************
# This CMake create a workspace containing the following projects
# 
# Programs
#  - flappy-bird

set (PROJECT_NAME flappy-bird)

project(${PROJECT_NAME})

# Disable C and C++ compiler extensions.
# C/CXX_EXTENSIONS are ON by default to allow the compilers to use extended
# variants of the C/CXX language.
# However, this could expose cross-platform bugs in user code or in the headers
# of third-party dependencies and thus it is strongly suggested to turn
# extensions off.
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
#set(NO_CONSOLE_CONTROL defined)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT ${CMAKE_GENERATOR} MATCHES "Visual Studio.*")

	# Link with pthread
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

	#add libmath during non visual studio builds
	set(CMAKE_EXTRA_LIB m)
else()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Add definitions for testing purposes
if(${TESTING})
	MESSAGE("Testing mode")
	add_definitions(-DNO_CONSOLE_CONTROL -DNB_GENERATIONS=10)
endif()


# *******************************************
# *********** GEGELATI LIBRARY **************
# *******************************************

if(WIN32)
	set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
    # find the gegelatilib-x.y.z folder in the lib directory.
	file(GLOB GEGELATI_ROOT_DIR "${LIBS_DIR}/gegelatilib-[\\.|0-9]*")
	set(ENV{GEGELATI_DIR} ${GEGELATI_ROOT_DIR})
endif()
find_package(GEGELATI)


if (WIN32)
	file(GLOB
		GEGELATI_DLL
		${GEGELATI_ROOT_DIR}/bin/*.dll
	)

	MESSAGE("Copy GEGELATI DLLs into ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
	if(NOT ${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
		file(COPY ${GEGELATI_DLL} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	else()
		file(COPY ${GEGELATI_DLL} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
		file(COPY ${GEGELATI_DLL} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
	endif()
endif()


# *******************************************
# ************** Executable  ****************
# *******************************************
file(GLOB_RECURSE
	flappy-bird_files
	#./src/*.cpp
	./src/*.h
	./params.json
)
message("include dir : " ${GEGELATI_INCLUDE_DIRS})
include_directories(${GEGELATI_INCLUDE_DIRS})
#SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -coverage")
add_compile_options(-fno-elide-constructors)

#add_executable(${PROJECT_NAME} ${flappy-bird_files})
#target_link_libraries(${PROJECT_NAME} ${GEGELATI_LIBRARIES} sfml-graphics sfml-window sfml-system sfml-audio)
#target_compile_definitions(${PROJECT_NAME} PRIVATE ROOT_DIR="${CMAKE_SOURCE_DIR}")

add_executable(flappy-bird_playable src/Flappy_playable.cpp)
target_link_libraries(flappy-bird_playable  ${GEGELATI_LIBRARIES} sfml-graphics sfml-window sfml-system sfml-audio)
target_compile_definitions(flappy-bird_playable PRIVATE MNIST_DATA_LOCATION="${MNIST_DATA_DIR}" ROOT_DIR="${CMAKE_SOURCE_DIR}")


add_executable(flappy-bird_TPG src/main.cpp src/Flappy_bird.cpp src/Render.cpp src/Game.cpp src/Flappy.cpp src/Texture.cpp src/Sound.cpp ${flappy-bird_files} ./params.json)
target_link_libraries(flappy-bird_TPG  ${GEGELATI_LIBRARIES} sfml-graphics sfml-window sfml-system sfml-audio)
target_compile_definitions(flappy-bird_TPG PRIVATE MNIST_DATA_LOCATION="${MNIST_DATA_DIR}" ROOT_DIR="${CMAKE_SOURCE_DIR}")
