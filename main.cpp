#include <thread>
#include <atomic>
#include <memory>
#include <list>

#define LOG_ERROR(...) dprintf(STDERR_FILENO, __VA_ARGS__)
#define LOG_INFO(...) printf(__VA_ARGS__)

#include "ESClient.h"

static std::atomic_bool working = true;

const static es_event_type_t events[] = {
    ES_EVENT_TYPE_NOTIFY_EXEC,
    ES_EVENT_TYPE_NOTIFY_OPEN,
    ES_EVENT_TYPE_NOTIFY_CREATE
};

void sigHandler(int) {
    working = false;
}

int main() {
    signal(SIGINT, &sigHandler);
    std::list<std::unique_ptr<ESClient>> clients;
    for (const auto e : events) {
        clients.push_back(std::make_unique<ESClient>(e));
    }


    while (working) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
