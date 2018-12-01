#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <ecosnail/thing.hpp>

#include <string>

using ecosnail::thing::EntityManager;

struct C1 {
    int id;
};

struct C2 {
    int id;
};

TEST_CASE("Simple", "[simple]")
{
    // TODO: test const manager for read-only access
    EntityManager manager;

    auto e1 = manager.createEntity();
    auto e2 = manager.createEntity();
    auto e12 = manager.createEntity();

    manager.add<C1>(e1);
    manager.component<C1>(e1).id = 1;

    manager.add<C2>(e2);
    manager.component<C2>(e2).id = 2;

    manager.add<C1>(e12);
    manager.component<C1>(e12).id = 3;

    manager.add<C2>(e12);
    manager.component<C2>(e12).id = 4;

    for (const C1& component : manager.components<C1>()) {
        REQUIRE(component.id % 2 == 1);
    }
    for (const C2& component : manager.components<C2>()) {
        REQUIRE(component.id % 2 == 0);
    }

    // TODO: entities of const manager

    for (const auto& entity : manager.entities<C1>()) {
        REQUIRE(entity != e2);
    }
    for (const auto& entity : manager.entities<C2>()) {
        REQUIRE(entity != e1);
    }
}

TEST_CASE("Entity creation", "[entities]")
{
    EntityManager manager;

    auto e0 = manager.createEntity();
    auto e1 = manager.createEntity();
    auto e2 = manager.createEntity();

    REQUIRE(e0 == 0);
    REQUIRE(e1 == 1);
    REQUIRE(e2 == 2);

    // TODO: kill entities
}

TEST_CASE("Modify component", "[component]")
{
    EntityManager manager;

    auto entity = manager.createEntity();
    auto& component = manager.add<int>(entity);
    component = 11;

    SECTION("Read value")
    {
        const auto& component = manager.component<int>(entity);
        REQUIRE(component == 11);
    }

    SECTION("Modify value")
    {
        auto& component = manager.component<int>(entity);
        component = 12;

        const auto& another = manager.component<int>(entity);
        REQUIRE(another == 12);
    }
}

TEST_CASE("Get component pack", "[component]")
{
    EntityManager manager;

    auto e1 = manager.createEntity();
    auto e2 = manager.createEntity();
    auto e3 = manager.createEntity();

    manager.add<int>(e1) = 1;
    manager.add<int>(e2) = 2;
    manager.add<std::string>(e2) = "a";
    manager.add<std::string>(e3) = "b";

    auto sum = [] (const EntityManager& manager) {
        int sum = 0;
        for (const auto& i : manager.components<int>()) {
            sum += i;
        }
        return sum;
    };

    REQUIRE(sum(manager) == 3);

    auto concat = [] (const EntityManager& manager) {
        std::string concat;
        for (const auto& s : manager.components<std::string>()) {
            concat += s;
        }
        return concat;
    };

    REQUIRE(concat(manager) == "ab");
}

TEST_CASE("No components", "[component]")
{
    const EntityManager manager;

    int sum = 0;
    for (int value : manager.components<int>()) {
        sum += value;
    }
    REQUIRE(sum == 0);
}
