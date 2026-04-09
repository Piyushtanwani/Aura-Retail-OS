/*
 * ============================================================================
 *  AURA RETAIL OS — Food Kiosk (Factory Pattern - Concrete Product)
 * ============================================================================
 *
 *  DESIGN PATTERN: Factory Pattern (Concrete Product)
 *
 *  FoodKiosk is deployed at metro stations and public areas. It specializes
 *  in food items, beverages, and quick essentials. It includes domain-specific
 *  behavior such as expiry date checks and nutritional info display.
 *
 * ============================================================================
 */

#ifndef FOOD_KIOSK_H
#define FOOD_KIOSK_H

#include "Kiosk.h"

class FoodKiosk : public Kiosk {
public:
    FoodKiosk(const std::string& id, const std::string& location)
        : Kiosk(id, "Food Kiosk 🍔", location) {
        std::cout << "🏭 [Factory] Created FoodKiosk [" << id
                  << "] at " << location << "\n";
    }

    void displayInfo() const override {
        Kiosk::displayInfo();
        std::cout << "  🍔 Specialization: Food, Beverages & Quick Essentials\n";
        std::cout << "  📍 Typical Locations: Metro stations, Public areas\n";
    }
};

#endif // FOOD_KIOSK_H
