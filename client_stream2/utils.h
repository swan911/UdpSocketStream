#pragma once

#include "defs.h"
#include <vector>
#include <set>
#include <stdio.h> 

const uint32_t crc32c_crc = 0;
uint32_t crc32c(uint32_t crc, const unsigned char* buf, size_t len)
{
    int k;
    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ 0x82f63b78 : crc >> 1;
    }
    return ~crc;
}


char* getBufCheckSum(uint32_t checksum, size_t& len) {
    const int size = 16;
    char* ret = new char[size]{'\0'};
    sprintf(ret, "%lu", checksum);
    len = strlen(ret);
    return ret;
}



std::vector<dataPackage*> getListPackages(unsigned char* send_data, int send_data_size, std::byte* id) {
    std::vector<dataPackage*> packages;
    int max_data_size = dataPackage::getMaxDataSize();
    double number = (double)send_data_size / max_data_size;
    double rest = number - (int)number;
    int send_data_total_number = (int)number;
    if (rest > 0)
        send_data_total_number++;
    for (int i = 0; i < send_data_total_number; i++) {
        dataPackage* pPackage = new dataPackage();
        pPackage->seq_number = i + 1;
        pPackage->seq_total = send_data_total_number;
        for (int j = 0; j < 8; j++)
            pPackage->id[j] = id[j];

        pPackage->type = dataPackage::put;
        pPackage->data = (std::byte*)(send_data + (i * max_data_size));
        if (i == (send_data_total_number - 1)) { //last
            pPackage->data_size = send_data_size - i * max_data_size;
        }
        else {
            pPackage->data_size = max_data_size;
        }
        packages.push_back(pPackage);
    }
    return packages;
}


dataPackage* readData(char* recv_data, int recv_data_size) {
    dataPackage* package = new dataPackage();
    uint32_t* p = (uint32_t*)recv_data;
    package->seq_number = *p;
    p++;
    package->seq_total = *p;
    p++;
    uint8_t* type = (uint8_t*)p;
    package->type = *type;
    type++;
    std::byte* id = (std::byte*)type;
    for (int i = 0; i < 8; i++) {
        package->id[i] = *id;
        id++;
    }
    std::byte* data = (std::byte*)id;
    int size = recv_data_size - package->getSizeHeader();
    package->data_size = size;
    package->data = new std::byte[size];
    for (int i = 0; i < size; i++) {
        package->data[i] = *data;
        data++;
    }
    return package;
}

char* getDataForSend(const dataPackage& package) {
    char* ret = new char[package.data_size + package.getSizeHeader()];
    char* p = ret;
    uint32_t* sn = (uint32_t*)p;
    *sn = package.seq_number;
    sn++;
    uint32_t* st = (uint32_t*)sn;
    *st = package.seq_total;
    st++;
    uint8_t* type = (uint8_t*)st;
    *type = package.type;
    type++;
    std::byte* id = (std::byte*)type;
    for (int i = 0; i < 8; i++) {
        *id = package.id[i];
        id++;
    }
    std::byte* data = (std::byte*)id;
    for (size_t i = 0; i < package.data_size; i++) {
        *data = package.data[i];
        data++;
    }
    return ret;
}


bool eqPackage(const dataPackage& pack, const dataPackage& package) {
    if (pack.seq_number != package.seq_number)
        return false;

    if (pack.seq_total != package.seq_total)
        return false;

    if (pack.type != package.type)
        return false;

    for (int i = 0; i < 8; i++) {
        if (pack.id[i] != package.id[i])
            return false;
    }

    for (size_t i = 0; i < package.data_size; i++) {
        if (pack.data[i] != package.data[i])
            return false;
    }
    return true;
}



void sendData(const dataPackage & package) {
    char* data = getDataForSend(package);
    int size = package.data_size + package.getSizeHeader();
    // TODO send    
    
    // test
    auto pack = readData(data, size);
    //eqPackage(*pack, package);
    uint32_t n = crc32c(0, (const unsigned char*)data, size);
    char* pack_data = getDataForSend(*pack);
    uint32_t n2 = crc32c(0, (const unsigned char*)pack_data, size);
    //if (n == n2) 
    int a = 0;
}

