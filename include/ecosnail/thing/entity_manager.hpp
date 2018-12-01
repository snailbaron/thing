#pragma once

#include <ecosnail/thing/entity.hpp>
#include <ecosnail/thing/entity_pool.hpp>

#include <ecosnail/tail.hpp>

#include <any>
#include <cassert>
#include <map>
#include <memory>
#include <optional>
#include <type_traits>
#include <typeindex>
#include <vector>

namespace ecosnail::thing {

class EntityManager {
public:
    template <class Component>
    const Component& component(Entity entity) const
    {
        assert(_components.count(typeid(Component)));
        const auto& componentMap =
            std::any_cast<const std::map<Entity, Component>&>(
                _components.at(typeid(Component)));

        assert(componentMap.count(entity));
        return componentMap.at(entity);
    }

    template <class Component>
    Component& component(Entity entity)
    {
        assert(_components.count(typeid(Component)));
        auto& componentMap =
            std::any_cast<std::map<Entity, Component>&>(
                _components.at(typeid(Component)));

        assert(componentMap.count(entity));
        return componentMap.at(entity);
    }

    template <class Component>
    auto components() const
    {
        using ComponentMap = std::map<Entity, Component>;

        auto it = _components.find(typeid(Component));
        if (it == _components.end()) {
            return tail::valueRange<const ComponentMap>();
        }

        const auto& componentMap =
            std::any_cast<const ComponentMap&>(it->second);
        return tail::valueRange(componentMap);
    }

    template <class Component>
    auto components()
    {
        using MapRef = std::map<Entity, Component>&;

        auto it = _components.find(typeid(Component));
        if (it == _components.end()) {
            return tail::valueRange<MapRef>();
        }

        auto& componentMap = std::any_cast<MapRef>(it->second);
        return tail::valueRange(componentMap);
    }

    template <class Component>
    auto entities() const
    {
        using MapRef = const std::map<Entity, Component>&;

        auto it = _components.find(typeid(Component));
        if (it == _components.end()) {
            return tail::keyRange<MapRef>();
        }

        const auto& componentMap =
            std::any_cast<const std::map<Entity, Component>&>(it->second);
        return tail::keyRange(componentMap);
    }

    template <class Component>
    auto& add(Entity entity)
    {
        static_assert(
            std::is_default_constructible<Component>(),
            "Component is not default-constructible");
        return add<Component>(entity, Component{});
    }

    template <class Component>
    auto& add(Entity entity, Component&& component)
    {
        using ComponentMap = std::map<Entity, Component>;

        std::type_index typeIndex(typeid(Component));
        auto it = _components.find(typeIndex);
        if (it == _components.end()) {
            it = _components.insert({typeIndex, ComponentMap()}).first;
        }

        auto& componentMap = std::any_cast<ComponentMap&>(it->second);
        assert(!componentMap.count(entity));
        return componentMap.insert(
            {entity, std::forward<Component>(component)}).first->second;
    }

    Entity createEntity()
    {
        return _entityPool.createEntity();
    }

    void killEntity(Entity entity)
    {
        _entityPool.killEntity(entity);
    }

private:
    EntityPool _entityPool;
    std::map<std::type_index, std::any> _components;
};

} // namespace ecosnail::thing
