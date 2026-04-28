/*
 * ============================================================================
 *  AURA RETAIL OS — Real Inventory Implementation
 * ============================================================================
 */

#include "RealInventory.h"
#include <iostream>

void RealInventory::addItem(std::shared_ptr<InventoryComponent> item, int quantity) {
    catalogue_[item->getId()] = item;
    stock_[item->getId()] = quantity;
}

bool RealInventory::isInStock(const std::string& itemId) const {
    auto it = stock_.find(itemId);
    return it != stock_.end() && it->second > 0;
}

int RealInventory::getStock(const std::string& itemId) const {
    auto item = getItem(itemId);
    if (!item) return 0;

    // Logic for Composite (Bundles): Dynamic Calculation
    if (item->isComposite()) {
        auto childIds = item->getChildIds();
        if (childIds.empty()) return 0;

        // Count required quantities for each unique child
        std::unordered_map<std::string, int> counts;
        for (const auto& cid : childIds) counts[cid]++;

        int minStock = -1;
        for (const auto& pair : counts) {
            int avail = getStock(pair.first); // Recursive call
            int possible = avail / pair.second;
            if (minStock == -1 || possible < minStock) {
                minStock = possible;
            }
        }
        return (minStock == -1) ? 0 : minStock;
    }

    // Logic for Leaf (Products): Map Lookup
    auto it = stock_.find(itemId);
    return (it != stock_.end()) ? it->second : 0;
}

bool RealInventory::decrementStock(const std::string& itemId, int quantity) {
    if (getStock(itemId) < quantity) return false;

    auto item = getItem(itemId);
    if (item && item->isComposite()) {
        // Decrement each child recursively
        for (const auto& cid : item->getChildIds()) {
            decrementStock(cid, quantity);
        }
        return true;
    }

    stock_[itemId] -= quantity;
    return true;
}

void RealInventory::incrementStock(const std::string& itemId, int quantity) {
    auto item = getItem(itemId);
    if (item && item->isComposite()) {
        // Increment each child recursively
        for (const auto& cid : item->getChildIds()) {
            incrementStock(cid, quantity);
        }
        return;
    }

    if (stock_.find(itemId) != stock_.end()) {
        stock_[itemId] += quantity;
    }
}

std::shared_ptr<InventoryComponent> RealInventory::getItem(const std::string& itemId) const {
    auto it = catalogue_.find(itemId);
    return (it != catalogue_.end()) ? it->second : nullptr;
}

void RealInventory::displayCatalogue() const {
    std::cout << "\n┌─────────────────────────────────────────────┐\n";
    std::cout << "│          📋 INVENTORY CATALOGUE              │\n";
    std::cout << "└─────────────────────────────────────────────┘\n";
    for (const auto& [id, item] : catalogue_) {
        item->display(1);
        std::cout << "    Stock: " << getStock(id) << " units" 
                  << (item->isComposite() ? " (Calculated from components)" : "") << "\n";
    }
}

std::vector<std::string> RealInventory::getAllItemIds() const {
    std::vector<std::string> ids;
    for (const auto& pair : catalogue_) {
        ids.push_back(pair.first);
    }
    return ids;
}

void RealInventory::setStock(const std::string& itemId, int quantity) {
    if (stock_.find(itemId) != stock_.end()) {
        stock_[itemId] = quantity;
    }
}
