#include "FlashcardApp.h"

int main() {
    FlashcardApp* app = new FlashcardApp();
    app->initAndRun();
    delete app;
    return 0;
}