/*
 * ============================================================================
 *  AURA RETAIL OS — Product (Composite Pattern - Leaf)
 * ============================================================================
 *
 *  DESIGN PATTERN: Composite Pattern (Leaf)
 *
 *  Product represents an individual, indivisible item in the inventory.
 *  It is a leaf node in the Composite tree — it cannot contain children.
 *  Examples: a single medicine, a snack, a battery pack.
 *
 * ============================================================================
 */

#ifndef PRODUCT_H
#define PRODUCT_H

#include "InventoryComponent.h"

class Product : public InventoryComponent {
private:
    std::string id_;
    std::string name_;
    double price_;
    bool needsRefrigeration_;

public:
    Product(const std::string& id,
            const std::string& name,
            double price,
            bool needsRefrigeration = false)
        : id_(id), name_(name), price_(price),
          needsRefrigeration_(needsRefrigeration) {}

    // --- InventoryComponent interface ---

    std::string getId() const override { return id_; }
    std::string getName() const override { return name_; }
    double getPrice() const override { return price_; }

    bool requiresRefrigeration() const override {
        return needsRefrigeration_;
    }

    std::string toJson(int stock) const override {
        return "      {\"id\": \"" + id_ + "\", \"name\": \"" + name_ + 
               "\", \"price\": " + std::to_string(price_) + 
               ", \"stock\": " + std::to_string(stock) + "}";
    }

    void display(int indent = 0) const override {
        printIndent(indent);
        std::cout << "📦 Product [" << id_ << "] " << name_
                  << " — ₹" << price_;
        if (needsRefrigeration_) std::cout << " ❄️ (requires refrigeration)";
        std::cout << "\n";
    }
};

#endif // PRODUCT_H
