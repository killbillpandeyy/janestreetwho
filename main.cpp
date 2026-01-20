#define CROW_USE_BOOST 1
#include "crow/include/crow.h"
#include "server/resources/userResource.h"
#include "core/services/auth.h"
#include "core/repositories/database.h"
#include "server/resources/tradingResource.h"
#include <string>

int main()
{
    init_db();  // Initialize database

    crow::SimpleApp app; //define your crow application

    // Register all resources

    UserResource::register_routes(app);
    TradingResource::register_routes(app);

    // root
    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });

    //set the port, set the app to run on multiple threads, and run the app
    app.port(18080).multithreaded().run();

    sqlite3_close(db);
}