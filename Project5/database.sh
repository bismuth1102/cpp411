#!/bin/bash

g++ -std=c++11 -o cs cs.cpp -lcrypto -lssl -lboost_system -lboost_thread -lboost_regex -lboost_serialization -lpthread $(pkg-config --cflags --libs libmongocxx)
