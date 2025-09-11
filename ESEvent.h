#pragma once

#include <fcntl.h>
#include <EndpointSecurity/EndpointSecurity.h>
#include <events.pb.h>

class ESEvent {
private:
    int setCommonInfo(const es_message_t *msg) {
        if (!msg->process || !msg->process->executable
            || !msg->process->executable->path.data) {
            LOG_ERROR("Got empty process path!\n");
            return -1;
        }
        proto.set_process_path(std::string(msg->process->executable->path.data,
                                 msg->process->executable->path.length));

        // LOG_INFO("proc: %s\n", proto.process_path().c_str());
        return 0;
    };

    template<es_event_type_t EventType>
    int setPrivateInfo(const es_message_t *msg) {
        LOG_ERROR("Got unsupported event: %d\n", EventType);
        return -1;
    };
    template<>
    int setPrivateInfo<ES_EVENT_TYPE_NOTIFY_EXEC>(const es_message_t *msg) {
        const es_event_exec_t *exec = &msg->event.exec;
        if (!exec->target->executable || !exec->target->executable->path.data) {
            LOG_ERROR("Got empty exec event!\n");
            return -1;
        }

        ProcessExec pe;
        pe.set_file_path(std::string(exec->target->executable->path.data,
                                     exec->target->executable->path.length));
        *proto.mutable_process_exec() = pe;

        // LOG_INFO("exec: %s\n", proto.process_exec().file_path().c_str());

        return 0;
    };
    template<>
    int setPrivateInfo<ES_EVENT_TYPE_NOTIFY_OPEN>(const es_message_t *msg) {
        const es_event_open_t *open = &msg->event.open;
        if (!open->file || !open->file->path.data) {
            LOG_ERROR("Got empty open event!\n");
            return -1;
        }

        if ((open->fflag & FWRITE) != FWRITE) {
            return 0;
        }

        FileOpen fo;
        fo.set_file_path(std::string(open->file->path.data,
                                     open->file->path.length));
        *proto.mutable_file_open() = fo;

        LOG_INFO("opened: %s\n", proto.file_open().file_path().c_str());

        return 0;
    };
    template<>
    int setPrivateInfo<ES_EVENT_TYPE_NOTIFY_CREATE>(const es_message_t *msg) {
        es_string_token_t const *path = nullptr;
        const es_event_create_t *create = &msg->event.create;

        switch(create->destination_type) {
        case ES_DESTINATION_TYPE_EXISTING_FILE: {         // opening for writing
            path = &create->destination.existing_file->path;
            break;
        }
        case ES_DESTINATION_TYPE_NEW_PATH: {
            path = &create->destination.new_path.filename;
            break;
        }
        default: {
            LOG_ERROR("Got wrong destination type!\n");
            return -1;
        }
        }

        if (!path || !path->data) {
            LOG_ERROR("Got empty create event!\n");
            return -1;
        }

        FileCreated fc;
        fc.set_file_path(std::string(path->data, path->length));
        *proto.mutable_file_created() = fc;

        LOG_INFO("create: %s\n", proto.file_created().file_path().c_str());

        return 0;
    };

    Event proto;
public:
    ESEvent(const es_message_t *msg) {
        if (!msg || setCommonInfo(msg)) return;

        switch(msg->event_type) {
        case ES_EVENT_TYPE_NOTIFY_EXEC: {
            setPrivateInfo<ES_EVENT_TYPE_NOTIFY_EXEC>(msg);
            break;
        }
        case ES_EVENT_TYPE_NOTIFY_OPEN: {
            setPrivateInfo<ES_EVENT_TYPE_NOTIFY_OPEN>(msg);
            break;
        }
        case ES_EVENT_TYPE_NOTIFY_CREATE: {
            setPrivateInfo<ES_EVENT_TYPE_NOTIFY_OPEN>(msg);
            break;
        }
        default: {
            LOG_ERROR("Got unsupported event: %d\n", msg->event_type);
            return;
        }
        }
    };
};
