#ifndef MEMORYSTRUCTS_H
#define MEMORYSTRUCTS_H

struct intMB;
struct intKB;
struct longB;

// int representing mega bytes
struct intMB {
    intMB() : fValue(0) {}
    explicit intMB(const int value) : fValue(value) {}
    explicit intMB(const longB& b);
    explicit intMB(const intKB& kb);
    int fValue;

    intMB operator-() const {
        return intMB(-fValue);
    }
};

// int representing kilo bytes
struct intKB {
    intKB() : fValue(0) {}
    explicit intKB(const int value) : fValue(value) {}
    explicit intKB(const longB& b);
    explicit intKB(const intMB& mb);
    int fValue;

    intKB operator-() const {
        return intKB(-fValue);
    }
};

// long representing bytes
struct longB {
    longB() : fValue(0) {}
    explicit longB(const long value) : fValue(value) {}
    explicit longB(const intKB& kb);
    explicit longB(const intMB& mb);
    long fValue;

    longB operator-() const {
        return longB(-fValue);
    }
};

bool operator>(const intMB& a, const intMB& b);
bool operator<(const intMB& a, const intMB& b);
intMB operator-(const intMB& a, const intMB& b);
intMB operator+(const intMB& a, const intMB& b);
intMB operator*(const intMB& a, const int& b);
intMB operator/(const intMB& a, const int& b);

bool operator>(const intKB& a, const intKB& b);
bool operator<(const intKB& a, const intKB& b);
intKB operator-(const intKB& a, const intKB& b);
intKB operator+(const intKB& a, const intKB& b);
intKB operator*(const intKB& a, const int& b);
intKB operator/(const intKB& a, const int& b);

bool operator>(const longB& a, const longB& b);
bool operator<(const longB& a, const longB& b);
longB operator-(const longB& a, const longB& b);
longB operator+(const longB& a, const longB& b);
longB operator*(const longB& a, const long& b);
longB operator/(const longB& a, const long& b);

#endif // MEMORYSTRUCTS_H
