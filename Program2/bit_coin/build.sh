#!/bin/bash
g++ -w -c Sha.cpp
g++ -w -c Block.cpp
g++ -w -c Blockchain.cpp
g++ -w -o server server.cpp -lcrypto -lssl -lboost_system -lboost_thread-mt -lboost_date_time -lboost_regex -lboost_serialization -lpthread Block.o Blockchain.o Sha.o
g++ -w -o client client.cpp -lcrypto -lssl -lboost_system -lboost_thread-mt -lboost_date_time -lboost_regex -lboost_serialization -lpthread Block.o Blockchain.o Sha.o
