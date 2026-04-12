/*
 * ============================================================================
 *  AURA RETAIL OS — Kiosk Factory (Factory Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Factory Pattern
 *
 *  KioskFactory centralizes kiosk creation logic. Instead of clients
 *  directly instantiating concrete kiosk classes (FoodKiosk, PharmacyKiosk,
 *  EmergencyKiosk), they request a kiosk by type string and the factory
 *  returns the appropriate instance.
 *
 *  Benefits:
 *    - Decouples client code from concrete classes
 *    - New kiosk types can be added without modifying client code
 *    - Centralizes construction logic and validation
 *
 * ============================================================================
 */

#ifndef KIOSK_FACTORY_H
#define KIOSK_FACTORY_H

#include "FoodKiosk.h"
#include "PharmacyKiosk.h"
#include "EmergencyKiosk.h"
#include <memory>
#include <string>
#include <iostream>

class KioskFactory {
public:
    /*
     * Create a kiosk of the specified type.
     *
     * @param type     "food", "pharmacy", or "emergency"
     * @param id       Unique kiosk identifier
     * @param location Physical deployment location
     * @return         unique_ptr to the created kiosk (nullptr if type unknown)
     */
    static std::unique_ptr<Kiosk> createKiosk(const std::string& type,
                                               const std::string& id,
                                               const std::string& location) {
        std::cout << "\n🏭 ═══════════════════════════════════════════════\n";
        std::cout << "🏭  KIOSK FACTORY — Creating kiosk...\n";
        std::cout << "🏭  Type: " << type << " | ID: " << id << " | Location: " << location << "\n";
        std::cout << "🏭 ═══════════════════════════════════════════════\n";

        if (type == "food") {
            return std::make_unique<FoodKiosk>(id, location);
        }
        else if (type == "pharmacy") {
            return std::make_unique<PharmacyKiosk>(id, location);
        }
        else if (type == "emergency") {
            return std::make_unique<EmergencyKiosk>(id, location);
        }
        else {
            std::cout << "🏭 [Factory] ❌ Unknown kiosk type: \"" << type << "\"\n";
            return nullptr;
        }
    }
};

#endif // KIOSK_FACTORY_H
