#ifndef USER_RESOURCE_H
#define USER_RESOURCE_H

#include "../../crow/include/crow.h"
#include "../../core/repositories/database.h"

class UserResource {
public:
    static void register_routes(crow::SimpleApp& app);
    
private:
    static crow::response login(const crow::request& req);
    static crow::response signup(const crow::request& req);
    static crow::response logout(const crow::request& req);
    static crow::response protected_endpoint(const crow::request& req);
};

#endif