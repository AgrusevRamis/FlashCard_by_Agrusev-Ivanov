#pragma once

const int W = 800;
const int H = 600;
const int MAX = 200;
const int MWORD = 100;

struct Card {
    char w[MWORD];
    char t[MWORD];
};

enum Screen { MENU, ADD, TEST, RESULT };
