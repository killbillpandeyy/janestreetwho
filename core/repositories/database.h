#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <sqlite3.h>
#include <mutex>

extern sqlite3* db;
extern std::mutex db_mutex;  // Global mutex for DB access

void init_db();
bool check_credentials(const std::string& user, const std::string& pass);
bool signup_user(const std::string& user, const std::string& pass);  // New signup function
void store_token(const std::string& token, const std::string& user, long long expires_at);
bool validate_token_db(const std::string& token, std::string& out_user);
void remove_token(const std::string& token);

#endif // DATABASE_H