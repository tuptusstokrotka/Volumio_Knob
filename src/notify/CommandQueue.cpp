#include "CommandQueue.h"
#include "dev_tools.h"
#include <functional>
#include <stdlib.h>

CommandQueue* CommandQueue::instance = nullptr;

CommandQueue::CommandQueue() {
    // Queue stores pointers to VolumioCommand (heap-allocated)
    commandQueue = xQueueCreate(QUEUE_SIZE, sizeof(VolumioCommand*));
    if (commandQueue == nullptr) {
        DEBUG_PRINTLN("[CommandQueue] Failed to create command queue");
    }
}

CommandQueue::~CommandQueue() {
    if (commandQueue != nullptr) {
        // Drain any remaining commands
        VolumioCommand* cmdPtr = nullptr;
        while (xQueueReceive(commandQueue, &cmdPtr, 0) == pdTRUE) {
            if (cmdPtr != nullptr) {
                delete cmdPtr;
            }
        }
        vQueueDelete(commandQueue);
        commandQueue = nullptr;
    }
}

CommandQueue& CommandQueue::getInstance() {
    if (instance == nullptr) {
        instance = new CommandQueue();
    }
    return *instance;
}

VolumioCommand* CommandQueue::createCommandCopy(const VolumioCommand& cmd) {
    VolumioCommand* copy = new VolumioCommand();
    if (copy != nullptr) {
        copy->type = cmd.type;
        copy->value = cmd.value;
    }
    return copy;
}

bool CommandQueue::postCommand(const VolumioCommand& cmd) {
    if (commandQueue == nullptr) {
        return false;
    }

    // Create heap-allocated copy for the queue
    VolumioCommand* cmdPtr = createCommandCopy(cmd);
    if (cmdPtr == nullptr) {
        DEBUG_PRINTLN("[CommandQueue] Failed to allocate command");
        return false;
    }

    // Queue the pointer (non-blocking)
    BaseType_t result = xQueueSend(commandQueue, &cmdPtr, 0);

    if (result != pdTRUE) {
        DEBUG_PRINTLN("[CommandQueue] Queue full, command dropped");
        delete cmdPtr;
        return false;
    }

    return true;
}

bool CommandQueue::getNextCommand(VolumioCommand& cmd) {
    if (commandQueue == nullptr) {
        return false;
    }

    VolumioCommand* cmdPtr = nullptr;

    // Try to get one command from queue (non-blocking)
    if (xQueueReceive(commandQueue, &cmdPtr, 0) == pdTRUE) {
        if (cmdPtr != nullptr) {
            // Copy command data
            cmd.type = cmdPtr->type;
            cmd.value = cmdPtr->value;

            // Free the heap-allocated command
            delete cmdPtr;
            return true;
        }
    }

    return false;
}

