/*
 * ============================================================================
 *  AURA RETAIL OS — Pharmacy Kiosk (Factory Pattern - Concrete Product)
 * ============================================================================
 *
 *  DESIGN PATTERN: Factory Pattern (Concrete Product)
 *
 *  PharmacyKiosk is deployed at hospitals and clinics. It specializes
 *  in medicines, first-aid supplies, and health products. It enforces
 *  prescription checks and temperature-controlled storage for medications.
 *
 * ============================================================================
 */

#ifndef PHARMACY_KIOSK_H
#define PHARMACY_KIOSK_H

#include "Kiosk.h"

class PharmacyKiosk : public Kiosk {
public:
    PharmacyKiosk(const std::string& id, const std::string& location)
        : Kiosk(id, "Pharmacy Kiosk 💊", location) {
        std::cout << "🏭 [Factory] Created PharmacyKiosk [" << id
                  << "] at " << location << "\n";
    }

    void displayInfo() const override {
        Kiosk::displayInfo();
        std::cout << "  💊 Specialization: Medicines, First-Aid & Health Products\n";
        std::cout << "  📍 Typical Locations: Hospitals, Clinics\n";
        std::cout << "  ⚠️  Note: Some items require prescription verification\n";
    }
};

#endif // PHARMACY_KIOSK_H
