# Проверка версии CMake.
cmake_minimum_required(VERSION 3.10)

set(PATH_TO_CXX_COMPILER, "D:/programs/mingw64/bin/g++.exe")
set(PATH_TO_C_COMPILER, "D:/programs/mingw64/bin/gcc.exe")

set(CMAKE_CXX_COMPILER ${PATH_TO_CXX_COMPILER})
set(CMAKE_C_COMPILER ${PATH_TO_C_COMPILER})

# Имя проекта
project(client VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
add_definitions(-D_LIB1_BUILD_DLL)
link_libraries(ws2_32 wsock32)

# ------ Определяем ОС (Кроссплатформенность ЖЭС) ------

set(IS_WINDOWS 0)
set(IS_UNIX 0)

if ((WIN32) OR (MSYS) OR (MINGW) OR (CYGWIN))
	set(IS_WINDOWS 1)
	if (MINGW)
		add_definitions(-D_WIN32_WINNT=0x0601)
	endif(MINGW)
endif ((WIN32) OR (MSYS) OR (MINGW) OR (CYGWIN))

if (UNIX)
	set(IS_UNIX 1)
endif (UNIX)

# -----------Продолжим настройку проекта------------

# Установка
set(SOURCE_EXE main_client.cpp)

# Объявляем header и source
set(SOURCE_LIB client.cpp)

# Создаем статическую библиотеку с именем client_lib
add_library(client_lib STATIC ${SOURCE_LIB})

# Создаем исполняемый файл
add_executable(${PROJECT_NAME} ${SOURCE_EXE} ${SOURCE_LIB})

# -------Линкуем библиотеки в зависимости от ОС--------

# Линкуем библиотеки для Windows
if (IS_WINDOWS)

	set(GCC_COVERAGE_LINK_FLAGS "-lwsock32 -lws2_32")
	add_definitions(${GCC_COVERAGE_LINK_FLAGS})
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
	# link_libraries(-lwsock32 -lws2_32)
	
	find_library(WSOCK32_LIB wsock32)
    find_library(WS2_32_LIB ws2_32)
	if ((WSOCK32_LIB) AND (WS2_32_LIB))
		add_library(wsock32_lib SHARED ${WSOCK32_LIB})
		add_library(ws2_32_lib SHARED ${WS2_32_LIB})
	
		# Линкуем программу с библиотекой
		target_link_libraries(${PROJECT_NAME} client_lib wsock32_lib ws2_32_lib)
	else ((WSOCK32_LIB) AND (WS2_32_LIB))
		#link_directories("C:/Windows/System32")
	endif ((WSOCK32_LIB) AND (WS2_32_LIB))
	
	# Линкуем программу с библиотекой
	target_link_libraries(${PROJECT_NAME} client_lib)
endif (IS_WINDOWS)

# Линкуем библиотеки для UNIX-like
if (UNIX)
	set(GCC_COVERAGE_LINK_FLAGS "-lnsl -lsocket -lresolv")
	add_definitions(${GCC_COVERAGE_LINK_FLAGS})
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
	target_link_libraries(${PROJECT_NAME} client_lib)
endif(UNIX)
