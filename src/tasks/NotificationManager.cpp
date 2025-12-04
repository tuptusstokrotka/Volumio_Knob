#include "NotificationManager.h"
#include "dev_tools.h"
#include <stdlib.h>

NotificationManager* NotificationManager::instance = nullptr;

NotificationManager::NotificationManager() {
    eventQueue = xQueueCreate(QUEUE_SIZE, sizeof(NotificationEvent*));
    if (eventQueue == nullptr) {
        DEBUG_PRINTLN("[NotificationManager] Failed to create event queue");
    }
}

NotificationManager::~NotificationManager() {
    if (eventQueue != nullptr) {
        vQueueDelete(eventQueue);
        eventQueue = nullptr;
    }
}

NotificationManager& NotificationManager::getInstance() {
    if (instance == nullptr) {
        instance = new NotificationManager();
    }
    return *instance;
}

NotificationEvent* NotificationManager::createEventCopy(const NotificationEvent& event) {
    NotificationEvent* copy = new NotificationEvent();
    if (copy != nullptr) {
        copy->title = event.title;
        copy->content = event.content;
        copy->duration_ms = event.duration_ms;
    }
    return copy;
}

void NotificationManager::subscribe(std::function<void(const NotificationEvent&)> callback) {
    notificationCallback = callback;
}

bool NotificationManager::postNotification(const std::string& title, const std::string& content, TickType_t duration_ms) {
    if (eventQueue == nullptr) {
        return false;
    }

    DEBUG_PRINTLN("[NotificationManager] Posting notification: " << title << " - " << content << " duration: " << duration_ms);

    NotificationEvent event = { title, content, duration_ms };

    // Create heap-allocated copy for the queue
    NotificationEvent* eventPtr = createEventCopy(event);
    if (eventPtr == nullptr) {
        DEBUG_PRINTLN("[NotificationManager] Failed to allocate event");
        return false;
    }

    // Queue the pointer (non-blocking)
    BaseType_t result = xQueueSend(eventQueue, &eventPtr, 0);

    if (result != pdTRUE) {
        DEBUG_PRINTLN("[NotificationManager] Queue full, notification dropped");
        delete eventPtr;
        return false;
    }
    return true;
}

void NotificationManager::processNotifications() {
    if (eventQueue == nullptr || !notificationCallback) {
        return;
    }

    NotificationEvent* eventPtr = nullptr;

    // Process all pending notifications
    while (xQueueReceive(eventQueue, &eventPtr, 0) == pdTRUE) {
        if (eventPtr != nullptr) {
            // Call callback with the event
            notificationCallback(*eventPtr);

            // Free the heap-allocated event
            delete eventPtr;
            eventPtr = nullptr;
        }
    }
}
