cmake_minimum_required(VERSION 3.12)
project(Middlewares)

set(CMAKE_CXX_STANDARD 17)

add_executable(MyExecutable src/main.cpp
        include/Singleton.h
        include/Semaphore.h
        include/ThreadSafeQue.h
        include/ThreadSafeStack.h
        include/LockFreeQue.h
        include/ThreadPool.h
        src/ThreadPool.cpp
)