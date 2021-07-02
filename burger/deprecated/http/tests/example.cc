#include "burger/http/beef.h"
#include "burger/base/Log.h"

using namespace burger;
using namespace burger::http;

Beef app;

int main() {
    LOGGER();
    app.route("/",
    []{
        return "Hello World!";
    });

    app.port(9001).run();
}