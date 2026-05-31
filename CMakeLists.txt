cmake_minimum_required(VERSION 3.15)
project(FlashcardsProject VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/sfml@2")

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system)

add_executable(Flashcards
    main.cpp
    FlashcardApp.h
    FlashcardApp.cpp
    Card.h
)

target_include_directories(Flashcards PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Flashcards PRIVATE sfml-graphics sfml-window sfml-system)
