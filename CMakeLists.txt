cmake_minimum_required(VERSION 3.0.0)

project(testHandler)

set(INCLUDE_DIR 
    ${CMAKE_CURRENT_LIST_DIR}/src
)

set(SRC_FILES
    ${INCLUDE_DIR}/Handler.cpp
    ${INCLUDE_DIR}/HandlerThread.cpp
    ${INCLUDE_DIR}/Looper.cpp
    ${INCLUDE_DIR}/Message.cpp
    ${INCLUDE_DIR}/MessageQueue.cpp
    ${INCLUDE_DIR}/NativeLooper.cpp
    ${INCLUDE_DIR}/NativeMessageQueue.cpp
    ${INCLUDE_DIR}/Timer.cpp
)

include_directories(${INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${INCLUDE_DIR}/main.cpp ${SRC_FILES})

# target_include_directories(${PROJECT_NAME} PUBLIC
    
# )

target_link_libraries(${PROJECT_NAME} PUBLIC
    -lpthread
)
