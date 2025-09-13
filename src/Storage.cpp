#include <Storage.h>
#include <nlohmann/json.hpp>
#include <google/protobuf/message_lite.h>

inline void to_json(nlohmann::json &j, const Event &e) {
    j["process_path"] = e.process_path();

    switch (e.data_case()) {
        case Event::kProcessExec:
            j.push_back({"process_exec", {{"file_path", e.process_exec().file_path()}}});
            break;
        case Event::kFileOpen:
            j.push_back({"file_open", {{"file_path", e.file_open().file_path()}}});
            break;
        case Event::kFileCreated:
            j.push_back({"file_created", {{"file_path", e.file_created().file_path()}}});
            break;
        case Event::kFileRename:
            j.push_back({"file_rename", {{"file_path", e.file_rename().file_path()}}});
            break;
        case Event::kFileClone:
            j.push_back({"file_clone", {{"file_path", e.file_clone().file_path()}}});
            break;
        case Event::kFileCopy:
            j.push_back({"file_copy", {{"file_path", e.file_copy().file_path()}}});
            break;
        default:
            std::cout << "No data set" << std::endl;
            break;
    }
}

// inline void from_json(const nlohmann::json &j, Event &e) {
//     s.setId((j.at("id").get<int>()));
//     s.setName(j.at("name").get<std::string>());
// }

Storage& Storage::Locate() {
    static Storage instance;
    return instance;
}

int Storage::SetUp(const std::string& storagePath) {
    if (storageOs.is_open()) {
        LOG_ERROR("Storage was inited already!\n");
        return -1;
    }

    storageOs.open(storagePath, std::ios::out | std::ios::binary | std::ios::app);
    if (!storageOs) {
        LOG_ERROR("Failed to open file: %s\n", storagePath.c_str());
        return -1;
    }

    raw_output = std::make_unique<google::protobuf::io::OstreamOutputStream>(&storageOs);
    coded_output = std::make_unique<google::protobuf::io::CodedOutputStream>(raw_output.get());
    path = fs::path(storagePath);

    return 0;
};

int Storage::SetDown() {
    storageOs.close();
    fs::permissions(path,
                    fs::perms::owner_all | fs::perms::group_all
                    | fs::perms::others_read, fs::perm_options::add);
    return 0;
};

int Storage::DumpEvents(const std::list<Event>& messages) {
    std::lock_guard l(mutex);

    for (const auto& m : messages) {
        coded_output->WriteVarint32(m.ByteSizeLong());
        m.SerializeToCodedStream(coded_output.get());
    }

    return 0;
};

std::list<Event> Storage::ReadStorage() const {
    std::list<Event> messages;
    std::ifstream input(path, std::ios::binary);

    if (!input.is_open()) {
        LOG_ERROR("Failed to open storage file: %s!\n", path.c_str());
        return {};
    }

    google::protobuf::io::IstreamInputStream raw_input(&input);
    google::protobuf::io::CodedInputStream coded_input(&raw_input);

    uint32_t message_size;
    while (coded_input.ReadVarint32(&message_size)) {
        auto limit = coded_input.PushLimit(message_size);

        Event message;
        if (message.ParseFromCodedStream(&coded_input)) {
            messages.push_back(std::move(message));
        } else {
            LOG_ERROR("Failed to parse message!\n");
            break;
        }

        coded_input.PopLimit(limit);
    }

    return messages;
}

int Storage::PrintStorage() const {
    std::list<Event> list = ReadStorage();

    if (list.size() == 0) {
        return -1;
    }

    for (const auto& event : list) {
        nlohmann::json j(event);
        LOG_INFO("%s\n", j.dump().c_str());
    }

    return 0;
}
