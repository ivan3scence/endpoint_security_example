#pragma once
#include <fstream>
#include <string>
#include <mutex>
#include <list>
#include <filesystem>
#include <Logger.h>
#include <events.pb.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/delimited_message_util.h>


namespace fs = std::filesystem;

class Storage {
public:
    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    Storage& operator = (const Storage&) = delete;
    ~Storage() = default;

    static Storage& Locate();
    int SetUp(const std::string& storagePath);
    int SetDown();
    int DumpEvents(const std::list<Event>& events);
    int PrintStorage() const;
    std::list<Event> ReadStorage() const;

private:
    Storage() = default;

    std::mutex    mutex;
    fs::path      path;
    std::ofstream storageOs;
    std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> zeroCopyOs;
};
