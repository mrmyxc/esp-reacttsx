#pragma once
extern "C"
{
#include "esp_err.h"
}

namespace U
{
    esp_err_t start_rest_server(const char *base_path);
}