#pragma once

static const int MAX   = 2048;
static const int MWORD = 128;
static const int W     = 800;
static const int H     = 560;

struct Card {
    char w[MWORD];
    char t[MWORD];
};

enum Screen { MENU, FILE_LOAD, ADD, EDIT_LIST, TEST, RESULT };