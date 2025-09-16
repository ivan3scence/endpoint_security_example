#include <Storage.h>
#include <fcntl.h>
#include <nlohmann/json.hpp>

inline void to_json(nlohmann::json &j, const Event &e) {
    try {
        switch (e.data_case()) {
        case Event::kProcessExec: {
            j = {{"process_exec", {{"file_path", e.process_exec().file_path()}}}};
            break;
        }
        case Event::kFileOpen: {
            j = {{"file_open", {{"file_path", e.file_open().file_path()}}}};
            break;
        }
        case Event::kFileCreated: {
            j = {{"file_created", {{"file_path", e.file_created().file_path()}}}};
            break;
        }
        case Event::kFileRename: {
            j = {{"file_rename", {{"file_path", e.file_rename().file_path()}}}};
            break;
        }
        case Event::kFileClone: {
            j = {{"file_clone", {{"file_path", e.file_clone().file_path()}}}};
            break;
        }
        case Event::kFileCopy: {
            j = {{"file_copy", {{"file_path", e.file_copy().file_path()}}}};
            break;
        }
        default: {
            std::cout << "No data set" << std::endl;
            return;
        }
    }
        j["process_path"] = e.process_path();
    } catch (std::exception& ex) {
        LOG_ERROR("Caught json exception: %s!\n", ex.what())
    }
}

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
        LOG_ERROR("Failed to open file: %s!\n", storagePath.c_str());
        return -1;
    }

    zeroCopyOs = std::make_unique<google::protobuf::io::OstreamOutputStream>(&storageOs);

    try {
        path = fs::path(storagePath);
    } catch (std::exception& ex) {
        LOG_ERROR("Failed to make path for storage: %s!\n", ex.what());
        return -1;
    }

    return 0;
};

int Storage::SetDown() {
    if (zeroCopyOs) {
        zeroCopyOs.reset();
    }

    storageOs.close();
    fs::permissions(path,
                    fs::perms::owner_all | fs::perms::group_all
                    | fs::perms::others_read, fs::perm_options::add);
    return 0;
};

int Storage::DumpEvents(const std::list<Event>& messages) {
    std::lock_guard l(mutex);

    if (!storageOs.is_open()) {
        LOG_ERROR("Set up storage before writing!");
        return -1;
    }

    for (const auto& m : messages) {
        if (!google::protobuf::util::SerializeDelimitedToZeroCopyStream(m, zeroCopyOs.get())) {
            LOG_ERROR("Failed to write message to file!");
            continue;
        }
    }

    return 0;
};

std::list<Event> Storage::ReadStorage() const {
    bool clean_eof;
    std::list<Event> messages;

    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input.is_open()) {
        LOG_ERROR("Failed to open storage file: %s!\n", path.c_str());
        return {};
    }

    google::protobuf::io::ZeroCopyInputStream *zeroCopyIs = new google::protobuf::io::IstreamInputStream(&input);

    while (1) {
        Event m;
        if (google::protobuf::util::ParseDelimitedFromZeroCopyStream(&m, zeroCopyIs, &clean_eof)) {
            messages.push_back(m);
        } else {
            if (!clean_eof) {
                LOG_ERROR("Failed to read message from file!\n");
            }
            break;
        }
    }

    delete zeroCopyIs;

    return messages;
}

int Storage::PrintStorage() const {
    std::list<Event> list = ReadStorage();

    if (list.size() == 0) {
        LOG_ERROR("Empty storage!\n");
        return -1;
    }

    for (const auto& event : list) {
        nlohmann::json j(event);
        LOG_INFO("%s\n", j.dump(2).c_str());
    }

    return 0;
}
