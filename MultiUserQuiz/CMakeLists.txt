# CMakeList.txt : CMake project for MultiUserQuiz, include source and define
# project specific logic here.
#
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/External/
    ${CMAKE_CURRENT_SOURCE_DIR}/External/ini
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Answer
    ${CMAKE_CURRENT_SOURCE_DIR}/Config
    ${CMAKE_CURRENT_SOURCE_DIR}/Question
    ${CMAKE_CURRENT_SOURCE_DIR}/QuestionTimer
    ${CMAKE_CURRENT_SOURCE_DIR}/Result
    ${CMAKE_CURRENT_SOURCE_DIR}/User
)

# Determine library folder based on build type (Debug/Release)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug libraries
    file(GLOB_RECURSE EXTERNAL_LIBS
        ${CMAKE_CURRENT_SOURCE_DIR}/External/libs/windows/Debug/*.lib
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Release libraries
    file(GLOB_RECURSE EXTERNAL_LIBS
        ${CMAKE_CURRENT_SOURCE_DIR}/External/libs/windows/Release/*.lib
    )
endif()

# Combined source files
file(GLOB_RECURSE SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Answer/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Config/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Question/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QuestionTimer/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Result/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/User/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/External/ini/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# Add source to this project's executable.
add_executable (MultiUserQuiz ${SRC_FILES})

# Link external libraries
target_link_libraries(MultiUserQuiz ${EXTERNAL_LIBS})

# Define resource directory
set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/Resources)

# Define where to copy to (next to the binary)
#set(OUTPUT_RESOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/Resources)
# Copy the Resources folder to the build output directory
#add_custom_command(TARGET MultiUserQuiz POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#    ${RESOURCE_DIR} ${OUTPUT_RESOURCE_DIR}
#)

# Get all files in the Resources folder (non-recursive, or make recursive if needed)
file(GLOB RESOURCE_FILES "${RESOURCE_DIR}/*")

# Copy each file into the same folder as the binary (Debug/Release folders)
foreach(RESOURCE_FILE ${RESOURCE_FILES})
    add_custom_command(TARGET MultiUserQuiz POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${RESOURCE_FILE} $<TARGET_FILE_DIR:MultiUserQuiz>
    )
endforeach()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MultiUserQuiz PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
