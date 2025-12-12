#ifndef TRACK_DATA_QUEUE_H
#define TRACK_DATA_QUEUE_H

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "volumio/volumio_trackdata.h"

class TrackDataQueue {
private:
    QueueHandle_t trackDataQueue;
    static constexpr size_t QUEUE_SIZE = 10;

    static TrackDataQueue* instance;
    TrackDataQueue();

public:
    ~TrackDataQueue();

    TrackDataQueue(const TrackDataQueue&) = delete;
    void operator=(const TrackDataQueue&) = delete;

    static TrackDataQueue& getInstance();

    bool postTrackData(const Info& info);
    bool getTrackData(Info& info);
};

#endif // TRACK_DATA_QUEUE_H


