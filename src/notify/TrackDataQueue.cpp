#include "TrackDataQueue.h"
#include "dev_tools.h"
#include <stdlib.h>
#include <cstring>

TrackDataQueue* TrackDataQueue::instance = nullptr;

TrackDataQueue::TrackDataQueue() {
    trackDataQueue = xQueueCreate(QUEUE_SIZE, sizeof(Info*));
}

TrackDataQueue::~TrackDataQueue() {
    if (trackDataQueue != nullptr) {
        Info* infoPtr = nullptr;
        while (xQueueReceive(trackDataQueue, &infoPtr, 0) == pdTRUE) {
            if (infoPtr != nullptr) {
                delete infoPtr;
            }
        }
        vQueueDelete(trackDataQueue);
        trackDataQueue = nullptr;
    }
}

TrackDataQueue& TrackDataQueue::getInstance() {
    if (instance == nullptr) {
        instance = new TrackDataQueue();
    }
    return *instance;
}

bool TrackDataQueue::postTrackData(const Info& info) {
    if (trackDataQueue == nullptr) {
        return false;
    }

    Info* infoPtr = new Info();
    *infoPtr = info;

    BaseType_t result = xQueueSend(trackDataQueue, &infoPtr, 0);

    if (result != pdTRUE) {
        delete infoPtr;
        return false;
    }

    return true;
}

bool TrackDataQueue::getTrackData(Info& info) {
    if (trackDataQueue == nullptr) {
        return false;
    }

    Info* infoPtr = nullptr;
    BaseType_t result = xQueueReceive(trackDataQueue, &infoPtr, 0);
    if (result != pdTRUE) {
        return false;
    }

    if (infoPtr == nullptr) {
        return false;
    }

    info = *infoPtr;
    delete infoPtr;

    return true;
}


