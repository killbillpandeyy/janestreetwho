#include "auth.h"
#include "database.h"
#include <random>
#include <sstream>
#include <iomanip>

std::string generate_token() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    uint64_t a = dist(gen);
    uint64_t b = dist(gen);
    std::stringstream ss;
    ss << std::hex << std::setw(16) << std::setfill('0') << a
       << std::hex << std::setw(16) << std::setfill('0') << b;
    return ss.str();
}

bool validate_token(const std::string &token, std::string &out_user) {
    return validate_token_db(token, out_user);
}