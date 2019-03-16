#!/bin/bash

g++ -std=c++11 -o cc cc.cpp -lcrypto -lssl -lboost_system -lboost_thread -lboost_regex -lboost_serialization -lpthread -ltwitcurl -ljsoncpp

g++ -std=c++11 -o cs cs.cpp -lcrypto -lssl -lboost_system -lboost_thread -lboost_regex -lboost_serialization -lpthread $(pkg-config --cflags --libs libmongocxx)

g++ -std=c++11 -o ss ss.cpp -lboost_system -lcrypto -lssl -lcpprest $(pkg-config --cflags --libs libmongocxx) -ljsoncpp
