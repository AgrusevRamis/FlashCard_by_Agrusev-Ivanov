#include "FlashcardApp.h"

int main() {
    FlashcardApp app;
    if (!app.initAndRun()) {
        return 1;
    }
    return 0;
}
