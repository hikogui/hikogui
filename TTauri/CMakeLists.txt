
cmake_minimum_required(VERSION 3.0)

project(TTauri)

add_subdirectory(Draw)
add_subdirectory(GUI)

add_library(${PROJECT_NAME} STATIC
	Application.cpp
	Application_win32.cpp
	Logging.cpp
	strings.cpp
	strings_tests.cpp
	Time.cpp
	Timestamp.cpp
)
