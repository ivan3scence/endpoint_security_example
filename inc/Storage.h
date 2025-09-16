/**
 * @file Storage.h
 * @brief Header file for Storage class
 */

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

/**
 * @class Storage
 * @brief A class for using disk storage for ESF events.
 *
 * This class provides API for dumping events to disk
 * and for reading disk storage. This class is a singleton,
 * use Locate method to work with it.
 *
 */
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
