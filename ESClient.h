#pragma once
#include <iostream>
#include <exception>
#include <unistd.h>
#include <EndpointSecurity/EndpointSecurity.h>
#include "ESEvent.h"



class ESClient {
private:
    es_client_t     *client;
    es_event_type_t collectingType;
    es_handler_block_t msgHandler = ^(es_client_t *cl, const es_message_t *msg) {
        es_retain_message(msg);

        if (!msg) {
            LOG_ERROR("Got empty message!\n");
            return;
        }
        ESEvent event(msg);

        es_release_message(msg);

    };

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
    };
};
