#include "interface.h"


InterFace::InterFace() {
    initscr();
    curs_set(0);
}

InterFace::~InterFace() {
    delwin(header_);
    delwin(output_);
    delwin(input_);
    delwin(onlinelist_);
    endwin();
}

void InterFace::createHeader() {
    int y = 0;
    int x = 0;
    int h = LINES / 5;
    int w = COLS;
    header_ = createNewInterFace(h, w, y, x);
}

void InterFace::createOutput() {
    int y = LINES / 5;
    int x = 0;
    int h = LINES * 3 / 5;
    int w = COLS * 2 / 3;
    output_ = createNewInterFace(h, w, y, x);
}

void InterFace::createInput() {
    int y = LINES * 4 / 5;
    int x = 0;
    int h = LINES / 5;
    int w = COLS;
    input_ = createNewInterFace(h, w, y, x);
}

void InterFace::createOnlineList() {
    int y = LINES / 5;
    int x = COLS * 2 / 3;
    int h = LINES * 3 / 5;
    int w = COLS * 1 / 3;
    onlinelist_ = createNewInterFace(h, w, y, x);
}

std::string InterFace::getStrFromInterFace(WINDOW* w) {
    char buf[1024] = {0};
    wgetnstr(w, buf, sizeof(buf));
    return buf;
}

void InterFace::putStrToInterFace(WINDOW* w, int startY, int startX, const std::string& message) {
    mvwaddstr(w, startY, startX, message.c_str());
}

WINDOW* InterFace::createNewInterFace(int h, int w, int startY, int startX) {
    WINDOW* local = newwin(h, w, startY, startX);
    box(local, 0, 0);
    return local;
}

void InterFace::clearInterFace(WINDOW* w, int begin, int line) {
    while (line-- > 0) {
        wmove(w, begin++, 0);
        wclrtoeol(w);
    }
}


