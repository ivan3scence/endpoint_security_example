#include <ESClient.h>


int ESClient::setCommonInfo(Event& message, const es_message_t *msg) {
    if (!msg->process || !msg->process->executable
        || !msg->process->executable->path.data) {
        LOG_ERROR("Got empty process path!\n");
        return -1;
    }
    message.set_process_path(std::string(msg->process->executable->path.data,
                             msg->process->executable->path.length));

    LOG_INFO("proc: %s\n", message.process_path().c_str());
    return 0;
};

template<es_event_type_t EventType>
int ESClient::setPrivateInfo(Event &, const es_message_t *) {
    LOG_ERROR("Got unsupported event: %d\n", EventType);
    return -1;
};

template<>
int ESClient::setPrivateInfo<ES_EVENT_TYPE_NOTIFY_EXEC>(Event &message, const es_message_t *msg) {
    const es_event_exec_t *exec = &msg->event.exec;
    if (!exec->target->executable || !exec->target->executable->path.data) {
        LOG_ERROR("Got empty exec event!\n");
        return -1;
    }

    ProcessExec pe;
    pe.set_file_path(std::string(exec->target->executable->path.data,
                                 exec->target->executable->path.length));
    *message.mutable_process_exec() = pe;

    LOG_INFO("exec: %s\n", message.process_exec().file_path().c_str());

    return 0;
};

template<>
int ESClient::setPrivateInfo<ES_EVENT_TYPE_NOTIFY_WRITE>(Event &message, const es_message_t *msg) {
    const es_event_write_t *write = &msg->event.write;
    if (!write->target || !write->target->path.data) {
        LOG_ERROR("Got empty write event!\n");
        return -1;
    }

    FileOpen fo;
    fo.set_file_path(std::string(write->target->path.data,
                                 write->target->path.length));
    *message.mutable_file_open() = fo;

    LOG_INFO("opened(writing): %s\n", message.file_open().file_path().c_str());

    return 0;
};

template<>
int ESClient::setPrivateInfo<ES_EVENT_TYPE_NOTIFY_CREATE>(Event &message, const es_message_t *msg) {
    es_string_token_t const *path = nullptr;
    const es_event_create_t *create = &msg->event.create;

    switch(create->destination_type) {
    case ES_DESTINATION_TYPE_EXISTING_FILE: {
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
    *message.mutable_file_created() = fc;

    LOG_INFO("created: %s\n", message.file_created().file_path().c_str());

    return 0;
};

template<>
int ESClient::setPrivateInfo<ES_EVENT_TYPE_NOTIFY_RENAME>(Event &message, const es_message_t *msg) {
    es_string_token_t const *path = nullptr;
    const es_event_rename_t *rename = &msg->event.rename;

    switch(rename->destination_type) {
    case ES_DESTINATION_TYPE_EXISTING_FILE: {
        path = &rename->destination.existing_file->path;
        break;
    }
    case ES_DESTINATION_TYPE_NEW_PATH: {
        path = &rename->destination.new_path.filename;
        break;
    }
    default: {
        LOG_ERROR("Got wrong destination type!\n");
        return -1;
    }
    }

    if (!path || !path->data) {
        LOG_ERROR("Got empty rename event!\n");
        return -1;
    }

    FileRename fr;
    fr.set_file_path(std::string(path->data, path->length));
    *message.mutable_file_rename() = fr;

    LOG_INFO("rename: %s\n", message.file_rename().file_path().c_str());

    return 0;
};

template<>
int ESClient::setPrivateInfo<ES_EVENT_TYPE_NOTIFY_CLONE>(Event &message, const es_message_t *msg) {
    const es_event_clone_t *clone = &msg->event.clone;
    if (!clone->target_name.data) {
        LOG_ERROR("Got empty clone event!\n");
        return -1;
    }

    FileClone fc;
    fc.set_file_path(std::string(clone->target_name.data,
                                 clone->target_name.length));
    *message.mutable_file_clone() = fc;

    LOG_INFO("cloned: %s\n", message.file_clone().file_path().c_str());

    return 0;
};

template<>
int ESClient::setPrivateInfo<ES_EVENT_TYPE_NOTIFY_COPYFILE>(Event &message, const es_message_t *msg) {
    const es_event_copyfile_t *copyfile = &msg->event.copyfile;
    if (!copyfile->target_name.data) {
        LOG_ERROR("Got empty copyfile event!\n");
        return -1;
    }

    FileCopy fc;
    fc.set_file_path(std::string(copyfile->target_name.data,
                                 copyfile->target_name.length));
    *message.mutable_file_copy() = fc;

    LOG_INFO("copied: %s\n", message.file_copy().file_path().c_str());

    return 0;
};

Event ESClient::parseMessage(const es_message_t *msg) {
    Event message;
    if (!msg || setCommonInfo(message, msg)) return {};

    switch(msg->event_type) {
    case ES_EVENT_TYPE_NOTIFY_EXEC: {
        setPrivateInfo<ES_EVENT_TYPE_NOTIFY_EXEC>(message, msg);
        break;
    }
    case ES_EVENT_TYPE_NOTIFY_WRITE: {
        setPrivateInfo<ES_EVENT_TYPE_NOTIFY_WRITE>(message, msg);
        break;
    }
    case ES_EVENT_TYPE_NOTIFY_CREATE: {
        setPrivateInfo<ES_EVENT_TYPE_NOTIFY_CREATE>(message, msg);
        break;
    }
    case ES_EVENT_TYPE_NOTIFY_RENAME: {
        setPrivateInfo<ES_EVENT_TYPE_NOTIFY_RENAME>(message, msg);
        break;
    }
    case ES_EVENT_TYPE_NOTIFY_CLONE: {
        setPrivateInfo<ES_EVENT_TYPE_NOTIFY_CLONE>(message, msg);
        break;
    }
    case ES_EVENT_TYPE_NOTIFY_COPYFILE: {
        setPrivateInfo<ES_EVENT_TYPE_NOTIFY_COPYFILE>(message, msg);
        break;
    }
    default: {
        LOG_ERROR("Got unsupported event: %d\n", msg->event_type);
        return {};
    }
    }
    return message;
}
