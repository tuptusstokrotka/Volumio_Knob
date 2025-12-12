#ifndef VOLUMIO_TRACKDATA_H
#define VOLUMIO_TRACKDATA_H

#include <string>

struct Info {
    std::string status      = "";
    std::string title       = "";
    std::string artist      = "";
    std::string album       = "";
    std::string trackType   = "";
    unsigned int seek       = 0;
    unsigned int duration   = 0;
    std::string samplerate  = "";
    std::string bitdepth    = "";
    bool random             = false;
    bool repeat             = false;
    bool repeatSingle       = false;
};

#endif // VOLUMIO_TRACKDATA_H