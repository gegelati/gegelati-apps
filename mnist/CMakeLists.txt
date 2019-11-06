cmake_minimum_required(VERSION 3.15)

# *******************************************
# ************* CMake Content ***************
# *******************************************
# This CMake create a workspace containing the following projects
# 
# Programs
#  - mnist

set (PROJECT_NAME mnist)

project(${PROJECT_NAME})

if(NOT ${CMAKE_GENERATOR} MATCHES "Visual Studio.*")
	# Set to 1 to activate debug compilation (0 for release)
	set(DEBUG 0)

	if(${DEBUG})
			MESSAGE("Generate Debug project")
			set(CMAKE_BUILD_TYPE Debug)
			set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Debug)
			set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -pg -Wall")
	else()
			MESSAGE("Generate Release project")
			set(CMAKE_BUILD_TYPE Release)
			set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
			set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall")
	endif()
	#add libmath during non visual studio builds
	set(CMAKE_EXTRA_LIB m)
else()
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

# *******************************************
# ************ MNIST database ***************
# *******************************************
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dat/train-images-idx3-ubyte.gz")
	message(STATUS "Download MNIST training database 1/4 (may take a few seconds).")
	file(DOWNLOAD http://yann.lecun.com/exdb/mnist/train-images-idx3-ubyte.gz ${CMAKE_CURRENT_SOURCE_DIR}/dat/train-images-idx3-ubyte.gz)
	message(STATUS "Download MNIST training database 2/4 (may take a few seconds).")
	file(DOWNLOAD http://yann.lecun.com/exdb/mnist/train-labels-idx1-ubyte.gz ${CMAKE_CURRENT_SOURCE_DIR}/dat/train-labels-idx1-ubyte.gz)
	message(STATUS "Download MNIST training database 3/4 (may take a few seconds).")
	file(DOWNLOAD http://yann.lecun.com/exdb/mnist/t10k-images-idx3-ubyte.gz ${CMAKE_CURRENT_SOURCE_DIR}/dat/t10k-images-idx3-ubyte.gz)
	message(STATUS "Download MNIST training database 4/4 (may take a few seconds).")
	file(DOWNLOAD http://yann.lecun.com/exdb/mnist/t10k-labels-idx1-ubyte.gz ${CMAKE_CURRENT_SOURCE_DIR}/dat/t10k-labels-idx1-ubyte.gz)
else()
	message(STATUS "MNIST data already downloaded.")
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
	mnist_files
	./src/*.cpp
	./src/*.h
)

include_directories(${GEGELATI_INCLUDE_DIRS})
add_executable(mnist ${mnist_files})
target_link_libraries(mnist ${GEGELATI_LIBRARIES})