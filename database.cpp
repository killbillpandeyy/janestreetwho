#include "database.h"
#include <iostream>
#include <chrono>

sqlite3* db;

void execute_sql(const std::string& sql) {
    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }
}

void init_db() {
    if (sqlite3_open("users.db", &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }

    execute_sql("CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password_hash TEXT);");
    execute_sql("CREATE TABLE IF NOT EXISTS tokens (token TEXT PRIMARY KEY, username TEXT, expires_at INTEGER);");
    execute_sql("INSERT OR IGNORE INTO users (username, password_hash) VALUES ('admin', 'password');");
}

bool check_credentials(const std::string& user, const std::string& pass) {
    std::string sql = "SELECT password_hash FROM users WHERE username = '" + user + "';";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    bool valid = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string stored_pass = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        valid = (stored_pass == pass);
    }
    sqlite3_finalize(stmt);
    return valid;
}

bool signup_user(const std::string& user, const std::string& pass) {
    std::string sql = "INSERT INTO users (username, password_hash) VALUES ('" + user + "', '" + pass + "');";
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "Signup error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;  // User already exists or other error
    }
    return true;
}

void store_token(const std::string& token, const std::string& user, long long expires_at) {
    std::string sql = "INSERT INTO tokens (token, username, expires_at) VALUES ('" + token + "', '" + user + "', " + std::to_string(expires_at) + ");";
    execute_sql(sql);
}

bool validate_token_db(const std::string& token, std::string& out_user) {
    std::string sql = "SELECT username, expires_at FROM tokens WHERE token = '" + token + "';";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    bool valid = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out_user = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        long long expires_at = sqlite3_column_int64(stmt, 1);
        if (std::chrono::system_clock::now().time_since_epoch().count() < expires_at) {
            valid = true;
        } else {
            std::string del_sql = "DELETE FROM tokens WHERE token = '" + token + "';";
            execute_sql(del_sql);
        }
    }
    sqlite3_finalize(stmt);
    return valid;
}

void remove_token(const std::string& token) {
    std::string sql = "DELETE FROM tokens WHERE token = '" + token + "';";
    execute_sql(sql);
}