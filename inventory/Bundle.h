/*
 * ============================================================================
 *  AURA RETAIL OS — Bundle (Composite Pattern - Composite)
 * ============================================================================
 *
 *  DESIGN PATTERN: Composite Pattern (Composite)
 *
 *  Bundle is a composite node that can contain both Products (leaves)
 *  and other Bundles (sub-composites). This enables nested, hierarchical
 *  inventory structures. The price of a bundle is the sum of all its
 *  children's prices, with an optional discount applied.
 *
 *  Example:  "Emergency Kit" bundle containing:
 *              - First Aid Sub-Bundle (bandages, antiseptic)
 *              - Flashlight (product)
 *              - Water Bottle (product)
 *
 * ============================================================================
 */

#ifndef BUNDLE_H
#define BUNDLE_H

#include "InventoryComponent.h"
#include <vector>
#include <algorithm>

class Bundle : public InventoryComponent {
private:
    std::string id_;
    std::string name_;
    double discountPercent_;                                    // bundle discount
    std::vector<std::shared_ptr<InventoryComponent>> children_; // products or sub-bundles

public:
    Bundle(const std::string& id,
           const std::string& name,
           double discountPercent = 0.0)
        : id_(id), name_(name), discountPercent_(discountPercent) {}

    // --- InventoryComponent interface ---

    std::string getId() const override { return id_; }
    std::string getName() const override { return name_; }

    // Price = sum of children minus discount
    double getPrice() const override {
        double total = 0.0;
        for (const auto& child : children_) {
            total += child->getPrice();
        }
        return total * (1.0 - discountPercent_ / 100.0);
    }

    bool requiresRefrigeration() const override {
        return std::any_of(children_.begin(), children_.end(),
            [](const std::shared_ptr<InventoryComponent>& c) {
                return c->requiresRefrigeration();
            });
    }

    std::string toJson(int stock) const override {
        std::string json = "      {\"id\": \"" + id_ + "\", \"name\": \"" + name_ + "\", \"items\": [";
        for (size_t i = 0; i < children_.size(); ++i) {
            json += "\"" + children_[i]->getId() + "\"";
            if (i < children_.size() - 1) json += ", ";
        }
        json += "], \"stock\": " + std::to_string(stock) + "}";
        return json;
    }

    // --- Composite operations ---

    void add(std::shared_ptr<InventoryComponent> component) override {
        children_.push_back(component);
        std::cout << "  ✅ Added \"" << component->getName()
                  << "\" to bundle \"" << name_ << "\"\n";
    }

    void remove(const std::string& id) override {
        auto it = std::remove_if(children_.begin(), children_.end(),
            [&id](const std::shared_ptr<InventoryComponent>& c) {
                return c->getId() == id;
            });
        if (it != children_.end()) {
            std::cout << "  ❌ Removed item [" << id << "] from bundle \"" << name_ << "\"\n";
            children_.erase(it, children_.end());
        }
    }

    std::vector<std::string> getChildIds() const override {
        std::vector<std::string> ids;
        for (const auto& child : children_) {
            ids.push_back(child->getId());
        }
        return ids;
    }

    void display(int indent = 0) const override {
        printIndent(indent);
        std::cout << "📦 Bundle [" << id_ << "] " << name_
                  << " (discount: " << discountPercent_ << "%) — Total: ₹"
                  << getPrice() << "\n";
        for (const auto& child : children_) {
            child->display(indent + 1);
        }
    }

    const std::vector<std::shared_ptr<InventoryComponent>>& getChildren() const {
        return children_;
    }
};

#endif // BUNDLE_H
