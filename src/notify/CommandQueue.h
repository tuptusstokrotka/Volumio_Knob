#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <string>

/**
 * @brief Volumio command types
 */
enum class VolumioCommandType {
    PLAY,
    PAUSE,
    TOGGLE,
    NEXT,
    PREV,
    SEEK,
    RANDOM,
    REPEAT
};

/**
 * @brief Command structure for Volumio commands
 */
struct VolumioCommand {
    VolumioCommandType type;
    int value;  // Used for volume, seek, random, repeat values
};

/**
 * @brief Thread-safe command queue for Volumio commands
 *
 * Dashboard (display task) posts commands without knowing about WiFiHandler.
 * WiFiHandler (WiFi task) processes commands from the queue.
 */
class CommandQueue {
private:
    QueueHandle_t commandQueue;
    static constexpr size_t QUEUE_SIZE = 20;

    static CommandQueue* instance;
    CommandQueue();

    VolumioCommand* createCommandCopy(const VolumioCommand& cmd);

public:
    ~CommandQueue();

    CommandQueue(const CommandQueue&) = delete;
    void operator=(const CommandQueue&) = delete;

    static CommandQueue& getInstance();

    /**
     * @brief Post a command to the queue (non-blocking, thread-safe)
     * @param cmd Command to post
     * @return true if successfully posted, false if queue is full
     */
    bool postCommand(const VolumioCommand& cmd);

    /**
     * @brief Get next command from queue (call this in a loop from WiFiHandler task)
     * @param cmd Output parameter for the command
     * @return true if command was retrieved, false if queue is empty
     */
    bool getNextCommand(VolumioCommand& cmd);
};

#endif // COMMAND_QUEUE_H

