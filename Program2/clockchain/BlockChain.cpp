//
// Created by Dave Nash on 20/10/2017.
//

#include "BlockChain.h"

Blockchain::Blockchain()
{
    _vChain.__emplace_back(Block(0, "Genesis Block"));
    _nDifficulty = 6;
}

void Blockchain::AddBlock(Block bNew)
{
    // bNew.sPrevHash = _GetLastBlock().sHash;
    // bNew.MineBlock(_nDifficulty);
    _vChain.push_back(bNew);
}

Block Blockchain::_GetLastBlock()
{
    return _vChain.back();
}

