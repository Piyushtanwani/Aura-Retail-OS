/*
 * ============================================================================
 *  AURA RETAIL OS — Inventory Component (Composite Pattern - Component)
 * ============================================================================
 *
 *  DESIGN PATTERN: Composite Pattern (Component Interface)
 *
 *  This is the abstract base class for the Composite pattern.
 *  It defines a uniform interface for both individual products (leaves)
 *  and product bundles (composites), allowing clients to treat them
 *  identically. This enables building tree-structured inventories where
 *  bundles can contain products or other bundles recursively.
 *
 * ============================================================================
 */

#ifndef INVENTORY_COMPONENT_H
#define INVENTORY_COMPONENT_H

#include <string>
#include <iostream>
#include <memory>
#include <vector>

class InventoryComponent {
public:
    virtual ~InventoryComponent() = default;

    // --- Core accessors (must be implemented by all components) ---
    virtual std::string getId() const = 0;
    virtual std::string getName() const = 0;
    virtual double getPrice() const = 0;

    // --- Serialization ---
    virtual std::string toJson(int stock) const = 0;

    // --- Display the component details (indented for hierarchy) ---
    virtual void display(int indent = 0) const = 0;

    // --- Composite operations (default: not supported for leaves) ---
    virtual void add(std::shared_ptr<InventoryComponent> /*component*/) {
        std::cout << "[InventoryComponent] add() not supported on this element.\n";
    }

    virtual void remove(const std::string& /*id*/) {
        std::cout << "[InventoryComponent] remove() not supported on this element.\n";
    }

    virtual std::vector<std::string> getChildIds() const {
        return {};
    }

    // Check if this is a composite (Bundle) or a leaf (Product)
    virtual bool isComposite() const { return false; }

    // --- Check if this component requires refrigeration ---
    virtual bool requiresRefrigeration() const { return false; }

protected:
    // Helper: print indentation for display hierarchy
    void printIndent(int indent) const {
        for (int i = 0; i < indent; ++i) std::cout << "  ";
    }
};

#endif // INVENTORY_COMPONENT_H
