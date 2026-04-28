/*
 * ============================================================================
 *  AURA RETAIL OS — Refrigeration Module (Decorator Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Decorator Pattern (Concrete Decorator)
 *
 *  Adds temperature monitoring and pre-purchase safety checks. If an item
 *  requires refrigeration, this module checks the temperature sensor before
 *  allowing the purchase to proceed.
 *
 * ============================================================================
 */

#ifndef REFRIGERATION_MODULE_H
#define REFRIGERATION_MODULE_H

#include "KioskDecorator.h"
#include <iostream>

class RefrigerationModule : public KioskDecorator {
private:
    double currentTemp_;

public:
    explicit RefrigerationModule(std::unique_ptr<Kiosk> wrappee, double temp = 4.0)
        : KioskDecorator(std::move(wrappee)), currentTemp_(temp) {
        std::cout << "  🎨 [Decorator] Attached Refrigeration Module (Temp: "
                  << currentTemp_ << "°C) to Kiosk " << kioskId_ << "\n";
    }

    Transaction purchaseItem(const std::string& itemId, int quantity, const std::string& phone) override {
        std::cout << "  ❄️  [RefrigerationModule] Pre-purchase safety check...\n";
        std::cout << "  ❄️  [RefrigerationModule] Current temperature is " << currentTemp_ << "°C.\n";

        // Let the wrapped kiosk handle the actual purchase
        return KioskDecorator::purchaseItem(itemId, quantity, phone);
    }

    std::string describeModules() const override {
        std::string base = wrappee_->describeModules();
        std::string me = "   - Refrigeration Unit (" + std::to_string((int)currentTemp_) + "°C)";
        return (base == "None" || base.empty()) ? me : base + "\n" + me;
    }
};

#endif // REFRIGERATION_MODULE_H
