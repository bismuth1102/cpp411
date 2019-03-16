#!/bin/bash

g++ -std=c++11 life.cc -o life `pkg-config --cflags --libs opencv` -lboost_system -ltbb