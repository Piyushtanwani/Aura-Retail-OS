/*
 * ============================================================================
 *  AURA RETAIL OS — Secure Inventory (Proxy Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Proxy Pattern
 *
 *  SecureInventory acts as a protective proxy around RealInventory.
 *  It intercepts every request to the real inventory and adds:
 *    1. ACCESS LOGGING — every operation is logged with a timestamp
 *    2. VALIDATION — stock checks before purchases, quantity validation
 *    3. SECURITY — controlled access to sensitive inventory operations
 *
 *  The client code interacts with InventoryInterface without knowing
 *  whether it's talking to the real inventory or this proxy.
 *
 * ============================================================================
 */

#ifndef SECURE_INVENTORY_H
#define SECURE_INVENTORY_H

#include "InventoryInterface.h"
#include "RealInventory.h"
#include <memory>

class SecureInventory : public InventoryInterface {
private:
    std::unique_ptr<RealInventory> realInventory_;  // the real subject being proxied

    // --- Proxy responsibilities ---
    void logAccess(const std::string& operation, const std::string& itemId = "") const;
    bool validateItemExists(const std::string& itemId) const;

public:
    SecureInventory();

    // --- InventoryInterface (delegates to RealInventory with added security) ---
    void addItem(std::shared_ptr<InventoryComponent> item, int quantity) override;
    bool isInStock(const std::string& itemId) const override;
    int getStock(const std::string& itemId) const override;
    bool decrementStock(const std::string& itemId, int quantity = 1) override;
    void incrementStock(const std::string& itemId, int quantity = 1) override;
    std::shared_ptr<InventoryComponent> getItem(const std::string& itemId) const override;
    void displayCatalogue() const override;

    std::vector<std::string> getAllItemIds() const override;

    // Direct stock setter for persistence layer
    void setStock(const std::string& itemId, int quantity);
};

#endif // SECURE_INVENTORY_H
