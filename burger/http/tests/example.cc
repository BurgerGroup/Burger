#include "burger/http/beef.h"

using namespace burger;
using namespace burger::http;

Beef app;

int main() {
    app.route("/",
    []{
        return "Hello World!";
    });

    app.port(9001).run();
}