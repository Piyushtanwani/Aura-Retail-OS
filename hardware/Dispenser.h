/*
 * ============================================================================
 *  AURA RETAIL OS — Dispenser Interface (Hardware Abstraction)
 * ============================================================================
 *
 *  Abstract interface for all dispenser hardware. This enables the system
 *  to work with different dispenser types (standard, refrigerated, external)
 *  through a uniform interface, supporting dynamic switching at runtime.
 *
 * ============================================================================
 */

#ifndef DISPENSER_H
#define DISPENSER_H

#include <string>

class Dispenser {
public:
    virtual ~Dispenser() = default;

    // Dispense an item by ID. Returns true on success.
    virtual bool dispense(const std::string& itemId, const std::string& itemName) = 0;

    // Get the type/name of this dispenser
    virtual std::string getType() const = 0;
};

#endif // DISPENSER_H
