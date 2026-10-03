#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

namespace smartwardrobe {

class Clothing {
public:
    using TagList = std::vector<std::string>;

    Clothing(int id, std::string subtype, std::string color,
             TagList occasions, TagList seasons);
    virtual ~Clothing();

    Clothing(const Clothing&) = delete;
    Clothing& operator=(const Clothing&) = delete;
    Clothing(Clothing&&) = delete;
    Clothing& operator=(Clothing&&) = delete;

    int getId() const;
    const std::string& getSubtype() const;
    const std::string& getColor() const;
    const TagList& getOccasions() const;
    const TagList& getSeasons() const;

    virtual std::string getType() const = 0;

    static std::size_t liveCount();

    friend std::ostream& operator<<(std::ostream& output, const Clothing& clothing);

private:
    int id_;
    std::string subtype_;
    std::string color_;
    TagList occasions_;
    TagList seasons_;

    static std::size_t liveCount_;
};

} // namespace smartwardrobe