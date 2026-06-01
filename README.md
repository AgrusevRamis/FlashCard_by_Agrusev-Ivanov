# 📑 Карточки для запоминаний (BMSTU PROJECT)

Приложение для запоминнаия иностранных слов с использование флеш-карт, разработанное в рамках проекта по программированию для первого курса.

## 👑 Особенности
- **Добавление слов**: создавайте свою собственную коллекцию слов и переводов.
- **Режим тестирования**: проверьте свои знания с помощью интерактивных карточек.
- **Статистика результатов**: отслеживайте количество правильных ответов и процент успеха.
- **Работа с ошибками**: программа запоминает слова, вызвавшие затруднения, и предлагает их для повторения.
- **Красивый GUI**: современный интерфейс, реализованный с помощью SFML.
- **Поддержка UTF-8**: возможность использования многих языков (английский, русский, и др.).

## 🛠 Технологический стек
- **Основная логика**: C++17
- **GUI-фреймворк**: SFML 2.x (Simple and Fast Multimedia Library)
- **Графика**: аппаратно-ускоренный рендеринг через OpenGL

## 📦 Установка и настройка

### Для macOS
``` bash
# Установка SFML
brew install sfml@2
```
### Для Linux
```bash
# Установка SFML
sudo apt update
sudo apt install libsfml-dev
```

# Сборка
Для сборки нужно создать CMakeLists.txt и вставить туда следующий код:
```bash
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
```
Затем создать папку build и преейти туда
```bash
mkdir build
cd build
```
А затем собрать проект и запустить его
```bash
cmake ..
cmake --build .
./Flashcards
```


## 💶 Как использовать
1. Запустите приложение.
2. Нажмите **"Добавить слова"**.
3. Вставьте слова, которые хотите добавить.
4. Выйдете и нажмите **"Начать тест"**.
5. Проверьте себя и узнайте количество ошибок!

---
Сделано с ❤️ студентами МГТУ им. Баумана.
