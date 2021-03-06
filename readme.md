# Theta-Ripper++  
![thetagang](https://user-images.githubusercontent.com/104965020/180367924-c6c7bb89-8acd-47d9-ac21-f28ceab92a04.png)

WARNING : The author of this software will not be responsible for your losses on the market, use at your own discretion.

This bot is still in construction.

## Introduction

Low realised vol regime in BTC gave unique opportunity to exploit. Since RV < IV (most of the time), shorting FTX MOVE contracts is +ev due to theta decay and simply collect premium over T. This bot monitors multiple other big exchanges to identify whether a big move in BTC is coming (eg : BART-move detection and multiple other risk management tools) as tails of this strategy can be really bad. 

## Risk Management Tools

* If speed of arrival market orders picks up across exchanges, then reduce/exit position. 
* If predicted RVOL is greater than implied vol by "X" margin exit the position.
* If (taker volume/liquidity at top "X" levels) ratio pick up, then reduce/exit position.


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

# Join our community

Join us on discord : https://discord.gg/8ZWx58SfWQ