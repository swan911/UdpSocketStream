#pragma once
#include "utils.h"

struct ClientManagerPackagesCmp {
    bool operator() (const dataPackage* l, const dataPackage* r) const {
        return l->seq_number < r->seq_number;
    }
};

class ClientManagerPackages {
public:
    ClientManagerPackages(dataPackage* pack) {
        packages.insert(pack);
        for (int i = 0; i < 8; i++)
            id[i] = pack->id[i];
    }
    ~ClientManagerPackages() {
        if (remove)
            for (auto it = packages.begin(); it != packages.end(); it++) {
                delete* it;
            }
        packages.clear();
    }
public:
    bool isLastPackage(dataPackage* pack) { 
        packages.insert(pack);
        remove = true;
        if (packages.size() == pack->seq_total) {// last
            return true;
        }
        return false;
    }
    uint32_t getCheckSum() const {
        uint32_t ret = 0;
        int data_size = 0;
        for (auto it = packages.begin(); it != packages.end(); it++) {
            data_size += (*it)->data_size;
        }
        unsigned char* buf = new unsigned char[data_size];
        int current_pointer = 0;
        for (auto it = packages.begin(); it != packages.end(); it++) {
            auto pack = *it;
            memcpy(buf + current_pointer, pack->data, pack->data_size);
            current_pointer += pack->data_size;
        }
        ret = crc32c(crc32c_crc, buf, data_size);
        delete[] buf;
        return ret;
    }
    std::byte getId(size_t i) const { return (i < 8) ? id[i] : (std::byte)0; }
    uint32_t getCountRecvPackages() const { return (uint32_t)packages.size(); }

private:
    ClientManagerPackages();
private:
    std::byte id[8];
    std::set<dataPackage*, ClientManagerPackagesCmp> packages;
    bool remove = false;

};

struct ManagerPackagesCmp {
    bool operator() (const ClientManagerPackages* l, const ClientManagerPackages* r) const {
        for (int i = 0; i < 8; i++) {
            if (l->getId(i) == r->getId(i))
                continue;
            else if (l->getId(i) < r->getId(i))
                return true;
            else return false;
        }
        return false;
    }
};

class ManagerPackages {
public:
    ManagerPackages() {}

    dataPackage* getReplyDataPackage(dataPackage* recv) {
        dataPackage* ret = new dataPackage();
        ClientManagerPackages* cl_mng = new ClientManagerPackages(recv);
        auto iter = client_managers.find(cl_mng);
        if (iter != client_managers.end()) {
            delete cl_mng;
            cl_mng = *iter;
        }
        else {
            client_managers.insert(cl_mng);
        }

        for (int i = 0; i < 8; i++) {
            ret->id[i] = recv->id[i];
        }
        ret->seq_number = recv->seq_number;
        bool last_pack = cl_mng->isLastPackage(recv);
        ret->seq_total = cl_mng->getCountRecvPackages();
        if (last_pack) {
            ret->data = (std::byte*)getBufCheckSum(cl_mng->getCheckSum(), ret->data_size);
            client_managers.erase(cl_mng);
        }
        ret->type = ret->ack;
        
        return ret;
    }
public:
    std::set<ClientManagerPackages*, ManagerPackagesCmp> client_managers;
};

