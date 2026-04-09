/*
 * ============================================================================
 *  AURA RETAIL OS — Secure Inventory Implementation (Proxy Pattern)
 * ============================================================================
 *
 *  Every method logs the operation and validates input before delegating
 *  to the underlying RealInventory. This demonstrates the Proxy pattern's
 *  ability to add cross-cutting concerns (logging, validation, access control)
 *  without modifying the real subject.
 *
 * ============================================================================
 */

#include "SecureInventory.h"
#include <iostream>
#include <ctime>
#include <iomanip>

// ─── Constructor ────────────────────────────────────────────────────────────
SecureInventory::SecureInventory()
    : realInventory_(std::make_unique<RealInventory>()) {
    std::cout << "🛡️  [SecureInventory] Proxy initialized. All access is monitored.\n";
}

// ─── Private: Log access with timestamp ─────────────────────────────────────
void SecureInventory::logAccess(const std::string& operation, const std::string& itemId) const {
    std::time_t now = std::time(nullptr);
    std::tm* lt = std::localtime(&now);
    std::cout << "🛡️  [SecureInventory] ["
              << std::put_time(lt, "%H:%M:%S") << "] "
              << operation;
    if (!itemId.empty()) std::cout << " (item: " << itemId << ")";
    std::cout << "\n";
}

// ─── Private: Check if item exists ──────────────────────────────────────────
bool SecureInventory::validateItemExists(const std::string& itemId) const {
    if (realInventory_->getItem(itemId) == nullptr) {
        std::cout << "🛡️  [SecureInventory] ⚠️  Validation FAILED: Item \""
                  << itemId << "\" not found in catalogue.\n";
        return false;
    }
    return true;
}

// ─── addItem ────────────────────────────────────────────────────────────────
void SecureInventory::addItem(std::shared_ptr<InventoryComponent> item, int quantity) {
    logAccess("ADD_ITEM", item->getId());
    if (quantity <= 0) {
        std::cout << "🛡️  [SecureInventory] ⚠️  Validation FAILED: Quantity must be > 0.\n";
        return;
    }
    realInventory_->addItem(item, quantity);
    std::cout << "🛡️  [SecureInventory] ✅ Item \"" << item->getName()
              << "\" added with " << quantity << " units.\n";
}

// ─── isInStock ──────────────────────────────────────────────────────────────
bool SecureInventory::isInStock(const std::string& itemId) const {
    logAccess("CHECK_STOCK", itemId);
    return realInventory_->isInStock(itemId);
}

// ─── getStock ───────────────────────────────────────────────────────────────
int SecureInventory::getStock(const std::string& itemId) const {
    logAccess("GET_STOCK", itemId);
    return realInventory_->getStock(itemId);
}

// ─── decrementStock ─────────────────────────────────────────────────────────
bool SecureInventory::decrementStock(const std::string& itemId) {
    logAccess("DECREMENT_STOCK", itemId);

    if (!validateItemExists(itemId)) return false;

    if (!realInventory_->isInStock(itemId)) {
        std::cout << "🛡️  [SecureInventory] ❌ DENIED: Item \"" << itemId
                  << "\" is OUT OF STOCK. Cannot decrement.\n";
        return false;
    }

    bool result = realInventory_->decrementStock(itemId);
    if (result) {
        std::cout << "🛡️  [SecureInventory] ✅ Stock decremented. Remaining: "
                  << realInventory_->getStock(itemId) << "\n";
    }
    return result;
}

// ─── incrementStock ─────────────────────────────────────────────────────────
void SecureInventory::incrementStock(const std::string& itemId, int quantity) {
    logAccess("INCREMENT_STOCK", itemId);

    if (!validateItemExists(itemId)) return;
    if (quantity <= 0) {
        std::cout << "🛡️  [SecureInventory] ⚠️  Validation FAILED: Restock quantity must be > 0.\n";
        return;
    }

    realInventory_->incrementStock(itemId, quantity);
    std::cout << "🛡️  [SecureInventory] ✅ Restocked " << quantity
              << " units. New stock: " << realInventory_->getStock(itemId) << "\n";
}

// ─── getItem ────────────────────────────────────────────────────────────────
std::shared_ptr<InventoryComponent> SecureInventory::getItem(const std::string& itemId) const {
    logAccess("GET_ITEM", itemId);
    return realInventory_->getItem(itemId);
}

// ─── displayCatalogue ───────────────────────────────────────────────────────
void SecureInventory::displayCatalogue() const {
    std::cout << "[SecureInventory] Processing request: displayCatalogue()\n";
    realInventory_->displayCatalogue();
}

std::vector<std::string> SecureInventory::getAllItemIds() const {
    return realInventory_->getAllItemIds();
}
