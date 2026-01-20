#include "../tradingResource.h"
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <string>
#include <iostream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

// Dummy order struct
struct DummyOrder {
    std::string symbol;
    int qty;
    std::string side;
};

// Static members for trading state
static std::atomic<bool> trading_active{false};
static std::vector<DummyOrder> dummy_orders = {
    {"AAPL", 1, "buy"},
    {"GOOG", 2, "sell"},
    {"TSLA", 1, "buy"},
};

static std::mutex mtx;
static std::condition_variable cv;
static std::thread trading_thread;

// Dummy function to simulate Alpaca API call
static void place_order(const DummyOrder& order) {
    try {
        // Setup HTTPS connection
        net::io_context ioc;
        ssl::context ctx(ssl::context::tlsv12_client);
        ctx.set_default_verify_paths();
        
        tcp::resolver resolver(ioc);
        beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
        
        // Set SNI Hostname
        if (!SSL_set_tlsext_host_name(stream.native_handle(), "paper-api.alpaca.markets")) {
            throw beast::system_error(
                beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()),
                "Failed to set SNI Hostname");
        }
        
        // Resolve and connect
        auto const results = resolver.resolve("paper-api.alpaca.markets", "443");
        beast::get_lowest_layer(stream).connect(results);
        stream.handshake(ssl::stream_base::client);
        
        // Build JSON body
        std::ostringstream json_body;
        json_body << "{"
                  << "\"symbol\":\"" << order.symbol << "\","
                  << "\"qty\":" << order.qty << ","
                  << "\"side\":\"" << order.side << "\","
                  << "\"type\":\"market\","
                  << "\"time_in_force\":\"day\""
                  << "}";
        
        std::string body_str = json_body.str();
        
        // Create HTTP request
        http::request<http::string_body> req{http::verb::post, "/v2/orders", 11};
        req.set(http::field::host, "paper-api.alpaca.markets");
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::content_type, "application/json");
        req.set("APCA-API-KEY-ID", "PK7GWPSDCBWNHTOBY5WK4SXFXA");
        req.set("APCA-API-SECRET-KEY", "43gMa25Enh2ZAxRgdfHcwaGuu525DKiptxUUzUwNz5VR");
        req.body() = body_str;
        req.prepare_payload();
        
        // Send request
        http::write(stream, req);
        
        // Receive response
        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream, buffer, res);
        
        std::cout << "Order placed: " << order.symbol << " " << order.qty << " " << order.side 
                  << " - Status: " << res.result_int() << std::endl;
        
        // Graceful shutdown
        beast::error_code ec;
        stream.shutdown(ec);
        
    } catch (std::exception const& e) {
        std::cerr << "Error placing order: " << e.what() << std::endl;
    }
}

static void trading_loop() {
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

void TradingResource::start_trading_internal() {
    if (trading_active) return;
    trading_active = true;
    trading_thread = std::thread(trading_loop);
}

void TradingResource::stop_trading_internal() {
    trading_active = false;
    cv.notify_all();
    if (trading_thread.joinable()) trading_thread.join();
}

crow::response TradingResource::start_trading(const crow::request& req) {
    start_trading_internal();
    return crow::response(200, "Trading started");
}

crow::response TradingResource::stop_trading(const crow::request& req) {
    stop_trading_internal();
    return crow::response(200, "Trading stopped");
}

void TradingResource::register_routes(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/startTrading").methods(crow::HTTPMethod::POST)(
        [](const crow::request& req) { return TradingResource::start_trading(req); }
    );
    
    CROW_ROUTE(app, "/stopTrading").methods(crow::HTTPMethod::POST)(
        [](const crow::request& req) { return TradingResource::stop_trading(req); }
    );
}
