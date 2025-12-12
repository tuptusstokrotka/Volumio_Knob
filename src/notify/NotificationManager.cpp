#include "NotificationManager.h"

NotificationManager* NotificationManager::instance = nullptr;

NotificationManager::NotificationManager() {
    eventQueue = xQueueCreate(QUEUE_SIZE, sizeof(NotificationEvent*));
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

void NotificationManager::subscribe(std::function<void(const NotificationEvent&)> callback) {
    notificationCallback = callback;
}

bool NotificationManager::postNotification(const std::string& title, const std::string& content, TickType_t duration_ms) {
    if (eventQueue == nullptr) {
        return false;
    }

    NotificationEvent* eventPtr = new NotificationEvent();
    eventPtr->title = title;
    eventPtr->content = content;
    eventPtr->duration_ms = duration_ms;

    BaseType_t result = xQueueSend(eventQueue, &eventPtr, 0);

    if (result != pdTRUE) {
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

    while (xQueueReceive(eventQueue, &eventPtr, 0) == pdTRUE) {
        if (eventPtr != nullptr) {
            notificationCallback(*eventPtr);
            delete eventPtr;
            eventPtr = nullptr;
        }
    }
}
