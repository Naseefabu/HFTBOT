# MM++  

WARNING : The author of this software will not be responsible for your losses on the market, use at your own discretion.
This bot is still in construction.

## Introduction

The cross exchange market making / Hedged Market Making strategy performs market making trades between two markets: 

* It emits limit orders to a less liquid, larger spread market. 
* It emits market orders on a more liquid, smaller spread market whenever the limit orders were hit. 

Market making isn't about placing bunch of bids and asks orders, you will soon realize orderflow isn't random at all and you get 
filled irregularly and accumulate inventory(which is a serious risk). Hedged Market making can help you with the inventory risk, unless you
are slow somewhere. Basics remains the same, MM is all about capturing the spread and managing your inventory.
You need to price in information as much as possible in a very low latency manner. 

![Photo](https://user-images.githubusercontent.com/104965020/183235797-03f2f9d1-648e-4e12-a68b-62059a870f4d.png)

## Components

1) Feedhandlers
* Recieves market data from exchanges, and sends the data to bookbuilders to build the books for all secuirities.
2) Book Builders
* Build the orderbook snapshots to be consumed by strategy component.
3) Strategy
* Recieves orderbook snapshots from Book builders; generates signals as indicator values,predictions of stock prices
 facilitates communication with the order management system 
4) OMS

## Architectural Design

![MM++ arch-Page-1 drawio (3)](https://user-images.githubusercontent.com/104965020/188065754-2ec5a554-9c3e-409c-84d5-10737a49e3b2.png)

Components are scheduled to execute on distinct CPU cores using thread affinity and CPU isolation, which helps in preventing a thread from wandering between CPUs and reducing context switching overheads. Component Threads are expected to busy poll and never be interrupted, as it reduces the number of context switches
because context switches only happens at each time slice.

## Risk Management Tools

* If speed of arrival market orders picks up across exchanges, stop quoting. 
* BART move detections helps in determining taker exchange overloads.

## Build Instructions

### Install dependencies

```
# install dependent packages
sudo apt-get update
sudo apt-get install libboost-all-dev


# install cmake
sudo apt install cmake

# install boost 1.79
wget https://www.boost.org/users/history/version_1_79_0.html
tar --bzip2 -xf boost_1_79_0.tar.bz2
cd boost_1_79_0/
./boostrap.sh
./b2
sudo ./b2 install

```

Steps to build and run the bot:
```
git clone --recurse-submodules https://github.com/Naseefabu/HFTBOT.git
cd HFTBOT/
./configure.sh
./build.sh
./run.sh
```
