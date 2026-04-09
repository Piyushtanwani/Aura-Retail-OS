/*
 * ============================================================================
 *  AURA RETAIL OS — UPI Adapter (Adapter + Strategy Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Adapter Pattern + Strategy Pattern
 *
 *  This class serves a dual role:
 *    1. ADAPTER: It wraps a legacy UPI payment system (LegacyUPIGateway)
 *       and adapts its incompatible interface to the PaymentProcessor
 *       interface that our system expects.
 *    2. STRATEGY: It is one of the interchangeable payment strategies
 *       that can be plugged into PaymentContext at runtime.
 *
 * ============================================================================
 */

#ifndef UPI_ADAPTER_H
#define UPI_ADAPTER_H

#include "PaymentProcessor.h"
#include <iostream>
#include <string>

// ─── Legacy UPI Gateway (external/third-party system with incompatible API) ─
class LegacyUPIGateway {
public:
    bool initiateUPIPayment(const std::string& upiId, double amount) {
        std::cout << "    💳 [LegacyUPI] Initiating UPI payment of ₹" << amount
                  << " via VPA: " << upiId << "\n";
        std::cout << "    💳 [LegacyUPI] Sending OTP... Verified ✅\n";
        return true;  // simulate success
    }

    bool reverseUPIPayment(double amount) {
        std::cout << "    💳 [LegacyUPI] Reversing UPI payment of ₹" << amount << "\n";
        return true;
    }
};

// ─── UPI Adapter (adapts LegacyUPIGateway → PaymentProcessor) ──────────────
class UPIAdapter : public PaymentProcessor {
private:
    LegacyUPIGateway legacyGateway_;   // the adaptee
    std::string vpa_;                   // virtual payment address

public:
    explicit UPIAdapter(const std::string& vpa = "user@aura-upi")
        : vpa_(vpa) {}

    bool processPayment(double amount) override {
        std::cout << "  🔌 [UPIAdapter] Adapting payment request to legacy UPI gateway...\n";
        return legacyGateway_.initiateUPIPayment(vpa_, amount);
    }

    bool processRefund(double amount) override {
        std::cout << "  🔌 [UPIAdapter] Adapting refund request to legacy UPI gateway...\n";
        return legacyGateway_.reverseUPIPayment(amount);
    }

    std::string getMethodName() const override {
        return "UPI (" + vpa_ + ")";
    }
};

#endif // UPI_ADAPTER_H
