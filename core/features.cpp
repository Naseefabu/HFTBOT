#include "features.hpp"

TapeSpeedIndicator::TapeSpeedIndicator(int& lookback)
{
    lookback_period = lookback;
}

int TapeSpeedIndicator::get_tapespeed()
{
    remove();
    return tapespeed;
}

void TapeSpeedIndicator::add(double &volume)
{
    if(volume > 0)
    {
        tapespeed++;
        times.push_back(get_sec_timestamp(current_time()).count());
    }
}

void TapeSpeedIndicator::remove()
{
    std::chrono::seconds lbt = get_sec_timestamp(current_time()).count() - lookback_period;
    int temp=0;
    int pop=0
}