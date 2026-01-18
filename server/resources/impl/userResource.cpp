#include "../userResource.h"
#include "../../../core/services/auth.h"
#include <chrono>

void UserResource::register_routes(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)(
        [](const crow::request& req) { return UserResource::login(req); }
    );
    
    CROW_ROUTE(app, "/signup").methods(crow::HTTPMethod::POST)(
        [](const crow::request& req) { return UserResource::signup(req); }
    );
    
    CROW_ROUTE(app, "/logout").methods(crow::HTTPMethod::POST)(
        [](const crow::request& req) { return UserResource::logout(req); }
    );

    CROW_ROUTE(app, "/protected").methods(crow::HTTPMethod::GET)(
    [](const crow::request& req) { return UserResource::protected_endpoint(req); }
);
}

crow::response UserResource::login(const crow::request& req) {
    auto body = crow::json::load(req.body);
    if (!body) return crow::response(400);
    std::string username = body["username"].s();
    std::string password = body["password"].s();

    if (!check_credentials(username, password)) {
        return crow::response(401);
    }

    std::string token = generate_token();
    auto now = std::chrono::system_clock::now();
    long long expires_at = (now + std::chrono::hours(1)).time_since_epoch().count();
    store_token(token, username, expires_at);

    crow::json::wvalue resp;
    resp["token"] = token;
    resp["expires_in_seconds"] = 3600;
    return crow::response{resp};
}

crow::response UserResource::protected_endpoint(const crow::request& req) {
    auto auth = req.get_header_value("Authorization");
    if (auth.rfind("Bearer ", 0) != 0) return crow::response(401);
    std::string token = auth.substr(7);

    std::string username;
    if (!validate_token(token, username)) return crow::response(401);

    crow::json::wvalue resp;
    resp["message"] = std::string("Hello ") + username;
    return crow::response{resp};
}

crow::response UserResource::signup(const crow::request& req) {
    auto body = crow::json::load(req.body);
    if (!body) return crow::response(400);
    std::string username = body["username"].s();
    std::string password = body["password"].s();

    if (signup_user(username, password)) {
        return crow::response(201, "User created");
    } else {
        return crow::response(409, "User already exists");
    }
}

crow::response UserResource::logout(const crow::request& req) {
    auto auth = req.get_header_value("Authorization");
    if (auth.rfind("Bearer ", 0) != 0) return crow::response(401);
    std::string token = auth.substr(7);
    remove_token(token);
    return crow::response(200);
}