# CMakeList.txt : CMake project for MultiUserQuiz, include source and define
# project specific logic here.

# WebSocket++: disable Boost usage by using standard C++11/17 features
set(WSPP_NO_BOOST_DEFS
    ASIO_STANDALONE
    _WEBSOCKETPP_CPP11_THREAD_
    _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
    _WEBSOCKETPP_CPP11_TYPE_TRAITS_
    _WEBSOCKETPP_CPP11_MEMORY_
    _WEBSOCKETPP_CPP11_FUNCTIONAL_
    _WEBSOCKETPP_CPP11_SYSTEM_ERROR_
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/External/include/
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Answer
    ${CMAKE_CURRENT_SOURCE_DIR}/Config
    ${CMAKE_CURRENT_SOURCE_DIR}/Question
    ${CMAKE_CURRENT_SOURCE_DIR}/QuestionTimer
    ${CMAKE_CURRENT_SOURCE_DIR}/Result
    ${CMAKE_CURRENT_SOURCE_DIR}/User
)

# === External libs directory (same for all builds)
if (WIN32)
    set(EXTERNAL_LIBS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/External/libs/windows)
else()
    set(EXTERNAL_LIBS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/External/libs/linux)
endif()

# Determine library folder based on build type (Debug/Release)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug libraries
    file(GLOB_RECURSE EXTERNAL_LIBS
        ${EXTERNAL_LIBS_ROOT}/debug/*
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Release libraries
    file(GLOB_RECURSE EXTERNAL_LIBS
        ${EXTERNAL_LIBS_ROOT}/release/*
    )
endif()

# ----------------------------
# SERVER TARGET
# ----------------------------
file(GLOB_RECURSE SERVER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/External/include/ini/ini.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Answer/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Config/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Question/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QuestionTimer/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Result/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/User/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ServerApp/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QuizDefs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QuizMgr.cpp
)

add_executable(ServerQuizApp ${SERVER_SOURCES})
target_compile_definitions(ServerQuizApp PRIVATE ${WSPP_NO_BOOST_DEFS})
target_link_libraries(ServerQuizApp ${EXTERNAL_LIBS} crypt32)

# Set server binary output dir
set_target_properties(ServerQuizApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/ServerApp/bin/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/ServerApp/bin/Release
)

# ----------------------------
# CLIENT TARGET
# ----------------------------
file(GLOB_RECURSE CLIENT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/External/include/ini/ini.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Answer/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Config/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Question/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QuestionTimer/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Result/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/User/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ClientApp/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QuizDefs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QuizMgr.cpp
)

add_executable(ClientQuizApp ${CLIENT_SOURCES})
target_compile_definitions(ClientQuizApp PRIVATE ${WSPP_NO_BOOST_DEFS})
target_link_libraries(ClientQuizApp ${EXTERNAL_LIBS} crypt32)

# Set client binary output dir
set_target_properties(ClientQuizApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_CURRENT_SOURCE_DIR}/ClientApp/bin/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/ClientApp/bin/Release
)

# ----------------------------
# Set C++ Standard
# ----------------------------
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET ServerQuizApp PROPERTY CXX_STANDARD 20)
    set_property(TARGET ClientQuizApp PROPERTY CXX_STANDARD 20)
endif()

# ----------------------------
# Copy Resources to binary folders
# ----------------------------
set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/MultiUserQuiz/Resources)

file(GLOB RESOURCE_FILES "${RESOURCE_DIR}/*")

foreach(RESOURCE_FILE ${RESOURCE_FILES})
    add_custom_command(TARGET ServerQuizApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${RESOURCE_FILE} $<TARGET_FILE_DIR:ServerQuizApp>
    )
    add_custom_command(TARGET ClientQuizApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${RESOURCE_FILE} $<TARGET_FILE_DIR:ClientQuizApp>
    )
endforeach()

# TODO: Add tests and install targets if needed.