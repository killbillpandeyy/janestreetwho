#include "crow/include/crow.h"
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <string>
#include <iostream>
// #include <boost/asio.hpp> // Uncomment if you want to use boost::asio

// Dummy order struct
struct DummyOrder {
    std::string symbol;
    int qty;
    std::string side;
};

namespace trading {
    std::atomic<bool> trading_active{false};
    std::vector<DummyOrder> dummy_orders = {
        {"AAPL", 1, "buy"},
        {"GOOG", 2, "sell"},
        {"TSLA", 1, "buy"},
    };
    std::mutex mtx;
    std::condition_variable cv;
    std::thread trading_thread;

    // Dummy function to simulate Alpaca API call
    void place_order(const DummyOrder& order) {
        // Replace with real Alpaca API call
        std::cout << "Placing order: " << order.symbol << " " << order.qty << " " << order.side << std::endl;
    }

    void trading_loop() {
        size_t idx = 0;
        while (trading_active) {
            {
                std::unique_lock<std::mutex> lock(mtx);
                if (!trading_active) break;
            }
            place_order(dummy_orders[idx % dummy_orders.size()]);
            idx++;
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait_for(lock, std::chrono::seconds(5), []{ return !trading_active.load(); });
        }
    }

    void start_trading() {
        if (trading_active) return;
        trading_active = true;
        trading_thread = std::thread(trading_loop);
    }

    void stop_trading() {
        trading_active = false;
        cv.notify_all();
        if (trading_thread.joinable()) trading_thread.join();
    }

    void register_routes(crow::SimpleApp& app) {
        CROW_ROUTE(app, "/startTrading").methods(crow::HTTPMethod::POST)([](const crow::request& req){
            start_trading();
            return crow::response(200, "Trading started");
        });
        CROW_ROUTE(app, "/stopTrading").methods(crow::HTTPMethod::POST)([](const crow::request& req){
            stop_trading();
            return crow::response(200, "Trading stopped");
        });
    }
}
