/*
 * ============================================================================
 *  AURA RETAIL OS — Real Inventory (Concrete Implementation)
 * ============================================================================
 *
 *  This is the actual inventory storage. It manages a catalogue of items
 *  (InventoryComponents) and their stock counts. The SecureInventory proxy
 *  wraps this class to add access control and logging.
 *
 * ============================================================================
 */

#ifndef REAL_INVENTORY_H
#define REAL_INVENTORY_H

#include "InventoryInterface.h"
#include <unordered_map>
#include <vector>

class RealInventory : public InventoryInterface {
private:
    // Item ID → InventoryComponent pointer
    std::unordered_map<std::string, std::shared_ptr<InventoryComponent>> catalogue_;
    // Item ID → stock count
    std::unordered_map<std::string, int> stock_;

public:
    RealInventory() = default;

    void addItem(std::shared_ptr<InventoryComponent> item, int quantity) override;
    bool isInStock(const std::string& itemId) const override;
    int getStock(const std::string& itemId) const override;
    bool decrementStock(const std::string& itemId, int quantity = 1) override;
    void incrementStock(const std::string& itemId, int quantity = 1) override;
    std::shared_ptr<InventoryComponent> getItem(const std::string& itemId) const override;
    void displayCatalogue() const override;

    std::vector<std::string> getAllItemIds() const override;
};

#endif // REAL_INVENTORY_H
