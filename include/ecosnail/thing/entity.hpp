#pragma once

#include <cstdint>

namespace ecosnail::thing {

class Entity {
public:
    using ValueType = uint64_t;

    explicit Entity(ValueType id) : _id(id) {}

    operator ValueType() const { return _id; }

    friend bool operator==(Entity lhs, Entity rhs)
    {
        return lhs._id == rhs._id;
    }

    friend bool operator!=(Entity lhs, Entity rhs)
    {
        return lhs._id != rhs._id;
    }

    friend bool operator<(Entity lhs, Entity rhs)
    {
        return lhs._id < rhs._id;
    }

    friend bool operator>(Entity lhs, Entity rhs)
    {
        return lhs._id > rhs._id;
    }

    friend bool operator<=(Entity lhs, Entity rhs)
    {
        return lhs._id <= rhs._id;
    }

    friend bool operator>=(Entity lhs, Entity rhs)
    {
        return lhs._id >= rhs._id;
    }

private:
    ValueType _id;
};

} // namespace ecosnail::thing
