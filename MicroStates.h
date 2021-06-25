#pragma once

// compatablility
#ifndef _WIN32
#include <inttypes.h>
typedef int64_t __int64;
#endif // _WIN32

class MicroStates
{
    int* array; int x, y;
public:
    MicroStates(int x, int y);
    MicroStates(const MicroStates& other);
    ~MicroStates();
    int& at(const int& i, const int& j);
    int& at(const int& i, const int& j) const;
    int& at_edge(const int& i, const int& j) const;
    void flip(const int& i, const int& j);
    int M();
    int E();
    int EnergyChange(const int& i, const int& j) const;
protected:
    int index(const int& i, const int& j) const;
};

inline MicroStates::MicroStates(int x, int y)
    : x(x), y(y) {
        array = new int[static_cast<unsigned __int64>(x)
            * static_cast<unsigned __int64>(y)];
    }

inline MicroStates::MicroStates(const MicroStates& other) {
    this->x = other.x;
    this->y = other.y;
    this->array = new int[static_cast<unsigned __int64>(x)
        * static_cast<unsigned __int64>(y)];
    for (int i = 0; i != x; ++i)
        for (int j = 0; j != y; ++j)
            this->at(i, j) = other.at(i, j);
}

inline MicroStates::~MicroStates() { delete[] array; }

inline int& MicroStates::at(const int& i, const int& j)
{ return array[index(i, j)]; }

inline int& MicroStates::at(const int& i, const int& j) const
{ return array[index(i, j)]; }

inline void MicroStates::flip(const int& i, const int& j)
{ this->at(i, j) *= -1; }

inline int MicroStates::M() {
    int ret = 0;
    for (int i = 0; i != x; ++i)
        for (int j = 0; j != y; ++j)
            ret += this->at(i, j);
    return ret;
}

inline int MicroStates::E() {
    int ret = 0;
    for (int i = 0; i != x; ++i)
        for (int j = 0; j != y; ++j)
            ret -= (this->at_edge(i + 1, j) +
                    this->at_edge(i - 1, j) +
                    this->at_edge(i, j + 1) +
                    this->at_edge(i, j - 1)  ) * this->at(i, j);
    return ret / 2;
}

inline int MicroStates::EnergyChange(const int& i, const int& j) const {
    int ret = this->at_edge(i + 1, j) +
        this->at_edge(i - 1, j) +
        this->at_edge(i, j + 1) +
        this->at_edge(i, j - 1);
    ret *= this->at(i, j) * 2;
    return ret;
}

inline int& MicroStates::at_edge(const int& i, const int& j) const
{ return array[index((i + x) % x, (j + y) % y)]; }

inline int MicroStates::index(const int& i, const int& j) const
{ return x * i + j; }
