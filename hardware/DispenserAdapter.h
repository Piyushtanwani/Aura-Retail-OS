/*
 * ============================================================================
 *  AURA RETAIL OS — Dispenser Adapter (Adapter Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Adapter Pattern
 *
 *  This adapter wraps an external/third-party dispenser hardware system
 *  (ExternalDispenserAPI) and adapts its incompatible interface to the
 *  standard Dispenser interface used by our kiosk system.
 *
 *  This demonstrates how the Adapter pattern enables integration with
 *  external hardware vendors without modifying existing code.
 *
 * ============================================================================
 */

#ifndef DISPENSER_ADAPTER_H
#define DISPENSER_ADAPTER_H

#include "Dispenser.h"
#include <iostream>
#include <string>

// ─── External Dispenser API (third-party hardware with incompatible interface)
class ExternalDispenserAPI {
public:
    int initHardware(int slotNumber) {
        std::cout << "    🔧 [ExternalAPI] Initializing hardware on slot " << slotNumber << "...\n";
        return 0;  // 0 = success
    }

    int pushItem(int slotNumber, const char* itemCode) {
        std::cout << "    🔧 [ExternalAPI] Pushing item \"" << itemCode
                  << "\" from slot " << slotNumber << "\n";
        return 0;
    }

    int confirmDelivery(int slotNumber) {
        std::cout << "    🔧 [ExternalAPI] Delivery confirmed on slot " << slotNumber << " ✅\n";
        return 0;
    }
};

// ─── Dispenser Adapter (adapts ExternalDispenserAPI → Dispenser) ────────────
class DispenserAdapter : public Dispenser {
private:
    ExternalDispenserAPI externalApi_;   // the adaptee
    int slotNumber_;

public:
    explicit DispenserAdapter(int slot = 1) : slotNumber_(slot) {
        externalApi_.initHardware(slotNumber_);
    }

    bool dispense(const std::string& itemId, const std::string& itemName) override {
        std::cout << "  🔌 [DispenserAdapter] Adapting dispense request to external API...\n";
        int pushResult = externalApi_.pushItem(slotNumber_, itemId.c_str());
        if (pushResult != 0) return false;

        int confirmResult = externalApi_.confirmDelivery(slotNumber_);
        return confirmResult == 0;
    }

    std::string getType() const override {
        return "External Dispenser (Slot " + std::to_string(slotNumber_) + ")";
    }
};

#endif // DISPENSER_ADAPTER_H
