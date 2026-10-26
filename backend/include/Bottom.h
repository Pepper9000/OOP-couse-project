#pragma once

#include "Clothing.h"

namespace smartwardrobe {

class Bottom : public Clothing {
public:
    std::string getType() const final;

protected:
    Bottom(int id, std::string subtype, std::string color, TagList occasions, TagList seasons);
};

class Jeans : public Bottom {
public:
    Jeans(int id, std::string color, TagList occasions, TagList seasons);
};

class Cargo : public Bottom {
public:
    Cargo(int id, std::string color, TagList occasions, TagList seasons);
};

class Trousers : public Bottom {
public:
    Trousers(int id, std::string color, TagList occasions, TagList seasons);
};

class Sweatpants : public Bottom {
public:
    Sweatpants(int id, std::string color, TagList occasions, TagList seasons);
};

} // namespace smartwardrobe