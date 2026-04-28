/*
 * ============================================================================
 *  AURA RETAIL OS — Central Registry (Singleton Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Singleton Pattern
 *
 *  The Central Registry is a global system that tracks all kiosks and
 *  transactions across the smart city network.
 *
 *  Singleton properties:
 *    - Only ONE instance can exist globally.
 *    - Global access point via getInstance().
 *    - Copy constructor and assignment operator are deleted to prevent copying.
 *
 * ============================================================================
 */

#ifndef CENTRAL_REGISTRY_H
#define CENTRAL_REGISTRY_H

#include "../payment/Transaction.h"
#include <vector>
#include <string>
#include <mutex>
#include <iostream>

class CentralRegistry {
private:
    std::vector<Transaction> allTransactions_;
    std::vector<std::string> registeredKioskIds_;

    // Private constructor (prevents instantiation from outside)
    CentralRegistry() {
        std::cout << "🔒 [CentralRegistry] Global instance initialized.\n";
    }

public:
    // Delete copy and move semantics
    CentralRegistry(const CentralRegistry&) = delete;
    CentralRegistry& operator=(const CentralRegistry&) = delete;
    CentralRegistry(CentralRegistry&&) = delete;
    CentralRegistry& operator=(CentralRegistry&&) = delete;

    // Global access point
    static CentralRegistry& getInstance();

    // Registry functions
    void registerKiosk(const std::string& kioskId);
    void recordTransaction(const Transaction& tx);
    void updateTransactionStatus(const std::string& txId, TransactionStatus status);

    void displayGlobalReport() const;

    const std::vector<Transaction>& getTransactions() const {
        return allTransactions_;
    }
};

#endif // CENTRAL_REGISTRY_H
