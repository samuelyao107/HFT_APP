#include "me_order.hpp"

namespace Exchange{
    auto MEOrder::toString() const -> std::string {
        std::stringstream ss;
            ss << "MEOrder"
            <<" ["
            <<" ticker:"<<tickerIdToString(ticker_id_)
            <<" cid:"<<clientIdToString(client_id_)
            <<" oid:"<<orderIdToString(client_order_id_)
            <<" moid:"<<orderIdToString(market_order_id_)
            <<" side:"<<sideToString(side_)
            <<" price:"<<priceToString(price_)
            <<" qty:"<<qtyToString(qty_)
            <<" prio:"<<priorityToString(priority_)
            <<" prev:"<<orderIdToString( 
                prev_order_? prev_order_->market_order_id_: OrderId_INVALID)
            <<" next:"<<orderIdToString(
                next_order_? next_order_ ->market_order_id_ : OrderId_INVALID)    
            <<"]";
            return ss.str();
    }

    auto MEOrdersAtPrice::toString() const -> std::string{
        std::stringstream ss;
         ss << "MEOrdersAtPrice"
            <<" ["
            <<" side:"<<sideToString(side_)
            <<" price:"<<priceToString(price_)
            <<" first_me_order:"<<(
                firs_me_order_? firs_me_order_->toString():"null")
            <<" prev:"<< priceToString(
                prev_entry_? prev_entry_->price_ :Price_INVALID)  
            <<" next:"<<priceToString(
                next_entry_? next_entry_->price_: Price_INVALID)      
            <<"]";
            return ss.str();
    }
}