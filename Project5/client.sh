#!/bin/bash

g++ -std=c++11 -o cc cc.cpp -lcrypto -lssl -lboost_system -lboost_thread -lboost_regex -lboost_serialization -lpthread -ltwitcurl -ljsoncpp
