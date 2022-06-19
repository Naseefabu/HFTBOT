# Short Gamma Strategy (In Construction)

Steps to build and run the bot:
```
git clone --recurse-submodules https://github.com/Naseefabu/HFTBOT.git
cd HFTBOT/
./configure.sh
./build.sh
./run.sh
```

# Introduction

Low realised vol regime in BTC gave unique opportunity to exploit. Since RV < IV (most of the time), shorting FTX MOVE contracts is +ev due to theta decay and simply collect premium over T. This bot monitors multiple other big exchanges to identify whether a big move in BTC is coming (eg : BART-move detection) and multiple other risk management tools. 

# Risk Management Tools

* If speed of arrival market orders picks up across exchanges, then reduce/exit position. 
* If predicted RVOL is greater than implied vol by "X" margin exit the position.
* If (taker volume/liquidity at top "X" levels) ratio pick up, then reduce/exit position.

