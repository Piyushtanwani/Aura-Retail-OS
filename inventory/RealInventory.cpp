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
    auto it = stock_.find(itemId);
    return (it != stock_.end()) ? it->second : 0;
}

bool RealInventory::decrementStock(const std::string& itemId, int quantity) {
    if (getStock(itemId) >= quantity) {
        stock_[itemId] -= quantity;
        return true;
    }
    return false;
}

void RealInventory::incrementStock(const std::string& itemId, int quantity) {
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
        std::cout << "    Stock: " << stock_.at(id) << " units\n";
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
