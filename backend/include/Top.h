#pragma once

#include "Clothing.h"

namespace smartwardrobe {

class Top : public Clothing {
public:
    std::string getType() const final;

protected:
    Top(int id, std::string subtype, std::string color,
        TagList occasions, TagList seasons);
};

class TShirt : public Top {
public:
    TShirt(int id, std::string color, TagList occasions, TagList seasons);
};

class Shirt : public Top {
public:
    Shirt(int id, std::string color, TagList occasions, TagList seasons);
};

class Hoodie : public Top {
public:
    Hoodie(int id, std::string color, TagList occasions, TagList seasons);
};

class Sweater : public Top {
public:
    Sweater(int id, std::string color, TagList occasions, TagList seasons);
};

} // namespace smartwardrobe