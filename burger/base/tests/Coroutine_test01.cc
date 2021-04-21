#include "burger/base/Log.h"
#include "burger/base/Coroutine.h"

#include <vector>

using namespace burger;

static int sum = 0;

void test() {
    DEBUG("in Coroutine( {} )", Coroutine::GetCoId());
	Coroutine::SwapOut();
	sum++;
    DEBUG("in Coroutine( {} )", Coroutine::GetCoId());
	Coroutine::SwapOut();
}

int main() {
    LOGGER(); LOG_LEVEL_DEBUG;
	const int sz = 5;
	std::vector<Coroutine::ptr> coroutines;
	Coroutine::GetCurCo();
	for (int i = 0; i < sz; ++i) {
		coroutines.push_back(std::make_shared<Coroutine>(test));
	}

	for (int i = 0; i < sz; ++i) {
		coroutines[i]->swapIn();
        DEBUG("back to main Coroutine( {} ) ", Coroutine::GetCoId());
	}
	for (int i = 0; i < sz; ++i) {
		coroutines[i]->swapIn();
        DEBUG("back to main Coroutine( {} ) ", Coroutine::GetCoId());
	}
	for (int i = 0; i < sz; ++i) {
		coroutines[i]->swapIn();
        DEBUG("back to main Coroutine( {} ) ", Coroutine::GetCoId());
	}
    DEBUG("All coroutine terminated, sum = {}", sum);

	return 0;
}