#pragma once

#include <stdint.h>
#include <cstddef>
#include <string.h>


struct dataPackage {
    size_t data_size = 0;

    uint32_t seq_number = 0; // номер пакета
    uint32_t seq_total = 0; // количество пакетов с данными
    uint8_t type = 0; // тип пакета: 0 == ACK, 1 == PUT
    std::byte id[8] = { (std::byte)'\0' }; // 8 байт - идентификатор, отличающий один файл от другого
    std::byte* data = nullptr; // после заголовка и до конца UDP пакета идут данные

    static int getSizeHeader() {
        static int size = 2 * sizeof(uint32_t) + sizeof(uint8_t) + 8 * sizeof(std::byte);
        return size;
    }

    static const size_t size_data_package = 1472;

    static int getMaxDataSize() {
        static int data_size = dataPackage::size_data_package - getSizeHeader();
        return data_size;
    }

    const static uint8_t ack = 0;
    const static uint8_t put = 1;

};


