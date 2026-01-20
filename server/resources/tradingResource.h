#ifndef TRADING_RESOURCE_H
#define TRADING_RESOURCE_H

#include "../../crow/include/crow.h"

class TradingResource {
public:
    static void register_routes(crow::SimpleApp& app);
    
private:
    static crow::response start_trading(const crow::request& req);
    static crow::response stop_trading(const crow::request& req);
    static void start_trading_internal();
    static void stop_trading_internal();
};

#endif
