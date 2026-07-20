cmake_minimum_required(VERSION 3.21)
project(Test1)

set(CMAKE_CXX_STANDARD 20)

# Main application
add_executable(Test1 main.cpp
    Agents/Pop.cpp Agents/Pop.h
    Agents/Factory.cpp Agents/Factory.h
    Agents/Market.cpp Agents/Market.h
    Agents/Government.cpp Agents/Government.h
    lib/Algorithms.h
)

# Common agent sources for tests
set(AGENT_SOURCES
    Agents/Pop.cpp Agents/Pop.h
    Agents/Factory.cpp Agents/Factory.h
    Agents/Market.cpp Agents/Market.h
    Agents/Government.cpp Agents/Government.h
    lib/Algorithms.h
)

# Test executables
add_executable(test_pop
    tests/test_pop.cpp
    ${AGENT_SOURCES}
)

add_executable(test_factory
    tests/test_factory.cpp
    ${AGENT_SOURCES}
)

add_executable(test_market
    tests/test_market.cpp
    ${AGENT_SOURCES}
)

add_executable(test_government
    tests/test_government.cpp
    ${AGENT_SOURCES}
)

add_executable(test_e2e
    tests/test_e2e.cpp
    ${AGENT_SOURCES}
)
