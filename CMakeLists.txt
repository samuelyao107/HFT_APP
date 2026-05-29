cmake_minimum_required(VERSION 3.20)

# Nom de votre projet
project(HFT_APP VERSION 1.0)

# ---------------------------------------------------
# Configuration du Standard C++
# ---------------------------------------------------
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Exiger C++20
set(CMAKE_CXX_STANDARD 20)
# Empêcher CMake de fallback sur une version plus ancienne
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Désactiver les extensions spécifiques au compilateur (ex: GNU) 
# pour garantir que votre code est 100% portable
set(CMAKE_CXX_EXTENSIONS OFF) 

# Création de l'exécutable
add_executable(engine src/main.cpp)

# ---------------------------------------------------
# Flags de compilation (Qualité et Performance)
# ---------------------------------------------------
target_compile_options(engine PRIVATE
    -Wall -Wextra -Wpedantic -Werror # Tolérance zéro pour les warnings
    -O3                              # Optimisation agressive
    -march=native                    # Optimisation pour l'architecture locale
)