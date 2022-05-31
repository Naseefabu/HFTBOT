#include "utils.hpp"

namespace utils
{
    std::string e_side_to_string(const e_side side)
    {
        switch (side)
        {
            case buy : return "BUY";
            case sell : return "SELL"; 
        }
        return nullptr;
    }

    std::string order_type_to_string(order_type type)
    {
        switch (type)
        {
            case limit : return "LIMIT";
            case market : return "MARKET";
        }
        return nullptr;
    }
    std::string timeforce_to_string(timeforce timeforc)
    {
        switch (timeforc)
        {
            case GTC : return "GTC";
            case IOC : return "IOC";
            case FOK : return "FOK";
        }
        return nullptr;
    }
    std::string trade_response_type_to_string(trade_response_type type)
    {
        switch(type)
        {
            case ack : return "ACK";
            case result : return "RESULT";
            case full : return "FULL";
            case test : return "TEST";
            case unknown : return "UNKNOWN";
        }
        return nullptr;
    }
    TimePoint current_time()
    {
        return Clock::now();
    }

    std::chrono::milliseconds get_ms_timestamp(TimePoint time)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch());
    }
}