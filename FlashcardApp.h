#pragma once
#include <SFML/Graphics.hpp>
#include "Card.h"

class FlashcardApp {
public:
    FlashcardApp();
    bool initAndRun();

private:
    void processEvents();
    void render();

    void drawMenuScreen(sf::Vector2i mouse);
    void drawAddScreen(sf::Vector2i mouse);
    void drawTestScreen(sf::Vector2i mouse);
    void drawResultScreen(sf::Vector2i mouse);

    sf::Text txt(const char* s, unsigned sz, sf::Color c, float x, float y);
    void drawCentered(const char* s, unsigned sz, sf::Color c, float cy);
    void drawRect(float x, float y, float w, float h, sf::Color fill, sf::Color outline = sf::Color::Transparent);
    void drawBtn(const char* s, float x, float y, float w, float h, sf::Color fill, sf::Vector2i mouse);
    bool clicked(sf::Event& e, float x, float y, float w, float h);

    void addUtf8(char* buf, uint32_t code);
    void shuffle(Card* a, int n);

private:
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
    bool focusW;
    bool cursor;
    bool flipped;

    sf::Color BG;
    sf::Color CARD;
    sf::Color ACCENT;
    sf::Color GREEN;
    sf::Color RED;
    sf::Color BLUE;
    sf::Color GRAY;
    sf::Color TXT;
    sf::Color MUTED;
};
