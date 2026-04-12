/*
 * ============================================================================
 *  AURA RETAIL OS — Standard Dispenser
 * ============================================================================
 *
 *  Default dispenser implementation for regular (non-temperature-sensitive)
 *  items. Uses a simple conveyor mechanism to deliver products.
 *
 * ============================================================================
 */

#ifndef STANDARD_DISPENSER_H
#define STANDARD_DISPENSER_H

#include "Dispenser.h"
#include <iostream>

class StandardDispenser : public Dispenser {
public:
    bool dispense(const std::string& itemId, const std::string& itemName) override {
        std::cout << "  ⚙️  [StandardDispenser] Activating conveyor belt...\n";
        if (itemId == "P-ERROR") {
            std::cout << "  ❌ Hardware malfunction detected!\n";
            return false;
        }
        std::cout << "  ⚙️  [StandardDispenser] Dispensing \"" << itemName
                  << "\" [" << itemId << "] via slot mechanism.\n";
        std::cout << "  ⚙️  [StandardDispenser] ✅ Item delivered to pickup tray.\n";
        return true;
    }

    std::string getType() const override {
        return "Standard Conveyor Dispenser";
    }
};

#endif // STANDARD_DISPENSER_H
