//
//  Sha.cpp
//  test2
//
//  Created by vancleecheng on 2018/10/1.
//  Copyright © 2018年 apple. All rights reserved.
//

#include <iostream>
#include <string>
#include "Sha.h"
#include "openssl/sha.h"

using namespace std;

string sha256(const string str)
{
    char buf[2];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    std::string NewString = "";
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(buf,"%02x",hash[i]);
        NewString = NewString + buf;
    }
    return NewString;
}
