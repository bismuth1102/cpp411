#!/bin/bash

g++ -std=c++11 -o ss ss.cpp -lboost_system -lcrypto -lssl -lcpprest $(pkg-config --cflags --libs libmongocxx) -ljsoncpp
