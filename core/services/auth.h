#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <chrono>

struct TokenInfo {
    std::string user;
    std::chrono::system_clock::time_point expires_at;
};

extern std::unordered_map<std::string, TokenInfo> tokens;
extern std::unordered_map<std::string, std::shared_ptr<std::mutex>> user_mutexes;
extern std::mutex global_mutex;

std::string generate_token();
std::shared_ptr<std::mutex> get_user_mutex(const std::string &user);
bool validate_token(const std::string &token, std::string &out_user);

#endif // AUTH_H