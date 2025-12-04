#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <string>
#include <functional>


struct NotificationEvent {
    std::string title;
    std::string content;
    TickType_t duration_ms;
};

class NotificationManager {
private:
    QueueHandle_t eventQueue;
    static constexpr size_t QUEUE_SIZE = 10;

    static NotificationManager* instance;
    NotificationManager();

    std::function<void(const NotificationEvent&)> notificationCallback;

    NotificationEvent* createEventCopy(const NotificationEvent& event);

public:
    ~NotificationManager();

    NotificationManager(const NotificationManager&) = delete;
    void operator=(const NotificationManager&) = delete;

    static NotificationManager& getInstance();

    /**
     * @brief Subscribe to notifications
     * @param callback Function to call when a notification is received
     */
    void subscribe(std::function<void(const NotificationEvent&)> callback);

    /**
     * @brief Post a notification event to the queue
     * @param title Notification title
     * @param content Notification content
     * @param duration_ms Duration in milliseconds (0 = persistent, no timeout
     */
    bool postNotification(const std::string& title, const std::string& content, TickType_t duration_ms = 5000);

    /**
     * @brief Process pending notifications (call this periodically from DisplayHandler task)
     */
    void processNotifications();
};

#endif // NOTIFICATION_MANAGER_H

