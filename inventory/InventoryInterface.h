/*
 * ============================================================================
 *  AURA RETAIL OS — Inventory Interface (Abstract Class)
 * ============================================================================
 *
 *  This abstract class defines the contract for all inventory implementations.
 *  Both RealInventory and SecureInventory (Proxy) implement this interface,
 *  ensuring the system uses polymorphism to interact with inventory without
 *  knowing whether it's the real one or the proxied/secured version.
 *
 * ============================================================================
 */

#ifndef INVENTORY_INTERFACE_H
#define INVENTORY_INTERFACE_H

#include "InventoryComponent.h"
#include <memory>
#include <vector>
#include <string>

class InventoryInterface {
public:
    virtual ~InventoryInterface() = default;

    // Add a product/bundle to the catalogue
    virtual void addItem(std::shared_ptr<InventoryComponent> item, int quantity) = 0;

    // Check if an item is in stock
    virtual bool isInStock(const std::string& itemId) const = 0;

    // Get the stock count for an item
    virtual int getStock(const std::string& itemId) const = 0;

    // Decrease stock by 1 (for purchase)
    virtual bool decrementStock(const std::string& itemId) = 0;

    // Increase stock (for restock / refund)
    virtual void incrementStock(const std::string& itemId, int quantity = 1) = 0;

    // Retrieve item details
    virtual std::shared_ptr<InventoryComponent> getItem(const std::string& itemId) const = 0;

    // Display the full catalogue
    virtual void displayCatalogue() const = 0;

    virtual std::vector<std::string> getAllItemIds() const = 0;
};

#endif // INVENTORY_INTERFACE_H
