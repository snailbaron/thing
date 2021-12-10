#pragma once

#include <thing/entity.hpp>
#include <thing/entity_pool.hpp>

#include <any>
#include <cassert>
#include <map>
#include <memory>
#include <optional>
#include <ranges>
#include <type_traits>
#include <typeindex>
#include <vector>

namespace thing {

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
            static const auto empty = std::map<Entity, Component>{};
            return std::views::values(empty);
        }

        const auto& componentMap =
            std::any_cast<const ComponentMap&>(it->second);
        return std::views::values(componentMap);
    }

    template <class Component>
    auto components()
    {
        using MapRef = std::map<Entity, Component>&;

        auto it = _components.find(typeid(Component));
        if (it == _components.end()) {
            static auto empty = std::map<Entity, Component>{};
            return std::views::values(empty);
        }

        auto& componentMap = std::any_cast<MapRef>(it->second);
        return std::views::values(componentMap);
    }

    template <class Component>
    auto entities() const
    {
        using MapRef = const std::map<Entity, Component>&;

        auto it = _components.find(typeid(Component));
        if (it == _components.end()) {
            static const auto empty = std::map<Entity, Component>{};
            return std::views::keys(empty);
        }

        const auto& componentMap =
            std::any_cast<const std::map<Entity, Component>&>(it->second);
        return std::views::keys(componentMap);
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

} // namespace thing
