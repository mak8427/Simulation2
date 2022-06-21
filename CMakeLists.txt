cmake_minimum_required(VERSION 3.21)
project(Test1)

set(CMAKE_CXX_STANDARD 20)
add_executable(Test1 main.cpp Agents/Pop.cpp Agents/Pop.h Agents/Factory.cpp Agents/Factory.h Agents/Market.cpp Agents/Market.h Agents/Government.cpp Agents/Government.h lib/Algorithms.h)
