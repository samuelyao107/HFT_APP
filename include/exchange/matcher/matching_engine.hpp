#pragma once
#include "common/lock_free_queue.hpp"
#include "common/thread_utils.hpp"
#include "common/macros.hpp"

#include "order_server/client_request.hpp"
#include "order_server/client_response.hpp"
#include "market_data/market_update.hpp"
#include "common/logging.hpp"

//#include "me_order_book.hpp"

namespace Exchange{
    class MatchingEngine final {
        public:
            MatchingEngine(ClientRequestLFQueue *client_requests,
                             ClientResponseLFQueue *client_responses,
                            MEMarketUpdateLFQueue *market_updates);
            
            ~MatchingEngine();
            auto start() -> void;
            auto stop() -> void;

            MatchingEngine() = delete;
            MatchingEngine(const MatchingEngine&) = delete;
            MatchingEngine(const MatchingEngine&&)= delete;
            MatchingEngine &operator=(const MatchingEngine&) = delete;
            MatchingEngine &operator= (const MatchingEngine &&) = delete;
        
        private:
           OrderBookHashMap ticker_order_book_;  
           
           ClientRequestLFQueue *incoming_requests_ = nullptr;
           ClientResponseLFQueue *outgoing_ogw_responses_ = nullptr;
           MEMarketUpdateLFQueue *outgoing_ogw_updates = nullptr;

           volatile bool run_ = false;

           std::string time_str_;
           Logger logger_;
           auto run() noexcept;
    }
}
