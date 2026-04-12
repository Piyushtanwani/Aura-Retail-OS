/*
 * ============================================================================
 *  AURA RETAIL OS — Kiosk Decorator Base Class (Decorator Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Decorator Pattern (Decorator Base)
 *
 *  The Decorator pattern is used to attach optional hardware modules
 *  or software capabilities to a kiosk dynamically at runtime, without
 *  modifying the base Kiosk classes.
 *
 *  KioskDecorator 'is-a' Kiosk and 'has-a' Kiosk. It implements the
 *  Kiosk interface by delegating all calls to the wrapped Kiosk object.
 *  Subclasses (concrete decorators) will override specific methods to
 *  add their own behavior before or after delegation.
 *
 * ============================================================================
 */

#ifndef KIOSK_DECORATOR_H
#define KIOSK_DECORATOR_H

#include "Kiosk.h"
#include <memory>
#include <stdexcept>

class KioskDecorator : public Kiosk {
protected:
    std::unique_ptr<Kiosk> wrappee_;

public:
    // Takes ownership of the kiosk being decorated
    explicit KioskDecorator(std::unique_ptr<Kiosk> wrappee)
        : Kiosk(wrappee->getId(), wrappee->getType(), wrappee->getLocation()),
          wrappee_(std::move(wrappee)) {
        if (!wrappee_) {
            throw std::invalid_argument("Cannot decorate a null kiosk");
        }
    }

    // ─── Delegate Core Operations to Wrappee ────────────────────────────────
    SecureInventory* getInventory() override {
        return wrappee_->getInventory();
    }

    void addProduct(std::shared_ptr<InventoryComponent> item, int qty) override {
        wrappee_->addProduct(item, qty);
    }

    void setPaymentStrategy(std::unique_ptr<PaymentProcessor> strategy) override {
        wrappee_->setPaymentStrategy(std::move(strategy));
    }

    void setDispenser(std::unique_ptr<Dispenser> disp) override {
        wrappee_->setDispenser(std::move(disp));
    }

    std::string getDispenserType() const override {
        return wrappee_->getDispenserType();
    }

    Transaction purchaseItem(const std::string& itemId, int quantity = 1) override {
        return wrappee_->purchaseItem(itemId, quantity);
    }

    bool refundTransaction(const Transaction& tx) override {
        return wrappee_->refundTransaction(tx);
    }

    void restockInventory(const std::string& itemId, int quantity) override {
        wrappee_->restockInventory(itemId, quantity);
    }

    bool dispenseItem(const std::string& itemId, const std::string& itemName) override {
        return wrappee_->dispenseItem(itemId, itemName);
    }

    void displayInfo() const override {
        wrappee_->displayInfo();
    }

    std::string describeModules() const override {
        return wrappee_->describeModules();
    }
};

#endif // KIOSK_DECORATOR_H
