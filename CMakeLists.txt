cmake_minimum_required(VERSION 3.20)
project(HFT_APP VERSION 1.0)

# Configuration C++20
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Trouver la bibliothèque standard de threads (pthread sous Linux)
find_package(Threads REQUIRED)

# ---------------------------------------------------
# EXÉCUTABLE PRINCIPAL (Le moteur de trading)
# ---------------------------------------------------
add_executable(engine src/main.cpp)

# On indique où trouver les headers (.hpp)
target_include_directories(engine PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Flags stricts et optimisés (Uniquement pour l'engine)
target_compile_options(engine PRIVATE
    -Wall -Wextra -Wpedantic -Werror 
    -O3 -march=native
)
# On lie la librairie pthread
target_link_libraries(engine PRIVATE Threads::Threads)

# ---------------------------------------------------
# CONFIGURATION DES TESTS (Google Test)
# ---------------------------------------------------
enable_testing() # Active CTest

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Création de l'exécutable de test
add_executable(unit_tests 
    tests/test_thread_utils.cpp
    tests/test_memory_pool.cpp
    tests/test_lock_free.cpp
)

# Le test doit pouvoir lire vos fichiers dans 'include/'
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)

# On lie Google Test et pthread à notre exécutable de test
target_link_libraries(unit_tests PRIVATE 
    gtest_main 
    Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(unit_tests)