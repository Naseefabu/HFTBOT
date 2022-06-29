#include "database.hpp"


void Database::READ(double timestamp, unsigned int bidprice, unsigned int bidsize, unsigned int askprice, unsigned int asksize)
{
    ms.timestamp = timestamp;
    ms.bidPrice = bidprice;
    ms.askPrice = askprice;
    ms.askSize = asksize;
    ms.bidSize = bidsize;
}

void Database::SHOW()
{
    std::cout<<"timestamp : "<<ms.timestamp<<std::endl;
    std::cout<<"Bid price : "<<ms.bidPrice<<std::endl;
    std::cout<<"Bid size : "<<ms.bidSize<<std::endl;
    std::cout<<"Ask price : "<<ms.askPrice<<std::endl;
    std::cout<<"Ask size : "<<ms.askSize<<std::endl;
}

void Database::ADDRECORD_AND_SAVE(double timestamp, unsigned int bidprice, unsigned int bidsize, unsigned int askprice, unsigned int asksize)
{
    fstream fin;
    fin.open("orderbook_database.dat",ios::in|ios::binary|ios::app);
    READ(timestamp,bidprice,bidsize,askprice,asksize);
    fin.write((char*)&ms,sizeof(ms));
    fin.close();
}



void SHOW_ORDERBOOK()
{
	market_snapshot S;
    Database d;
	fstream fin;

	fin.open("orderbook_database.dat",ios::in|ios::binary|ios::app) ;
	while(fin.read((char*)&S,sizeof(S)))
	{
		d.ms = S;
		d.SHOW();
        std::cout << "-----------------NEXT DATA-----------------"<<std::endl;
    }
    
	fin.close();

}