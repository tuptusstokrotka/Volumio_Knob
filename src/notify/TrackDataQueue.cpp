#include "TrackDataQueue.h"
#include "dev_tools.h"
#include <stdlib.h>
#include <cstring>

TrackDataQueue* TrackDataQueue::instance = nullptr;

TrackDataQueue::TrackDataQueue() {
    // Queue stores pointers to Info (heap-allocated)
    trackDataQueue = xQueueCreate(QUEUE_SIZE, sizeof(Info*));
    if (trackDataQueue == nullptr) {
        DEBUG_PRINTLN("[TrackDataQueue] Failed to create track data queue");
    }
}

TrackDataQueue::~TrackDataQueue() {
    if (trackDataQueue != nullptr) {
        // Drain any remaining track data
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

    // Allocate Info on heap and copy data
    Info* infoPtr = new Info();
    *infoPtr = info;  // Copy assignment (std::string will be copied properly)

    BaseType_t result = xQueueSend(trackDataQueue, &infoPtr, 0);

    if (result != pdTRUE) {
        DEBUG_PRINTLN("[TrackDataQueue] Queue full, track data dropped");
        delete infoPtr;  // Free if queue is full
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
        DEBUG_PRINTLN("[TrackDataQueue] Queue empty, no track data available");
        return false;
    }

    if (infoPtr == nullptr) {
        DEBUG_PRINTLN("[TrackDataQueue] Received null pointer");
        return false;
    }

    // Copy data from heap to output parameter
    info = *infoPtr;
    
    // Free the heap-allocated Info
    delete infoPtr;

    return true;
}


