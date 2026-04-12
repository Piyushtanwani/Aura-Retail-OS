/*
 * ============================================================================
 *  AURA RETAIL OS — Refrigerated Dispenser
 * ============================================================================
 *
 *  Specialized dispenser for temperature-sensitive items. Maintains cold
 *  chain integrity during dispensing. Used for medicines, perishable food, etc.
 *
 * ============================================================================
 */

#ifndef REFRIGERATED_DISPENSER_H
#define REFRIGERATED_DISPENSER_H

#include "Dispenser.h"
#include <iostream>

class RefrigeratedDispenser : public Dispenser {
private:
    double temperature_;  // operating temperature in Celsius

public:
    explicit RefrigeratedDispenser(double temp = 4.0)
        : temperature_(temp) {}

    bool dispense(const std::string& itemId, const std::string& itemName) override {
        std::cout << "  ❄️  [RefrigeratedDispenser] Cold chamber at " << temperature_ << "°C.\n";
        std::cout << "  ❄️  [RefrigeratedDispenser] Opening insulated compartment...\n";
        std::cout << "  ❄️  [RefrigeratedDispenser] Dispensing \"" << itemName
                  << "\" [" << itemId << "] with cold-chain maintained.\n";
        std::cout << "  ❄️  [RefrigeratedDispenser] ✅ Item delivered. Cold seal intact.\n";
        return true;
    }

    std::string getType() const override {
        return "Refrigerated Dispenser (" + std::to_string((int)temperature_) + "°C)";
    }
};

#endif // REFRIGERATED_DISPENSER_H
