/*
 * ============================================================================
 *  AURA RETAIL OS — Emergency Kiosk (Factory Pattern - Concrete Product)
 * ============================================================================
 *
 *  DESIGN PATTERN: Factory Pattern (Concrete Product)
 *
 *  EmergencyKiosk is deployed in disaster zones and emergency shelters.
 *  It specializes in survival essentials: water, food rations, flashlights,
 *  first-aid kits, and communication devices. It can operate in low-power
 *  and offline modes.
 *
 * ============================================================================
 */

#ifndef EMERGENCY_KIOSK_H
#define EMERGENCY_KIOSK_H

#include "Kiosk.h"

class EmergencyKiosk : public Kiosk {
public:
    EmergencyKiosk(const std::string& id, const std::string& location)
        : Kiosk(id, "Emergency Kiosk 🚨", location) {
        std::cout << "🏭 [Factory] Created EmergencyKiosk [" << id
                  << "] at " << location << "\n";
    }

    void displayInfo() const override {
        Kiosk::displayInfo();
        std::cout << "  🚨 Specialization: Survival Essentials & Emergency Supplies\n";
        std::cout << "  📍 Typical Locations: Disaster zones, Emergency shelters\n";
        std::cout << "  ⚡ Feature: Low-power & offline operation supported\n";
    }
};

#endif // EMERGENCY_KIOSK_H
