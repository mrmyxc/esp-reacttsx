#pragma once

extern "C"
{
#include "esp_err.h"
}

#include <string>

namespace U
{
    esp_err_t filesystem_init(const std::string_view mountPoint);
}
