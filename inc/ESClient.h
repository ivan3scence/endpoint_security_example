#pragma once
#include <iostream>
#include <exception>
#include <vector>
#include <mutex>
#include <unistd.h>
#include <EndpointSecurity/EndpointSecurity.h>
#include <events.pb.h>
#include <Logger.h>
#include <Storage.h>


class ESClient {
public:
    ESClient(const es_event_type_t type) :
             client(nullptr), collectingType(type) {
        auto res = es_new_client(&client, msgHandler);

        if (res != ES_NEW_CLIENT_RESULT_SUCCESS) {
            switch(res) {
            case ES_NEW_CLIENT_RESULT_ERR_NOT_ENTITLED:
                throw std::runtime_error(
                      "Application requires 'com.apple.developer"
                      ".endpoint-security.client' entitlement");
            case ES_NEW_CLIENT_RESULT_ERR_NOT_PERMITTED:
                throw std::runtime_error(
                      "Application lacks Transparency, Consent, and "
                      "Control (TCC) approval from the user. "
                      "This can be resolved by granting 'Full Disk Access' "
                      "from the 'Security & Privacy' "
                      "tab of System Preferences.");
            case ES_NEW_CLIENT_RESULT_ERR_NOT_PRIVILEGED:
                throw std::runtime_error("Application needs to be run as root");
            default:
                throw std::runtime_error(
                    "es_new_client: " + std::to_string(res)
                );
            }
        }

        auto resCache = es_clear_cache(client);
        if (resCache != ES_CLEAR_CACHE_RESULT_SUCCESS) {
            throw std::runtime_error("es_clear_cache: "
                                     + std::to_string(resCache));
        }

        es_subscribe(client, &collectingType, 1);
        LOG_INFO("Listening for ESF %d\n", collectingType);
    };

    ~ESClient() {
        LOG_INFO("destructing ESF type = %d\n", collectingType);

        es_unsubscribe(client, &collectingType, 1);
        es_delete_client(client);
        client = nullptr;

        std::lock_guard l(mutex);
        if (localStorage.size()) {
            Storage::Locate().DumpEvents(localStorage);
        }
    };

private:
    std::mutex              mutex;
    es_client_t             *client;
    es_event_type_t         collectingType;
    static constexpr size_t CLIENT_STORAGE_SIZE = 512;
    std::list<Event>        localStorage;

    Event parseMessage(const es_message_t *msg);
    template<es_event_type_t EventType>
    int setPrivateInfo(Event &, const es_message_t *);
    int setCommonInfo(Event& message, const es_message_t *msg);

    es_handler_block_t      msgHandler =
                                     ^(es_client_t *, const es_message_t *msg) {
        es_retain_message(msg);
        if (!msg) {
            LOG_ERROR("Got empty message!\n");
            return;
        }
        Event message = parseMessage(msg);
        es_release_message(msg);

        if (!message.ByteSizeLong()) return;

        std::lock_guard l(mutex);
        if (localStorage.size() == CLIENT_STORAGE_SIZE) {
            if (!Storage::Locate().DumpEvents(localStorage)) {
                localStorage.clear();
            }
        }
        localStorage.push_back(std::move(message));
    };
};
