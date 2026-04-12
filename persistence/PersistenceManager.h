#ifndef PERSISTENCE_MANAGER_H
#define PERSISTENCE_MANAGER_H

#include "../inventory/InventoryInterface.h"
#include "../payment/Transaction.h"
#include "../registry/CentralRegistry.h"
#include <vector>
#include <string>

class PersistenceManager {
public:
    static void saveInventoryToFile(InventoryInterface* inventory, const std::string& filename);
    static void loadInventoryFromFile(InventoryInterface* inventory, const std::string& filename);
    static void saveTransactionsToFile(const std::vector<Transaction>& txs, const std::string& filename);
};

#endif // PERSISTENCE_MANAGER_H
