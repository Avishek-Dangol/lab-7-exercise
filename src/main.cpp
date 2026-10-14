#include <cassert>
#include <iostream>
#include <string>

#include "entity_manager.h"

int main() {
    // registry check
    Registry<int> reg;
    size_t a = reg.store(10);
    size_t b = reg.store(20);
    assert(a == 0 && b == 1);
    assert(reg[a] == 10);     // operator[] returns what we stored
    assert(reg.size() == 2);  // size counts stored items
    reg.remove(a);
    assert(!reg.valid(a));    // removed ticket is no longer valid
    assert(reg.size() == 1);  // check if size went down
    size_t c = reg.store(30);
    assert(c == a);  // removed ticket gets recycled
    assert(reg[c] == 30);
    std::cout << "Registry Check Passed\n";

    EntityManager<int, float, bool> entityManager;
    EntityID e = entityManager.createEntity();
    assert(e == 0);                         // first entity ID is 0
    assert(entityManager.validEntity(e));   // created entity is valid
    assert(!entityManager.validEntity(5));  // non-existent entity is invalid
    entityManager.addComponent(e, 5);
    std::cout << entityManager.hasComponent<int>(e) << '\n';    // prints 1
    std::cout << entityManager.hasComponent<float>(e) << '\n';  // prints 0
    std::cout << entityManager.hasComponent<bool>(e) << '\n';   // prints 0
    // std::cout << entityManager.getComponent<bool>(e) << '\n';   // should fail an assertion
    //  std::cout << entityManager.getComponent<int>(e) << '\n';    // prints 5
    //   a second entity gets its own components
    EntityID e2 = entityManager.createEntity();
    entityManager.addComponent(e2, 2.5f);
    assert(e2 == 1);
    assert(entityManager.getComponent<float>(e2) == 2.5f);
    assert(!entityManager.hasComponent<int>(e2));  // e2 did not get e's int
    std::cout << "EntityManager Check Passed\n";
    return 0;
}
