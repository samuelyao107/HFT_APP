#include <cstdint>
#include <limits>
#include "common/macros.hpp"


namespace Common{

    constexpr size_t LOG_QUEUE_SIZE = 8 * 1024 * 1024;
    //ME = Matching Engine
    constexpr size_t ME_MAX_TICKERS = 8;

    constexpr size_t ME_MAX_CLIENT_UPDATES = 256 * 1024;
    constexpr size_t ME_MAX_MARKET_UPDATES = 256 * 1024;

    constexpr size_t ME_MAX_NUM_CLIENTS = 256;
    constexpr size_t ME_MAX_ORDER_IDS = 1024 * 1024;
    constexpr size_t ME_MAX_PRICE_LEVELS = 256;


    //OrderId
    typedef uint64_t OrderId;
    constexpr auto OrderId_INVALID = std::numeric_limits<OrderId>::max();

    inline auto orderIdToString(OrderId orderId) -> std::string {
        if(orderId == OrderId_INVALID)[[unlikely]]{
            return "INVALID";
        }
        return std::to_string(orderId);
    }

    //TickerId
    typedef uint32_t TickerId;
    constexpr auto TickerId_INVALID = std::numeric_limits<TickerId>::max();

      inline auto tickerIdToString(TickerId tickerId) -> std::string {
        if(tickerId == TickerId_INVALID)[[unlikely]]{
            return "INVALID";
        }
        return std::to_string(tickerId);
    }

    //ClientId
    typedef uint32_t ClientId;
    constexpr auto ClientId_INVALID = std::numeric_limits<ClientId>::max();

    inline auto clientIdToString(ClientId clientId) -> std::string {
        if(clientId == ClientId_INVALID)[[unlikely]]{
            return "INVALID";
        }
        return std::to_string(clientId);
    }

    //PRICE
    typedef int64_t Price;
    constexpr auto Price_INVALID = std::numeric_limits<Price>::max();

    inline auto priceToString(Price price) -> std::string {
        if(price == Price_INVALID)[[unlikely]]{
            return "INVALID";
        }
        return std::to_string(price);
    }

    //Quantity
    typedef uint32_t Qty;
    constexpr auto Qty_INVALID = std::numeric_limits<Qty>::max();

    inline auto qtyToString(Qty qty) -> std::string {
        if(qty == Qty_INVALID)[[unlikely]]{
            return "INVALID";
        }
        return std::to_string(qty);
    }

    //Priority
    typedef uint64_t Priority;
    constexpr auto Priority_INVALID = std::numeric_limits<Priority>::max();

    inline auto priorityToString(Priority priority) -> std::string {
        if(priority == Price_INVALID)[[unlikely]]{
            return "INVALID";
        }
        return std::to_string(priority);
    }

    //Side
    enum class Side : int8_t {
        INVALID = 0,
        BUY=1,
        SELL=-1
    };
    inline auto sideToString(Side side) -> std::string {
        switch(side){
            case Side::BUY:
                return "BUY";
            case Side::SELL :
                return "SELL";
            case Side::INVALID :
                return "INVALID";        
        }
        return "UNKNOWN";
    }
}
