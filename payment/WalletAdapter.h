/*
 * ============================================================================
 *  AURA RETAIL OS — Wallet Adapter (Adapter + Strategy Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Adapter Pattern + Strategy Pattern
 *
 *  Adapts a legacy digital wallet system (LegacyWalletService) to the
 *  PaymentProcessor interface. Also serves as an interchangeable strategy
 *  in the Strategy pattern.
 *
 * ============================================================================
 */

#ifndef WALLET_ADAPTER_H
#define WALLET_ADAPTER_H

#include "PaymentProcessor.h"
#include <iostream>
#include <string>

// ─── Legacy Wallet Service (external system with incompatible API) ───────────
class LegacyWalletService {
public:
    bool debitWallet(const std::string& walletId, double amount) {
        std::cout << "    💳 [LegacyWallet] Debiting ₹" << amount
                  << " from wallet: " << walletId << "\n";
        std::cout << "    💳 [LegacyWallet] Balance verified. Transaction complete ✅\n";
        return true;
    }

    bool creditWallet(const std::string& walletId, double amount) {
        std::cout << "    💳 [LegacyWallet] Crediting ₹" << amount
                  << " to wallet: " << walletId << "\n";
        return true;
    }
};

// ─── Wallet Adapter (adapts LegacyWalletService → PaymentProcessor) ─────────
class WalletAdapter : public PaymentProcessor {
private:
    LegacyWalletService legacyService_;   // the adaptee
    std::string walletId_;

public:
    explicit WalletAdapter(const std::string& walletId = "AURA-WALLET-001")
        : walletId_(walletId) {}

    bool processPayment(double amount) override {
        std::cout << "  🔌 [WalletAdapter] Adapting payment request to legacy wallet service...\n";
        return legacyService_.debitWallet(walletId_, amount);
    }

    bool processRefund(double amount) override {
        std::cout << "  🔌 [WalletAdapter] Adapting refund request to legacy wallet service...\n";
        return legacyService_.creditWallet(walletId_, amount);
    }

    std::string getMethodName() const override {
        return "Digital Wallet (" + walletId_ + ")";
    }
};

#endif // WALLET_ADAPTER_H
