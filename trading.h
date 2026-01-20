#pragma once
#include "crow/include/crow.h"

namespace trading {
    void register_routes(crow::SimpleApp& app);
    void start_trading();
    void stop_trading();
}
