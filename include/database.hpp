#pragma once

#include<fstream>
#include<stdio.h>
#include<iostream>
#include<cstring>

struct market_snapshot
{
    double timestamp;
    float bidPrice;
    float bidSize;
    float askPrice;
    float askSize;
};

void SHOW_ORDERBOOK();

// save data in binary files to the disk
class Database
{
public:
    market_snapshot ms;
public:
	int get_id();
	void READ(double timestamp, unsigned int bidprice, unsigned int bidsize, unsigned int askprice, unsigned int asksize);
	void SHOW();
	void ADDRECORD_AND_SAVE(double timestamp, unsigned int bidprice, unsigned int bidsize, unsigned int askprice, unsigned int asksize);

};