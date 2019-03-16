//
// Created by Dave Nash on 20/10/2017.
//

#ifndef TESTCHAIN_BLOCK_H
#define TESTCHAIN_BLOCK_H

#include <cstdint>
#include <iostream>
#include <sstream>

using namespace std;

class Block {
public:
    string sHash;
    string sPrevHash;
    
    Block(uint32_t nIndexIn, const string &sDataIn);

    Block(string id, uint32_t nIndexIn, const string &sDataIn, uint32_t nNonceIn, time_t tTimeIn, string sHashIn, string sPrevHashIn);
    
    void MineBlock(uint32_t nDifficulty);
    
    string getID(){return id;};
    uint32_t getIndex(){return _nIndex;};
    uint32_t getNonce(){return _nNonce;};
    string getData(){return _sData;};
    time_t getTime(){return _tTime;};
    
private:
    string id;
    uint32_t _nIndex;
    uint32_t _nNonce;
    string _sData;
    time_t _tTime;
    
    string _CalculateHash() const;
};

#endif //TESTCHAIN_BLOCK_H
