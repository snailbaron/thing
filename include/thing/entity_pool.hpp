#pragma once

#include <thing/entity.hpp>

#include <deque>

namespace thing {

class EntityPool {
public:
    Entity createEntity();
    void killEntity(Entity entity);

private:
    std::deque<Entity> _freeEntities;
    Entity::ValueType _nextEntity = 0;
};

} // namespace thing
