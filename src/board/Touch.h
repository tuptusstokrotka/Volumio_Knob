#pragma once

#include "../drivers/FT3267.h"
#include "dev_tools.h"

class Touch : public FT3267::TP_FT3267{
private:
    bool last_touched = false;
public:
    Touch(void);
    ~Touch(void);

    bool GetTouched(void);
};
