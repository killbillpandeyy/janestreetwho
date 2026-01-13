#define CROW_USE_BOOST 1
#include "crow/include/crow.h"
#include "auth.h"
#include "database.h"
#include <string>

int main()
{
    init_db();  // Initialize database

    crow::SimpleApp app; //define your crow application

    // POST /login  body: {"user":"admin","pass":"password"}
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)([] (const crow::request &req) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);
        std::string user = body["user"].s();
        std::string pass = body["pass"].s();

        if (!check_credentials(user, pass)) {
            return crow::response(401);
        }

        std::string token = generate_token();
        auto now = std::chrono::system_clock::now();
        long long expires_at = (now + std::chrono::hours(1)).time_since_epoch().count();

        store_token(token, user, expires_at);

        crow::json::wvalue resp;
        resp["token"] = token;
        resp["expires_in_seconds"] = 3600;
        return crow::response{resp};
    });

    // GET /protected  requires header: Authorization: Bearer <token>
    CROW_ROUTE(app, "/protected").methods(crow::HTTPMethod::GET)([] (const crow::request &req) {
        auto auth = req.get_header_value("Authorization");
        if (auth.rfind("Bearer ", 0) != 0) return crow::response(401);
        std::string token = auth.substr(7);

        std::string user;
        if (!validate_token(token, user)) return crow::response(401);

        crow::json::wvalue resp;
        resp["message"] = std::string("Hello ") + user;
        return crow::response{resp};
    });

    // POST /logout  requires Authorization: Bearer <token>
    CROW_ROUTE(app, "/logout").methods(crow::HTTPMethod::POST)([] (const crow::request &req) {
        auto auth = req.get_header_value("Authorization");
        if (auth.rfind("Bearer ", 0) != 0) return crow::response(401);
        std::string token = auth.substr(7);

        remove_token(token);
        return crow::response(200);
    });

    //
    CROW_ROUTE(app, "/signup").methods(crow::HTTPMethod::POST)([] (const crow::request &req) {
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400);
        std::string user = body["user"].s();
        std::string pass = body["pass"].s();

        if (signup_user(user, pass)) {
            return crow::response(201, "User created");
        } else {
            return crow::response(409, "User already exists");
        }
    });

    // root
    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });

    //set the port, set the app to run on multiple threads, and run the app
    app.port(18080).multithreaded().run();

    sqlite3_close(db);
}