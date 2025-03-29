cmake_minimum_required(VERSION 3.12)
project(Middlewares)

set(CMAKE_CXX_STANDARD 17)

include_directories(/usr/local/mysql/include)

link_directories(/usr/local/mysql/lib)

add_executable(MyExecutable src/main.cpp
        include/Singleton.h
        include/Semaphore.h
        include/ThreadSafeQue.h
        include/ThreadSafeStack.h
        include/LockFreeQue.h
        include/ThreadPool.h
        src/ThreadPool.cpp
        include/RingLog.h
        src/RingLog.cpp
        include/json11.h
        src/json11.cpp
        tests/JsonTest.cpp
)

add_executable(JsonTest
        include/json11.h
        src/json11.cpp
        tests/JsonTest.cpp
)
add_executable(Reflection include/Reflect.h
        src/Primitives.cpp
        tests/ReflectTest.cpp)
add_executable(ObjectPool include/ObjectPool.h
        tests/ObjectPoolTest.cpp)
add_executable(TimerTest include/Timer.h
        tests/TimerTest.cpp
)
add_executable(ConnPool include/Connection.h
        src/Connection.cpp
        include/CommonConnectionPool.h
        src/CommonConnectionPool.cpp
        tests/ConnPoolTest.cpp
        include/RingLog.h
        src/RingLog.cpp)

# 明确指定要连接的库
target_link_libraries(ConnPool mysqlclient)
add_executable(MsgQueTest include/Msg.h
        include/MsgQueue.h
        src/Msg.cpp
        src/MsgQueue.cpp
        tests/MsgQueTest.cpp)
