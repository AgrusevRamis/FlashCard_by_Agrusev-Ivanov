#pragma once
#include <SFML/Graphics.hpp>
#include "Card.h"

class FlashcardApp {
    sf::RenderWindow win;
    sf::Font font;
    Screen screen;
    Card cards[MAX];
    Card deck[MAX];
    Card missed[MAX];
    int cardN;
    int deckN;
    int deckI;
    int correct;
    int missedN;
    char wBuf[MWORD];
    char tBuf[MWORD];
    char fileBuf[512];
    char answerBuf[MWORD];
    bool focusW;
    bool cursor;
    bool flipped;
    bool fileError;
    int  writeResult;
    int  editIdx;
    int  editListOffset;

    sf::Color BG, CARD, ACCENT, GREEN, RED, BLUE, GRAY, TXT, MUTED;

    void processEvents();
    void render();
    void drawMenuScreen(sf::Vector2i mouse);
    void drawFileScreen(sf::Vector2i mouse);
    void drawAddScreen(sf::Vector2i mouse);
    void drawEditScreen(sf::Vector2i mouse);
    void drawModeScreen(sf::Vector2i mouse);
    void drawTestScreen(sf::Vector2i mouse);
    void drawWriteScreen(sf::Vector2i mouse);
    void drawResultScreen(sf::Vector2i mouse);

    sf::Text txt(const char* s, unsigned sz, sf::Color c, float x, float y);
    void drawCentered(const char* s, unsigned sz, sf::Color c, float cy);
    void drawRect(float x, float y, float w, float h, sf::Color fill,
                  sf::Color outline = sf::Color::Transparent);
    void drawBtn(const char* s, float x, float y, float w, float h,
                 sf::Color fill, sf::Vector2i mouse);
    bool clicked(sf::Event& e, float x, float y, float w, float h);
    void addUtf8(char* buf, uint32_t code, int maxLen);
    void popUtf8(char* buf);
    void shuffle(Card* a, int n);
    void advanceWriteTest();          // переход к след. карточке в письменном тесте

    bool loadFromFile(const char* path);
    void saveToFile(const char* path);

public:
    FlashcardApp();
    bool initAndRun();
};