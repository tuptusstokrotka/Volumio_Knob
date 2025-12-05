#ifndef VOLUMIO_CMD_H
#define VOLUMIO_CMD_H

//https://developers.volumio.com/api/rest-api

#define VOLUMIO_CMD_PLAY            "play"
#define VOLUMIO_CMD_PAUSE           "pause"
#define VOLUMIO_CMD_STOP            "stop"
#define VOLUMIO_CMD_TOGGLE          "toggle"
#define VOLUMIO_CMD_PREV            "prev"
#define VOLUMIO_CMD_NEXT            "next"
#define VOLUMIO_CMD_RANDOM          "random" // No value = toggle
#define VOLUMIO_CMD_REPEAT          "repeat" // No value = toggle
#define VOLUMIO_CMD_SEEK(value)     "seek&position=" + std::to_string(value)

#endif // VOLUMIO_CMD_H
