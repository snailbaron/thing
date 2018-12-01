#include <ecosnail/thing/entity_pool.hpp>

#include <cassert>
#include <algorithm>

namespace ecosnail::thing {

Entity EntityPool::createEntity()
{
    if (!_freeEntities.empty()) {
        auto result = _freeEntities.front();
        _freeEntities.pop_front();
        return result;
    } else {
        return Entity{_nextEntity++};
    }
}

void EntityPool::killEntity(Entity entity)
{
    // WARNING: slow assertion
    assert(Entity{_nextEntity} > entity &&
        std::find(
            _freeEntities.begin(),
            _freeEntities.end(),
            entity) == _freeEntities.end());

    _freeEntities.push_back(entity);
}

} // namespace ecosnail::thing
