#pragma once

#include <ecosnail/thing/entity.hpp>

#include <deque>

namespace ecosnail::thing {

class EntityPool {
public:
    Entity createEntity();
    void killEntity(Entity entity);

private:
    std::deque<Entity> _freeEntities;
    Entity::ValueType _nextEntity = 0;
};

} // namespace ecosnail::thing
