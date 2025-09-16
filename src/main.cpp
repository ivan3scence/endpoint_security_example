#include <thread>
#include <atomic>
#include <memory>
#include <list>
#include <nlohmann/json.hpp>

#include <Logger.h>
#include <ESClient.h>
#include <Storage.h>

static std::atomic_bool working = true;

static constexpr es_event_type_t events[] = {
    ES_EVENT_TYPE_NOTIFY_EXEC,
    ES_EVENT_TYPE_NOTIFY_WRITE,
    ES_EVENT_TYPE_NOTIFY_CREATE,
    ES_EVENT_TYPE_NOTIFY_RENAME,
    ES_EVENT_TYPE_NOTIFY_COPYFILE,
    ES_EVENT_TYPE_NOTIFY_CLONE
};

void sigHandler(int) {
    working = false;
}

int printUsage(char *progName, int code) {
    std::cerr << "Usage: " << progName << " [Options] STORAGE_PATH\n\n"
              << "\tSTORAGE_PATH - path to db file where to store protobuf"
                 "messages or which to print out.\n\n"
              << "Options:\n"
              << "\t-h,--help\t\tShow this message\n"
              << "\t-p,--print\t\tPrint out storage file\n"
              << "\t-v,--verboose\t\tEnable logging\n";

    return code;
}

int main(int argc, char **argv) {
    bool isPrintOut;
    Logger::LogLevel logLevel = Logger::ERROR;
    std::string storagePath;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            return printUsage(argv[0], 0);
        } else if ((arg == "-v") || (arg == "--verboose")) {
            logLevel = Logger::TRACE;
        } else if ((arg == "-p" || arg == "--print")) {
            if (i + 1 == argc)
                return printUsage(argv[0], 1);
            storagePath = argv[i + 1];
            logLevel = Logger::INFO;
            isPrintOut = true;
            break;
        } else {
            if (storagePath.length())
                return printUsage(argv[0], 1);
            storagePath = arg;
        }
    }

    if (!storagePath.length())
        return printUsage(argv[0], 1);

    Logger::Locate().SetLogLevel(logLevel);

    if (Storage::Locate().SetUp(storagePath) == -1) {
        LOG_ERROR("Failed to set up storage file: %s!\n", storagePath.c_str());
        return 1;
    }

    if (isPrintOut) {
        return Storage::Locate().PrintStorage();
    }

    signal(SIGINT, &sigHandler);

    {
        std::list<std::unique_ptr<ESClient>> clients;
        try {
            for (const auto e : events) {
                clients.push_back(std::make_unique<ESClient>(e));
            }
        } catch (std::exception& ex) {
            LOG_ERROR("\nCaught exception while ESF client startup: %s!\n",
                      ex.what());
            return 1;
        }

        while (working) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    Storage::Locate().SetDown();

    return 0;
}
