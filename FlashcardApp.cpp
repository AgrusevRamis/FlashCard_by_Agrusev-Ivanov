#include "FlashcardApp.h"
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <algorithm>

FlashcardApp::FlashcardApp()
    : win(sf::VideoMode(W, H), "Flashcards",
          sf::Style::Titlebar | sf::Style::Close),
      screen(MENU), cardN(0), deckN(0), deckI(0),
      correct(0), missedN(0),
      focusW(true), cursor(true), flipped(false),
      editIdx(-1), editListOffset(0), fileError(false)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    win.setFramerateLimit(60);

    wBuf[0] = tBuf[0] = fileBuf[0] = answerBuf[0] = '\0';
    writeResult = 0;

    BG     = sf::Color(20,  22,  35);
    CARD   = sf::Color(35,  40,  70);
    ACCENT = sf::Color(130, 160, 255);
    GREEN  = sf::Color(50,  160, 90);
    RED    = sf::Color(180, 60,  60);
    BLUE   = sf::Color(60,  80,  180);
    GRAY   = sf::Color(50,  55,  80);
    TXT    = sf::Color(230, 235, 255);
    MUTED  = sf::Color(140, 150, 190);
}

static bool matchAnswer(const char* ans, const char* correct) {
    char buf[MWORD];
    std::strncpy(buf, correct, MWORD - 1);
    buf[MWORD - 1] = '\0';

    char* tok = std::strtok(buf, "/");
    while (tok) {

        while (*tok == ' ') tok++;
        char* end = tok + std::strlen(tok) - 1;
        while (end > tok && *end == ' ') { *end = '\0'; end--; }

        if (std::strcmp(ans, tok) == 0) return true;
        tok = std::strtok(nullptr, "/");
    }
    return false;
}

static void toLowerAscii(char* s) {
    for (; *s; s++)
        if (*s >= 'A' && *s <= 'Z') *s += 32;
}

bool FlashcardApp::initAndRun() {
#ifdef _WIN32
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf"))
#elif __linux__
    if (!font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"))
#else
    if (!font.loadFromFile("arial.ttf"))
#endif
        return false;

    while (win.isOpen()) {
        processEvents();
        render();
    }

    if (fileBuf[0] && cardN > 0)
        saveToFile(fileBuf);

    return true;
}

bool FlashcardApp::loadFromFile(const char* path) {
    FILE* f = std::fopen(path, "r");
    if (!f) return false;

    char line[MWORD * 2 + 8];
    int loaded = 0;
    while (std::fgets(line, sizeof(line), f) && cardN < MAX) {
        int len = std::strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            line[--len] = '\0';
        if (len == 0) continue;

        char* sep = std::strchr(line, '\t');
        if (!sep) sep = std::strchr(line, ';');
        if (!sep) continue;

        *sep = '\0';
        const char* word  = line;
        const char* trans = sep + 1;
        if (*word == '\0' || *trans == '\0') continue;

        std::strncpy(cards[cardN].w, word,  MWORD - 1);
        std::strncpy(cards[cardN].t, trans, MWORD - 1);
        cards[cardN].w[MWORD-1] = cards[cardN].t[MWORD-1] = '\0';
        cardN++;
        loaded++;
    }
    std::fclose(f);
    return loaded > 0;
}

void FlashcardApp::saveToFile(const char* path) {
    FILE* f = std::fopen(path, "w");
    if (!f) return;
    for (int i = 0; i < cardN; i++)
        std::fprintf(f, "%s\t%s\n", cards[i].w, cards[i].t);
    std::fclose(f);
}

void FlashcardApp::advanceWriteTest() {
    deckI++;
    if (deckI >= deckN) {
        screen = RESULT;
    } else {
        answerBuf[0] = '\0';
        writeResult = 0;
    }
}

void FlashcardApp::processEvents() {
    sf::Event e;
    while (win.pollEvent(e)) {
        if (e.type == sf::Event::Closed) {
            win.close();
            return;
        }

        auto mouse = sf::Mouse::getPosition(win);

        if (screen == MENU) {
            if (clicked(e, 200, 240, 180, 50)) {
                fileBuf[0] = '\0';
                screen = FILE_LOAD;
            }
            if (clicked(e, 420, 240, 180, 50)) {
                wBuf[0] = tBuf[0] = '\0';
                focusW = true;
                screen = ADD;
            }
            if (cardN > 0) {
                if (clicked(e, 200, 320, 180, 50)) {
                    for (int i = 0; i < cardN; i++) deck[i] = cards[i];
                    deckN = cardN; deckI = 0; correct = 0; missedN = 0; flipped = false;
                    shuffle(deck, deckN);
                    screen = MODE_SELECT;
                }
                if (clicked(e, 420, 320, 180, 50)) {
                    editIdx = -1;
                    editListOffset = 0;
                    screen = EDIT_LIST;
                }
            }
        }

        else if (screen == MODE_SELECT) {
            if (clicked(e, 50, 30, 120, 40)) screen = MENU;
            if (clicked(e, 200, 260, 180, 60)) screen = TEST;
            if (clicked(e, 420, 260, 180, 60)) {
                answerBuf[0] = '\0';
                writeResult = 0;
                screen = WRITE_TEST;
            }
        }

        else if (screen == FILE_LOAD) {
            if (clicked(e, 50, 30, 120, 40)) screen = MENU;
            if (clicked(e, 270, 330, 260, 50)) {
                if (fileBuf[0]) {
                    if (loadFromFile(fileBuf)) { fileError = false; screen = MENU; }
                    else fileError = true;
                }
            }
            if (e.type == sf::Event::TextEntered) {
                uint32_t c = e.text.unicode;
                if (c == 8 || c == 127) { popUtf8(fileBuf); fileError = false; }
                else if (c == '\r' || c == '\n') {
                    if (fileBuf[0]) {
                        if (loadFromFile(fileBuf)) { fileError = false; screen = MENU; }
                        else fileError = true;
                    }
                }
                else if (c >= 32) { addUtf8(fileBuf, c, 510); fileError = false; }
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
                if (c == 8 || c == 127) popUtf8(buf);
                else if (c == '\t' || c == '\r') focusW = !focusW;
                else if (c >= 32) addUtf8(buf, c, MWORD);
            }
        }

        else if (screen == EDIT_LIST) {
            if (clicked(e, 50, 30, 120, 40)) {
                if (editIdx >= 0) editIdx = -1;
                else screen = MENU;
            }

            if (editIdx < 0) {

                const int ROWS = 8;
                const float rowH = 44.f;
                const float listY = 100.f;
                for (int i = 0; i < ROWS; i++) {
                    int ci = editListOffset + i;
                    if (ci >= cardN) break;
                    float ry = listY + i * rowH;
                    if (clicked(e, 560, ry + 6, 70, 30)) {
                        editIdx = ci;
                        std::strncpy(wBuf, cards[ci].w, MWORD);
                        std::strncpy(tBuf, cards[ci].t, MWORD);
                        focusW = true;
                    }
                    if (clicked(e, 638, ry + 6, 70, 30)) {
                        for (int k = ci; k < cardN - 1; k++)
                            cards[k] = cards[k+1];
                        cardN--;
                        if (editListOffset > 0 && editListOffset >= cardN)
                            editListOffset -= ROWS;
                    }
                }
                if (clicked(e, 320, 470, 80, 35) && editListOffset > 0)
                    editListOffset -= ROWS;
                if (clicked(e, 420, 470, 80, 35) && editListOffset + ROWS < cardN)
                    editListOffset += ROWS;
            }
            else {

                if (clicked(e, 170, 210, 460, 50)) focusW = true;
                if (clicked(e, 170, 310, 460, 50)) focusW = false;
                if (clicked(e, 270, 400, 260, 50)) {
                    if (wBuf[0] && tBuf[0]) {
                        std::strncpy(cards[editIdx].w, wBuf, MWORD);
                        std::strncpy(cards[editIdx].t, tBuf, MWORD);
                        editIdx = -1;
                    }
                }
                if (e.type == sf::Event::TextEntered) {
                    char* buf = focusW ? wBuf : tBuf;
                    uint32_t c = e.text.unicode;
                    if (c == 8 || c == 127) popUtf8(buf);
                    else if (c == '\t' || c == '\r') focusW = !focusW;
                    else if (c >= 32) addUtf8(buf, c, MWORD);
                }
            }
        }

        else if (screen == WRITE_TEST) {

            if (clicked(e, 50, 30, 130, 40)) screen = RESULT;

            if (writeResult != 0 && clicked(e, 300, 430, 200, 50)) {
                advanceWriteTest();
            }

            if (e.type == sf::Event::TextEntered) {
                uint32_t c = e.text.unicode;

                if (writeResult == 0) {

                    if (c == 8 || c == 127) {
                        popUtf8(answerBuf);
                    }
                    else if (c == '\r' || c == '\n') {

                        char ans[MWORD], cor[MWORD];
                        std::strncpy(ans, answerBuf, MWORD);
                        std::strncpy(cor, deck[deckI].t, MWORD);
                        toLowerAscii(ans);
                        toLowerAscii(cor);
                        writeResult = matchAnswer(ans, cor) ? 1 : -1;
                        if (writeResult == 1) correct++;
                        else missed[missedN++] = deck[deckI];

                    }
                    else if (c >= 32) {
                        addUtf8(answerBuf, c, MWORD);
                    }
                }
                else {

                    if (c == '\r' || c == '\n') {
                        advanceWriteTest();
                    }
                }
            }
        }

        else if (screen == TEST) {

            if (clicked(e, 50, 30, 130, 40)) screen = RESULT;

            if (!flipped && clicked(e, 300, 460, 200, 50)) flipped = true;
            if (flipped) {
                bool act = false;
                if (clicked(e, 540, 460, 160, 50)) { correct++; act = true; }
                if (clicked(e, 100, 460, 160, 50)) { missed[missedN++] = deck[deckI]; act = true; }
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
                screen = MODE_SELECT;
            }
        }
    }
}

void FlashcardApp::render() {
    win.clear(BG);
    auto mouse = sf::Mouse::getPosition(win);
    switch (screen) {
        case MENU:        drawMenuScreen(mouse);   break;
        case FILE_LOAD:   drawFileScreen(mouse);   break;
        case ADD:         drawAddScreen(mouse);    break;
        case EDIT_LIST:   drawEditScreen(mouse);   break;
        case MODE_SELECT: drawModeScreen(mouse);   break;
        case TEST:        drawTestScreen(mouse);   break;
        case WRITE_TEST:  drawWriteScreen(mouse);  break;
        case RESULT:      drawResultScreen(mouse); break;
    }
    win.display();
}

void FlashcardApp::drawMenuScreen(sf::Vector2i mouse) {
    drawCentered("Flashcards", 48, ACCENT, 60);

    char buf[64];
    std::snprintf(buf, 64, "Карточек: %d", cardN);
    drawCentered(buf, 22, MUTED, 130);

    if (fileBuf[0]) {
        char fb[128];
        std::snprintf(fb, 128, "Файл: %s", fileBuf);
        drawCentered(fb, 15, MUTED, 155);
    }

    drawBtn("Загрузить из файла", 200, 240, 180, 50, BLUE, mouse);
    drawBtn("Добавить слово",     420, 240, 180, 50, BLUE, mouse);

    if (cardN > 0) {
        drawBtn("Начать тест",   200, 320, 180, 50, GREEN,                  mouse);
        drawBtn("Редактировать", 420, 320, 180, 50, sf::Color(90, 60, 140), mouse);
    } else {
        drawCentered("Загрузите файл или добавьте слова вручную", 16, MUTED, 390);
    }
}

void FlashcardApp::drawModeScreen(sf::Vector2i mouse) {
    drawBtn("< Назад", 50, 30, 120, 40, GRAY, mouse);
    drawCentered("Выберите режим", 38, ACCENT, 80);
    drawCentered("Как вы хотите проверить себя?", 18, MUTED, 150);

    drawBtn("Устный",     200, 260, 180, 60, BLUE,  mouse);
    drawBtn("Письменный", 420, 260, 180, 60, GREEN, mouse);

    win.draw(txt("Видите перевод - вспоминаете слово.", 15, MUTED, 130, 360));
    win.draw(txt("Видите слово - пишете перевод.",      15, MUTED, 420, 360));
}

void FlashcardApp::drawWriteScreen(sf::Vector2i mouse) {

    drawBtn("Завершить", 50, 30, 130, 40, RED, mouse);

    drawRect(200, 38, 550, 8, sf::Color(50, 55, 90));
    float filled = deckN > 0 ? (float)deckI / deckN * 550.f : 0.f;
    drawRect(200, 38, filled, 8, ACCENT);
    char pb[32];
    std::snprintf(pb, 32, "%d / %d", deckI + 1, deckN);
    drawCentered(pb, 16, MUTED, 52);

    drawRect(150, 100, 500, 100, CARD, sf::Color(80, 100, 180));
    drawCentered("СЛОВО", 13, ACCENT, 114);
    drawCentered(deck[deckI].w, 32, TXT, 138);

    if (writeResult == 0) {

        win.draw(txt("Введите перевод:", 18, MUTED, 170, 230));
        drawRect(170, 255, 460, 50, sf::Color(30, 34, 55), ACCENT);
        char show[MWORD + 2];
        std::snprintf(show, sizeof(show), "%s|", answerBuf);
        win.draw(txt(show, 22, TXT, 182, 265));
        win.draw(txt("Нажмите Enter для проверки", 14, MUTED, 270, 318));

        int variants = 1;
        for (const char* p = deck[deckI].t; *p; p++)
            if (*p == '/') variants++;
        if (variants > 1) {
            char hint[48];
            std::snprintf(hint, 48, "Вариантов ответа: %d (любой верный)", variants);
            win.draw(txt(hint, 13, MUTED, 170, 345));
        }
    }
    else {
        bool ok = (writeResult == 1);

        win.draw(txt("Ваш ответ:", 16, MUTED, 170, 235));
        win.draw(txt(answerBuf, 20, ok ? GREEN : RED, 170, 258));

        drawCentered(ok ? "Верно!" : "Неверно", 30, ok ? GREEN : RED, 295);

        if (!ok) {
            win.draw(txt("Правильный ответ:", 16, MUTED, 170, 342));

            win.draw(txt(deck[deckI].t, 20, TXT, 170, 366));
        }

        drawBtn("Далее  →", 300, 430, 200, 50, BLUE, mouse);
        win.draw(txt("(или нажмите Enter)", 13, MUTED, 355, 490));
    }
}

void FlashcardApp::drawFileScreen(sf::Vector2i mouse) {
    drawBtn("< Назад", 50, 30, 120, 40, GRAY, mouse);
    drawCentered("Загрузить из файла", 34, ACCENT, 80);

    win.draw(txt("Введите путь к файлу (.txt):", 18, MUTED, 170, 170));
    win.draw(txt("Формат строк:  слово TAB перевод", 14, MUTED, 170, 196));
    win.draw(txt("               или:  слово;перевод", 14, MUTED, 170, 214));

    drawRect(170, 240, 460, 50, sf::Color(30, 34, 55), ACCENT);
    char show[514];
    std::snprintf(show, sizeof(show), "%s|", fileBuf);
    win.draw(txt(show, 20, TXT, 182, 252));

    drawBtn("Загрузить", 270, 330, 260, 50, BLUE, mouse);
    win.draw(txt("(Enter - тоже подтверждает)", 14, MUTED, 270, 392));
    if (fileError)
        drawCentered("Файл не найден! Проверьте путь.", 16, RED, 440);
}

void FlashcardApp::drawAddScreen(sf::Vector2i mouse) {
    drawBtn("< Назад", 50, 30, 120, 40, GRAY, mouse);
    drawCentered("Добавить слово", 34, ACCENT, 80);

    auto fieldBg = [&](float y, bool focus) {
        drawRect(170, y, 460, 50, sf::Color(30, 34, 55),
                 focus ? sf::Color(ACCENT.r, ACCENT.g, ACCENT.b, 200)
                       : sf::Color(70, 80, 120));
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

    char lb[32];
    std::snprintf(lb, 32, "Добавлено (%d):", cardN);
    win.draw(txt(lb, 16, MUTED, 170, 470));
    int n = cardN < 3 ? cardN : 3;
    for (int i = 0; i < n; i++) {
        char line[MWORD * 2 + 8];
        std::snprintf(line, sizeof(line), "%d. %s → %s",
                      cardN - i, cards[cardN-1-i].w, cards[cardN-1-i].t);
        win.draw(txt(line, 14, MUTED, 170, 494 + i * 20.f));
    }
}

void FlashcardApp::drawEditScreen(sf::Vector2i mouse) {
    if (editIdx < 0) {
        drawBtn("< Назад", 50, 30, 120, 40, GRAY, mouse);
        drawCentered("Редактировать карточки", 28, ACCENT, 55);

        const int   ROWS  = 8;
        const float rowH  = 44.f;
        const float listY = 100.f;

        for (int i = 0; i < ROWS; i++) {
            int ci = editListOffset + i;
            if (ci >= cardN) break;
            float ry = listY + i * rowH;

            sf::Color rowBg = (i % 2 == 0) ? sf::Color(30, 34, 55)
                                             : sf::Color(25, 28, 48);
            drawRect(80, ry, 630, rowH - 2, rowBg);

            char line[MWORD * 2 + 8];
            std::snprintf(line, sizeof(line), "%d.  %s  →  %s",
                          ci + 1, cards[ci].w, cards[ci].t);
            win.draw(txt(line, 14, TXT, 90, ry + 13));

            drawBtn("Изм.",  560, ry + 6, 70, 30, sf::Color(60, 80, 180),  mouse);
            drawBtn("Удал.", 638, ry + 6, 70, 30, sf::Color(160, 50, 50),  mouse);
        }

        if (editListOffset > 0)
            drawBtn("< Пред.", 320, 470, 80, 35, GRAY, mouse);
        if (editListOffset + ROWS < cardN)
            drawBtn("След. >", 420, 470, 80, 35, GRAY, mouse);

        char pg[32];
        std::snprintf(pg, 32, "%d-%d из %d",
                      editListOffset + 1,
                      std::min(editListOffset + ROWS, cardN),
                      cardN);
        drawCentered(pg, 14, MUTED, 512);
    }
    else {
        drawBtn("< Назад", 50, 30, 120, 40, GRAY, mouse);
        char title[32];
        std::snprintf(title, 32, "Карточка %d", editIdx + 1);
        drawCentered(title, 28, ACCENT, 80);

        win.draw(txt("Можно несколько вариантов через  /  (напр: его / ему)",
                     13, MUTED, 170, 163));

        auto fieldBg = [&](float y, bool focus) {
            drawRect(170, y, 460, 50, sf::Color(30, 34, 55),
                     focus ? sf::Color(ACCENT.r, ACCENT.g, ACCENT.b, 200)
                           : sf::Color(70, 80, 120));
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

        drawBtn("Сохранить", 270, 400, 260, 50, GREEN, mouse);
    }
}

void FlashcardApp::drawTestScreen(sf::Vector2i mouse) {
    drawBtn("Завершить", 50, 30, 130, 40, RED, mouse);

    drawRect(200, 38, 550, 8, sf::Color(50, 55, 90));
    float filled = deckN > 0 ? (float)deckI / deckN * 550.f : 0.f;
    drawRect(200, 38, filled, 8, ACCENT);
    char pb[32];
    std::snprintf(pb, 32, "%d / %d", deckI + 1, deckN);
    drawCentered(pb, 16, MUTED, 52);

    const float cx = 150, cy = 100, cw = 500, ch = 300;
    drawRect(cx, cy, cw, ch,
             flipped ? sf::Color(40, 65, 55) : CARD,
             flipped ? GREEN : sf::Color(80, 100, 180));
    drawCentered(flipped ? "СЛОВО" : "ПЕРЕВОД", 13,
                 flipped ? GREEN : ACCENT, cy + 18);
    drawCentered(flipped ? deck[deckI].w : deck[deckI].t, 30, TXT, cy + 90);

    if (!flipped)
        drawBtn("Перевернуть", 300, 460, 200, 50, BLUE, mouse);
    else {
        drawBtn("Не знал", 100, 460, 160, 50, RED,   mouse);
        drawBtn("Знал!",   540, 460, 160, 50, GREEN, mouse);
    }
}

void FlashcardApp::drawResultScreen(sf::Vector2i mouse) {
    drawCentered("Результат", 42, ACCENT, 60);
    int answered = deckI;
    int pct = answered > 0 ? correct * 100 / answered : 0;
    char sb[32];
    std::snprintf(sb, 32, "%d / %d", correct, answered);
    drawCentered(sb, 52, pct >= 70 ? GREEN : RED, 130);
    char pb[16];
    std::snprintf(pb, 16, "(%d%%)", pct);
    drawCentered(pb, 24, MUTED, 195);

    const char* msg = pct == 100 ? "Все слова знаешь!"
                    : pct >= 70  ? "Хороший результат!"
                                 : "Стоит повторить.";
    drawCentered(msg, 22, TXT, 240);

    if (missedN > 0) {
        win.draw(txt("Повторить:", 18, RED, 100, 285));
        int n = missedN < 5 ? missedN : 5;
        for (int i = 0; i < n; i++) {
            char line[MWORD * 2 + 4];
            std::snprintf(line, sizeof(line), "%s → %s",
                          missed[i].w, missed[i].t);
            win.draw(txt(line, 15, MUTED, 100, 312 + i * 22.f));
        }
    }
    drawBtn("В меню",  190, 460, 180, 50, GRAY, mouse);
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
    t.setPosition(800 / 2.f - b.width / 2.f - b.left, cy);
    win.draw(t);
}

void FlashcardApp::drawRect(float x, float y, float w, float h,
                             sf::Color fill, sf::Color outline) {
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
    return e.type == sf::Event::MouseButtonPressed
        && e.mouseButton.x >= x && e.mouseButton.x <= x + w
        && e.mouseButton.y >= y && e.mouseButton.y <= y + h;
}

void FlashcardApp::drawBtn(const char* s, float x, float y, float w, float h,
                            sf::Color fill, sf::Vector2i mouse) {
    bool hov = mouse.x >= x && mouse.x <= x + w
            && mouse.y >= y && mouse.y <= y + h;
    drawRect(x, y, w, h,
             hov ? sf::Color(std::min(255, (int)fill.r + 30),
                             std::min(255, (int)fill.g + 30),
                             std::min(255, (int)fill.b + 30))
                 : fill);
    auto t = txt(s, 18, sf::Color::White, 0, 0);
    auto b = t.getLocalBounds();
    t.setPosition(x + w / 2 - b.width / 2 - b.left,
                  y + h / 2 - b.height / 2 - b.top);
    win.draw(t);
}

void FlashcardApp::addUtf8(char* buf, uint32_t code, int maxLen) {
    int len = std::strlen(buf);
    if (len >= maxLen - 4) return;
    if (code < 0x80) {
        buf[len] = code; buf[len+1] = 0;
    } else if (code < 0x800) {
        buf[len]   = 0xC0 | (code >> 6);
        buf[len+1] = 0x80 | (code & 0x3F);
        buf[len+2] = 0;
    } else {
        buf[len]   = 0xE0 | (code >> 12);
        buf[len+1] = 0x80 | ((code >> 6) & 0x3F);
        buf[len+2] = 0x80 | (code & 0x3F);
        buf[len+3] = 0;
    }
}

void FlashcardApp::popUtf8(char* buf) {
    int l = std::strlen(buf);
    if (l == 0) return;
    while (l > 0) {
        l--;
        char c = buf[l];
        buf[l] = '\0';
        if ((c & 0xC0) != 0x80) break;
    }
}

void FlashcardApp::shuffle(Card* a, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = std::rand() % (i + 1);
        Card tmp = a[i]; a[i] = a[j]; a[j] = tmp;
    }
}