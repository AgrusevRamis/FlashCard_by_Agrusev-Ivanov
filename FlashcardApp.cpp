#include "FlashcardApp.h"
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cstdio>

FlashcardApp::FlashcardApp() :
    win(sf::VideoMode(W, H), "Flashcards", sf::Style::Titlebar | sf::Style::Close),
    screen(MENU), cardN(0), deckN(0), deckI(0), correct(0), missedN(0),
    focusW(true), cursor(true), flipped(false){
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    win.setFramerateLimit(60);

    wBuf[0] = '\0';
    tBuf[0] = '\0';

    BG = sf::Color(20, 22, 35);
    CARD = sf::Color(35, 40, 70);
    ACCENT = sf::Color(130, 160, 255);
    GREEN = sf::Color(50, 160, 90);
    RED = sf::Color(180, 60, 60);
    BLUE = sf::Color(60, 80, 180);
    GRAY = sf::Color(50, 55, 80);
    TXT = sf::Color(230, 235, 255);
    MUTED = sf::Color(140, 150, 190);
    }

bool FlashcardApp::initAndRun() {
    #ifdef _WIN32
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
    #elif __linux__
        if (!font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf")) {
    #else
        if (!font.loadFromFile("arial.ttf")) {
    #endif
        return false;
    }
    while (win.isOpen()) {
        processEvents();
        render();
    }
    return true;
}

void FlashcardApp::processEvents() {
    sf::Event e;
    auto mouse = sf::Mouse::getPosition(win);

    while (win.pollEvent(e)) {
        if (e.type == sf::Event::Closed) win.close();

        if (screen == MENU) {
            if (cardN == 0 && clicked(e, 300, 270, 180, 50)){
                wBuf[0] = tBuf[0] = '\0';
                focusW = true;
                screen = ADD;
            }
            if (clicked(e, 200, 310, 180, 50) && cardN > 0) {
                wBuf[0] = tBuf[0] = '\0';
                focusW = true;
                screen = ADD;
            }
            if (clicked(e, 420, 310, 180, 50) && cardN > 0) {
                for (int i = 0; i < cardN; i++) deck[i] = cards[i];
                deckN = cardN; deckI = 0; correct = 0; missedN = 0; flipped = false;
                shuffle(deck, deckN);
                screen = TEST;
            }
        }
        else if (screen == ADD) {
            if (clicked(e, 50, 30, 120, 40)) screen = MENU;
            if (clicked(e, 270, 400, 260, 50)) {
                if (wBuf[0] && tBuf[0] && cardN < MAX) {
                    std::strcpy(cards[cardN].w, wBuf);
                    std::strcpy(cards[cardN].t, tBuf);
                    cardN++;
                    wBuf[0] = tBuf[0] = '\0';
                    focusW = true;
                }
            }
            if (clicked(e, 170, 210, 460, 50)) focusW = true;
            if (clicked(e, 170, 310, 460, 50)) focusW = false;

            if (e.type == sf::Event::TextEntered) {
                char* buf = focusW ? wBuf : tBuf;
                uint32_t c = e.text.unicode;
                if (c == 8) {
                    int l = std::strlen(buf);
                    if (l > 0) buf[l - 1] = '\0';
                }
                else if (c == '\t' || c == '\r') focusW = !focusW;
                else if (c >= 32) addUtf8(buf, c);
            }
        }
        else if (screen == TEST) {
            if (clicked(e, 50, 30, 120, 40)) screen = MENU;
            if (!flipped && clicked(e, 300, 460, 200, 50)) flipped = true;
            if (flipped) {
                bool act = false;
                if (clicked(e, 460, 460, 160, 50)) { correct++; act = true; }
                if (clicked(e, 180, 460, 160, 50)) { missed[missedN++] = deck[deckI]; act = true; }
                if (act) {
                    deckI++;
                    if (deckI >= deckN) screen = RESULT;
                    else flipped = false;
                }
            }
        }
        else if (screen == RESULT) {
            if (clicked(e, 190, 460, 180, 50)) screen = MENU;
            if (clicked(e, 430, 460, 180, 50)) {
                for (int i = 0; i < cardN; i++) deck[i] = cards[i];
                deckN = cardN; deckI = 0; correct = 0; missedN = 0; flipped = false;
                shuffle(deck, deckN);
                screen = TEST;
            }
        }
    }
}

void FlashcardApp::render() {
    win.clear(BG);
    auto mouse = sf::Mouse::getPosition(win);

    switch (screen) {
        case MENU:   drawMenuScreen(mouse);   break;
        case ADD:    drawAddScreen(mouse);    break;
        case TEST:   drawTestScreen(mouse);   break;
        case RESULT: drawResultScreen(mouse); break;
    }

    win.display();
}

void FlashcardApp::drawMenuScreen(sf::Vector2i mouse) {
    drawCentered("Flashcards", 48, ACCENT, 80);
    char buf[64];
    std::snprintf(buf, 64, "Карточек: %d", cardN);
    drawCentered(buf, 22, MUTED, 190);
    if (cardN==0){drawBtn("Добавить слово", 300, 270, 180, 50, BLUE, mouse);
    }
    else{
        drawBtn("Добавить слово", 200, 310, 180, 50, BLUE, mouse);
        drawBtn("Начать тест", 420, 310, 180, 50, BLUE, mouse);
    }
    if (cardN == 0) drawCentered("Добавьте слова для теста", 16, RED, 378);
}

void FlashcardApp::drawAddScreen(sf::Vector2i mouse) {
    drawBtn("< Назад", 50, 30, 120, 40, GRAY, mouse);
    drawCentered("Добавить слово", 34, ACCENT, 80);

    auto fieldBg = [&](float y, bool focus) {
        drawRect(170, y, 460, 50, sf::Color(30, 34, 55),
                 focus ? sf::Color(ACCENT.r, ACCENT.g, ACCENT.b, 200) : sf::Color(70, 80, 120));
    };

    fieldBg(210, focusW);
    win.draw(txt("Слово:", 18, MUTED, 170, 185));
    char wd[MWORD + 2];
    std::snprintf(wd, sizeof(wd), "%s%s", wBuf, (focusW && cursor) ? "|" : "");
    win.draw(txt(wd, 22, TXT, 182, 220));

    fieldBg(310, !focusW);
    win.draw(txt("Перевод:", 18, MUTED, 170, 285));
    char td[MWORD + 2];
    std::snprintf(td, sizeof(td), "%s%s", tBuf, (!focusW && cursor) ? "|" : "");
    win.draw(txt(td, 22, TXT, 182, 320));

    drawBtn("Добавить", 270, 400, 260, 50, BLUE, mouse);
}

void FlashcardApp::drawTestScreen(sf::Vector2i mouse) {
    drawBtn("< Назад", 50, 30, 120, 40, GRAY, mouse);

    drawRect(100, 75, 600, 8, sf::Color(50, 55, 90));
    float filled = deckN > 0 ? (float)deckI / deckN * 600 : 0;
    drawRect(100, 75, filled, 8, ACCENT);
    char pb[32];
    std::snprintf(pb, 32, "%d / %d", deckI + 1, deckN);
    drawCentered(pb, 18, MUTED, 90);

    const float cx = 150, cy = 150, cw = 500, ch = 260;
    drawRect(cx, cy, cw, ch, flipped ? sf::Color(40, 65, 55) : CARD,
             flipped ? GREEN : sf::Color(80, 100, 180));
    drawCentered(flipped ? "СЛОВО" : "ПЕРЕВОД", 14, flipped ? GREEN : ACCENT, cy + 20);
    drawCentered(flipped ? deck[deckI].w : deck[deckI].t, 32, TXT, cy + 95);

    if (!flipped) drawBtn("Перевернуть", 300, 460, 200, 50, BLUE, mouse);
    else {
        drawBtn("Не знал",100, 460, 160, 50, RED, mouse);
        drawBtn("Знал!", 540, 460, 160, 50, GREEN, mouse);
    }
}

void FlashcardApp::drawResultScreen(sf::Vector2i mouse) {
    drawCentered("Результат", 42, ACCENT, 60);
    int pct = deckN > 0 ? correct * 100 / deckN : 0;
    char sb[32];
    std::snprintf(sb, 32, "%d / %d", correct, deckN);
    drawCentered(sb, 52, pct >= 70 ? GREEN : RED, 130);
    char pb[16];
    std::snprintf(pb, 16, "(%d%%)", pct);
    drawCentered(pb, 24, MUTED, 195);

    const char* msg = pct == 100 ? "Все слова знаешь!" : pct >= 70 ? "Хороший результат!" : "Стоит повторить.";
    drawCentered(msg, 22, TXT, 240);

    if (missedN > 0) {
        win.draw(txt("Повторить:", 18, RED, 100, 285));
        int n = missedN < 5 ? missedN : 5;
        for (int i = 0; i < n; i++) {
            char line[MWORD * 2 + 4];
            std::snprintf(line, sizeof(line), "%s -> %s", missed[i].w, missed[i].t);
            win.draw(txt(line, 15, MUTED, 100, 310 + i * 22.f));
        }
    }
    drawBtn("В меню", 190, 460, 180, 50, GRAY, mouse);
    drawBtn("Ещё раз", 430, 460, 180, 50, BLUE, mouse);
}

sf::Text FlashcardApp::txt(const char* s, unsigned sz, sf::Color c, float x, float y) {
    sf::Text t;
    t.setFont(font);
    t.setString(sf::String::fromUtf8(s, s + std::strlen(s)));
    t.setCharacterSize(sz);
    t.setFillColor(c);
    t.setPosition(x, y);
    return t;
}

void FlashcardApp::drawCentered(const char* s, unsigned sz, sf::Color c, float cy) {
    auto t = txt(s, sz, c, 0, 0);
    auto b = t.getLocalBounds();
    t.setPosition(W / 2.f - b.width / 2.f - b.left, cy);
    win.draw(t);
}

void FlashcardApp::drawRect(float x, float y, float w, float h, sf::Color fill, sf::Color outline) {
    sf::RectangleShape r({w, h});
    r.setPosition(x, y);
    r.setFillColor(fill);
    if (outline != sf::Color::Transparent) {
        r.setOutlineThickness(2);
        r.setOutlineColor(outline);
    }
    win.draw(r);
}

bool FlashcardApp::clicked(sf::Event& e, float x, float y, float w, float h) {
    return e.type == sf::Event::MouseButtonPressed &&
           e.mouseButton.x >= x && e.mouseButton.x <= x + w &&
           e.mouseButton.y >= y && e.mouseButton.y <= y + h;
}

void FlashcardApp::drawBtn(const char* s, float x, float y, float w, float h, sf::Color fill, sf::Vector2i mouse) {
    bool hov = mouse.x >= x && mouse.x <= x + w && mouse.y >= y && mouse.y <= y + h;
    drawRect(x, y, w, h, hov ? sf::Color(fill.r + 30, fill.g + 30, fill.b + 30) : fill);
    auto t = txt(s, 18, sf::Color::White, 0, 0);
    auto b = t.getLocalBounds();
    t.setPosition(x + w / 2 - b.width / 2 - b.left, y + h / 2 - b.height / 2 - b.top);
    win.draw(t);
}

void FlashcardApp::addUtf8(char* buf, uint32_t code) {
    int len = std::strlen(buf);
    if (len >= MWORD - 4) return;
    if (code < 0x80) { buf[len] = code; buf[len + 1] = 0; }
    else if (code < 0x800) { buf[len] = 0xC0 | (code >> 6); buf[len + 1] = 0x80 | (code & 0x3F); buf[len + 2] = 0; }
    else { buf[len] = 0xE0 | (code >> 12); buf[len + 1] = 0x80 | ((code >> 6) & 0x3F); buf[len + 2] = 0x80 | (code & 0x3F); buf[len + 3] = 0; }
}

void FlashcardApp::shuffle(Card* a, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = std::rand() % (i + 1);
        Card tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
    }
}
