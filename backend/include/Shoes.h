#pragma once

#include "Clothing.h"

namespace smartwardrobe {

class Shoes : public Clothing {
public:
    std::string getType() const final;

protected:
    Shoes(int id, std::string subtype, std::string color,
          TagList occasions, TagList seasons);
};

class Sneakers : public Shoes {
public:
    Sneakers(int id, std::string color, TagList occasions, TagList seasons);
};

class Boots : public Shoes {
public:
    Boots(int id, std::string color, TagList occasions, TagList seasons);
};

class FormalShoes : public Shoes {
public:
    FormalShoes(int id, std::string color, TagList occasions, TagList seasons);
};

} // namespace smartwardrobe