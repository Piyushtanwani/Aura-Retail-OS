/*
 * ============================================================================
 *  AURA RETAIL OS — Card Adapter (Adapter + Strategy Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Adapter Pattern + Strategy Pattern
 *
 *  Adapts a legacy card payment system (LegacyCardTerminal) to the
 *  PaymentProcessor interface. Also serves as an interchangeable strategy
 *  in the Strategy pattern.
 *
 * ============================================================================
 */

#ifndef CARD_ADAPTER_H
#define CARD_ADAPTER_H

#include "PaymentProcessor.h"
#include <iostream>
#include <string>

// ─── Legacy Card Terminal (external system with incompatible API) ────────────
class LegacyCardTerminal {
public:
    bool swipeCard(const std::string& cardNumber, double amount) {
        std::cout << "    💳 [LegacyCard] Swiping card ending in ****"
                  << cardNumber.substr(cardNumber.size() - 4) << "\n";
        std::cout << "    💳 [LegacyCard] Authorizing ₹" << amount << "... Approved ✅\n";
        return true;
    }

    bool chargeBack(double amount) {
        std::cout << "    💳 [LegacyCard] Processing chargeback of ₹" << amount << "\n";
        return true;
    }
};

// ─── Card Adapter (adapts LegacyCardTerminal → PaymentProcessor) ────────────
class CardAdapter : public PaymentProcessor {
private:
    LegacyCardTerminal legacyTerminal_;   // the adaptee
    std::string cardNumber_;
    std::string expiry_;
    std::string cvv_;

public:
    explicit CardAdapter(const std::string& cardNumber, const std::string& expiry, const std::string& cvv)
        : cardNumber_(cardNumber), expiry_(expiry), cvv_(cvv) {}

    bool processPayment(double amount) override {
        std::cout << "  🔌 [CardAdapter] Adapting payment request to legacy card terminal...\n";
        // Legacy system only takes card number and amount, but we store CVV/Expiry for record
        return legacyTerminal_.swipeCard(cardNumber_, amount);
    }

    bool processRefund(double amount) override {
        std::cout << "  🔌 [CardAdapter] Adapting refund request to legacy card terminal...\n";
        return legacyTerminal_.chargeBack(amount);
    }

    std::string getMethodName() const override {
        return "Credit Card (****" + cardNumber_.substr(cardNumber_.size() - 4) + ")";
    }
};

#endif // CARD_ADAPTER_H
