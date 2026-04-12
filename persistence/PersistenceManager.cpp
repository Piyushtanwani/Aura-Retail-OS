#include "PersistenceManager.h"
#include "../inventory/Product.h"
#include "../inventory/Bundle.h"
#include <fstream>
#include <iostream>
#include <sstream>

// ─── Save Inventory ────────────────────────────────────────────────────────

void PersistenceManager::saveInventoryToFile(InventoryInterface* inventory, const std::string& filename) {
    if (!inventory) return;

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "  ❌ [PersistenceManager] Failed to open " << filename << " for writing.\n";
        return;
    }

    file << "{\n  \"inventory\": [\n";
    auto itemIds = inventory->getAllItemIds();
    
    for (size_t i = 0; i < itemIds.size(); ++i) {
        auto item = inventory->getItem(itemIds[i]);
        if (item) {
            int stock = inventory->getStock(itemIds[i]);
            file << item->toJson(stock);
            if (i < itemIds.size() - 1) {
                file << ",";
            }
            file << "\n";
        }
    }
    file << "  ]\n}\n";
    std::cout << "  💾 [PersistenceManager] Successfully saved inventory to " << filename << "\n";
}

// ─── Save Transactions ─────────────────────────────────────────────────────

void PersistenceManager::saveTransactionsToFile(const std::vector<Transaction>& txs, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "  ❌ [PersistenceManager] Failed to open " << filename << " for writing.\n";
        return;
    }

    file << "{\n  \"transactions\": [\n";
    for (size_t i = 0; i < txs.size(); ++i) {
        const auto& t = txs[i];
        file << "    {\n";
        file << "      \"id\": \"" << t.transactionId << "\",\n";
        file << "      \"kiosk\": \"" << t.kioskId << "\",\n";
        file << "      \"item\": \"" << t.itemName << "\",\n";
        file << "      \"quantity\": " << t.quantity << ",\n";
        file << "      \"amount\": " << t.amount << ",\n";
        file << "      \"payment\": \"" << t.paymentMethod << "\",\n";
        file << "      \"status\": \"" << ((t.status == TransactionStatus::SUCCESS) ? "SUCCESS" : "FAILED") << "\"\n";
        file << "    }";
        if (i < txs.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ]\n}\n";
}

// ─── Load Inventory (Naive Line Parser) ────────────────────────────────────

// Helper to extract string values like "name": "Crocin" -> Crocin
std::string extractStringValue(const std::string& line, const std::string& key) {
    size_t keyPos = line.find("\"" + key + "\":");
    if (keyPos == std::string::npos) return "";
    size_t startQuote = line.find("\"", keyPos + key.length() + 2);
    if (startQuote == std::string::npos) return "";
    size_t endQuote = line.find("\"", startQuote + 1);
    if (endQuote == std::string::npos) return "";
    return line.substr(startQuote + 1, endQuote - startQuote - 1);
}

// Helper to extract num values like "price": 50 -> 50
double extractDoubleValue(const std::string& line, const std::string& key) {
    size_t keyPos = line.find("\"" + key + "\":");
    if (keyPos == std::string::npos) return 0.0;
    size_t startVal = keyPos + key.length() + 2;
    while(startVal < line.length() && (line[startVal] == ' ' || line[startVal] == '\"')) startVal++;
    
    size_t endVal = startVal;
    while(endVal < line.length() && (isdigit(line[endVal]) || line[endVal] == '.')) endVal++;
    
    std::string valStr = line.substr(startVal, endVal - startVal);
    if (valStr.empty()) return 0.0;
    try { return std::stod(valStr); } catch (...) { return 0.0; }
}

void PersistenceManager::loadInventoryFromFile(InventoryInterface* inventory, const std::string& filename) {
    if (!inventory) return;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "  ℹ️  [PersistenceManager] No previous inventory file found (" << filename << "). Proceeding fresh.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("{\"id\"") != std::string::npos) {
            std::string id = extractStringValue(line, "id");
            std::string name = extractStringValue(line, "name");
            double price = extractDoubleValue(line, "price");
            int stock = (int)extractDoubleValue(line, "stock");

            if (line.find("\"items\":") != std::string::npos) {
                // It's a bundle. We won't fully parse nested children IDs in this naive parser for time constraints, 
                // but we will create the abstract bundle wrapper to represent the item.
                // A flawless parser would regex array elements.
                auto bundle = std::make_shared<Bundle>(id, name);
                inventory->addItem(bundle, stock);
                std::cout << "  📥 Loaded historical Bundle: " << name << "\n";
            } else {
                auto product = std::make_shared<Product>(id, name, price);
                inventory->addItem(product, stock);
                std::cout << "  📥 Loaded historical Product: " << name << "\n";
            }
        }
    }
}
