//
// Created by Dave Nash on 20/10/2017.
//

#include "Block.h"
#include "Sha.h"

Block::Block(uint32_t nIndexIn, const string &sDataIn) : _nIndex(nIndexIn), _sData(sDataIn)
{
    id = "";
    _nNonce = 0;
    _tTime = time(nullptr);
    
    sHash = _CalculateHash();
}

Block::Block(string idIn, uint32_t nIndexIn, const string &sDataIn, uint32_t nNonceIn, time_t tTimeIn, string sHashIn, string sPrevHashIn)
{
    id = idIn;
    _nIndex = nIndexIn;
    _sData = sDataIn;
    _nNonce = nNonceIn;
    _tTime = tTimeIn;
    sHash = sHashIn;
    sPrevHash = sPrevHashIn;
}

void Block::MineBlock(uint32_t nDifficulty)
{
    char cstr[nDifficulty + 1];
    for (uint32_t i = 0; i < nDifficulty; ++i)
    {
        cstr[i] = '0';
    }
    cstr[nDifficulty] = '\0';
    
    string str(cstr);
    
    while (sHash.substr(0, nDifficulty) != str)
    {
        _nNonce++;
        sHash = _CalculateHash();
    }
}

inline string Block::_CalculateHash() const
{
    stringstream ss;
    ss << _nIndex << sPrevHash << _tTime << _sData << _nNonce;
    
    return sha256(ss.str());
}
