#ifndef VOLUMIO_TRACKDATA_H
#define VOLUMIO_TRACKDATA_H

#include <string>

struct Info{
    std::string status = "";
    std::string title = "";
    std::string artist = "";
    std::string samplerate = "";
    std::string bitdepth = "";
    std::string trackType = "";
    unsigned int duration = 0;
    unsigned int seek = 0;
    unsigned int elapsedTime = 0;
};

#endif // VOLUMIO_TRACKDATA_H