#pragma once

#include <array>
#include <sstream>
#include "common/types.hpp"

using namespace Common;

namespace Exchange {

    struct MEOrder {
         
        ClientId client_id_ = ClientId_INVALID;
        TickerId ticker_id_ = TickerId_INVALID;
        OrderId client_order_id_ = OrderId_INVALID;
        OrderId market_order_id_ = OrderId_INVALID;
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;
        Qty qty_ = Qty_INVALID;
        Priority priority_ = Priority_INVALID;

        MEOrder *prev_order_ = nullptr;
        MEOrder *next_order_ = nullptr;

        MEOrder()=default; //For creating orders memory pool

        MEOrder(TickerId ticker_id, ClientId client_order_id, OrderId client_order_id,
        OrderId market_order_id, Side side, Price price, Qty qty, Priority priority,
        MEOrder* prev_order, MEOrder* next_order):
            ticker_id_(ticker_id),
            client_order_id_(client_order_id),
            market_order_id_(market_order_id),
            side_(side),
            price_(price),
            qty_(qty),
            priority_(priority),
            prev_order_(prev_order),
            next_order_(next_order) {}
        
        auto toString() const -> std::string;     
    };
    //std::array<Type, Taille> hash func f(x)=x Direct Access Table
    typedef std::array<MEOrder*, ME_MAX_ORDER_IDS> OrderHashMap;
    typedef std::array<OrderHashMap, ME_MAX_NUM_CLIENTS> ClientOrderHashMap;

    struct MEOrdersAtPrice {
        Side side_ = Side::INVALID;
        Price price_ = Price_INVALID;

        MEOrder *firs_me_order_ = nullptr;

        MEOrdersAtPrice *prev_entry_ = nullptr;
        MEOrdersAtPrice *next_entry_ = nullptr;

        MEOrdersAtPrice() = default;

        MEOrdersAtPrice(Side side, Price price, MEOrder *firs_me_order,
        MEOrdersAtPrice *prev_entry, MEOrdersAtPrice *next_entry):
        side_(side),
        price_(price),
        firs_me_order_(firs_me_order),
        prev_entry_(prev_entry),
        next_entry_(next_entry) {}

        auto toString() const -> std::string; 
    };
}