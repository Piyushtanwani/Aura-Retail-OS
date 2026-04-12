/*
 * ============================================================================
 *  AURA RETAIL OS — Network Module (Decorator Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Decorator Pattern (Concrete Decorator)
 *
 *  Provides cloud connectivity. It intercepts transactions and simulates
 *  syncing them with a central cloud server asynchronously.
 *
 * ============================================================================
 */

#ifndef NETWORK_MODULE_H
#define NETWORK_MODULE_H

#include "KioskDecorator.h"
#include <iostream>

class NetworkModule : public KioskDecorator {
public:
    explicit NetworkModule(std::unique_ptr<Kiosk> wrappee)
        : KioskDecorator(std::move(wrappee)) {
        std::cout << "  🎨 [Decorator] Attached Network/Cloud Sync Module to Kiosk " << kioskId_ << "\n";
    }

    Transaction purchaseItem(const std::string& itemId, int quantity = 1) override {
        // Delegate purchase
        Transaction tx = KioskDecorator::purchaseItem(itemId, quantity);

        // Add post-purchase behavior
        if (tx.status == TransactionStatus::SUCCESS) {
            std::cout << "  🌐 [NetworkModule] Syncing transaction " << tx.transactionId
                      << " to Aura Cloud Server... ✅\n";
        }
        return tx;
    }

    std::string describeModules() const override {
        std::string base = wrappee_->describeModules();
        std::string me = "   - Cloud Connectivity (5G)";
        return (base == "None" || base.empty()) ? me : base + "\n" + me;
    }
};

#endif // NETWORK_MODULE_H
