/*
 * ============================================================================
 *  AURA RETAIL OS — Central Registry Implementation
 * ============================================================================
 */

#include "CentralRegistry.h"
#include "../persistence/PersistenceManager.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

// Thread-safe Meyer's Singleton implementation
CentralRegistry& CentralRegistry::getInstance() {
    static CentralRegistry instance; // Guaranteed to be destroyed. Instantiated on first use.
    return instance;
}

void CentralRegistry::registerKiosk(const std::string& kioskId) {
    if (std::find(registeredKioskIds_.begin(), registeredKioskIds_.end(), kioskId) == registeredKioskIds_.end()) {
        registeredKioskIds_.push_back(kioskId);
        std::cout << "🔒 [CentralRegistry] Kiosk \"" << kioskId << "\" registered successfully.\n";
    }
}

void CentralRegistry::recordTransaction(const Transaction& tx) {
    allTransactions_.push_back(tx);
    std::cout << "[Registry] Transaction " << tx.transactionId
              << " recorded globally.\n";
    PersistenceManager::saveTransactionsToFile(allTransactions_, "transactions.json");
}

void CentralRegistry::displayGlobalReport() const {
    std::cout << "\n=======================================================\n";
    std::cout << "🌐 GLOBAL CENTRAL REGISTRY REPORT\n";
    std::cout << "=======================================================\n";

    std::cout << "\nRegistered Kiosks (" << registeredKioskIds_.size() << "):\n";
    for (const auto& id : registeredKioskIds_) {
        std::cout << "  - " << id << "\n";
    }

    std::cout << "\nGlobal Transactions (" << allTransactions_.size() << "):\n";
    double totalRevenue = 0.0;
    for (const auto& tx : allTransactions_) {
        tx.display();
        if (tx.status == TransactionStatus::SUCCESS) {
            totalRevenue += tx.amount;
        }
    }

    std::cout << "\nTotal Network Revenue: ₹" << totalRevenue << "\n";
    std::cout << "=======================================================\n\n";
}
