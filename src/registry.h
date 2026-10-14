#ifndef REGISTRY_H
#define REGISTRY_H

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

template <typename T>
class Registry {
public:
    template <typename U>
        requires std::is_constructible_v<T, U&&>
    size_t store(U&& item) {
        size_t id;
        if (!removedIDs.empty()) {
            // check for holes from removed items
            id = removedIDs.back();
            removedIDs.pop_back();
            // recycle the old slot with the new item (forwarding)
            mData[id] = std::forward<U>(item);
            isAlive[id] = true;
        } else {
            // no holes, just add to the end of the vector
            id = mData.size();
            mData.push_back(std::forward<U>(item));
            isAlive.push_back(true);
        }
        return id;
    }

    void remove(size_t id) {
        assert(valid(id) && "Registry::remove on an invalid ticket");
        mData[id] = T{};
        isAlive[id] = false;
        removedIDs.push_back(id);  // remember this for recycling
    }
    T& operator[](size_t id) {
        assert(valid(id) && "Registry::operator[] on an invalid ticket");
        return mData[id];
    }
    const T& operator[](size_t id) const {
        assert(valid(id) && "Registry::operator[] on an invalid ticket");
        return mData[id];
    }

    // true if the given ID is valid and points to a live component, false otherwise
    bool valid(size_t id) const { return id < isAlive.size() && isAlive[id]; }
    // number of live components in the registry (not counting removed ones)
    size_t size() const { return mData.size() - removedIDs.size(); }

    void reserve(size_t capacity) {
        mData.reserve(capacity);
        isAlive.reserve(capacity);
    }

private:
    std::vector<T> mData;  // storage for the components
    std::vector<bool>
        isAlive;  // flags for whether the component at a given index is alive or has been removed
    std::vector<size_t> removedIDs;  // stack of removed IDs to recycle
};

#endif  // REGISTRY_H
