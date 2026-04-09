/*
 * ============================================================================
 *  AURA RETAIL OS — Main Simulation (Entry Point)
 * ============================================================================
 *
 *  This file orchestrates the entire system and demonstrates all 7 design
 *  patterns working together cohesively.
 *
 *  Dynamic Inputs:
 *    - Kiosk IDs and locations
 *    - Product names, prices, and stock quantities
 *    - Payment method selection (UPI / Card / Wallet)
 *    - UPI VPA or Card number
 *    - Refrigeration temperature for the pharmacy kiosk
 *
 * ============================================================================
 */

#include "persistence/PersistenceManager.h"
#include "registry/CentralRegistry.h"
#include <iostream>
#include <memory>
#include <string>
#include <limits>

// Core & Factory
#include "core/KioskFactory.h"
#include "core/NetworkModule.h"
#include "core/RefrigerationModule.h"

// Inventory & Composite
#include "inventory/Bundle.h"
#include "inventory/Product.h"
#include "inventory/SecureInventory.h"

// Hardware
#include "hardware/RefrigeratedDispenser.h"

// Payment
#include "payment/CardAdapter.h"
#include "payment/UPIAdapter.h"
#include "payment/WalletAdapter.h"

using namespace std;

// ─── Helpers ────────────────────────────────────────────────────────────────

void printSectionHeader(const string &title) {
    cout << "\n\n";
    cout << "████████████████████████████████████████████████████████████████\n";
    cout << "█  " << title << "\n";
    cout << "████████████████████████████████████████████████████████████████\n\n";
}

void clearInput() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string promptString(const string &prompt, const string &defaultVal = "") {
    string val;
    cout << prompt;
    if (!defaultVal.empty()) cout << " [default: " << defaultVal << "]";
    cout << ": ";
    getline(cin, val);
    if (val.empty()) val = defaultVal;
    return val;
}

double promptDouble(const string &prompt, double defaultVal) {
    string raw;
    cout << prompt << " [default: " << defaultVal << "]: ";
    getline(cin, raw);
    if (raw.empty()) return defaultVal;
    try { return stod(raw); } catch (...) { return defaultVal; }
}

int promptInt(const string &prompt, int defaultVal) {
    string raw;
    cout << prompt << " [default: " << defaultVal << "]: ";
    getline(cin, raw);
    if (raw.empty()) return defaultVal;
    try { return stoi(raw); } catch (...) { return defaultVal; }
}

// Returns a unique_ptr<PaymentProcessor> based on user choice
unique_ptr<PaymentProcessor> choosePaymentStrategy(const string &label) {
    cout << "\n  Select payment method for " << label << ":\n";
    cout << "    [1] UPI\n";
    cout << "    [2] Card\n";
    cout << "    [3] Wallet\n";
    int choice = promptInt("  Enter choice", 1);

    if (choice == 2) {
        string cardNum = promptString("  Enter card number", "5555444433332222");
        return make_unique<CardAdapter>(cardNum);
    } else if (choice == 3) {
        string wallet = promptString("  Enter wallet name/ID", "AuraWallet");
        return make_unique<WalletAdapter>(wallet);
    } else {
        string vpa = promptString("  Enter UPI VPA", "user@aura-upi");
        return make_unique<UPIAdapter>(vpa);
    }
}

// ─── Main ────────────────────────────────────────────────────────────────────

int main() {

    // ── Welcome ──────────────────────────────────────────────────────────────
    cout << "\n";
    cout << "╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║        🌟  AURA RETAIL OS — Interactive Simulation  🌟       ║\n";
    cout << "║   Press ENTER at any prompt to accept the default value.     ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n";

    auto foodProxy = make_shared<SecureInventory>();

    // Load existing items BEFORE we inject the defaults.
    PersistenceManager::loadInventoryFromFile(foodProxy.get(), "inventory.json");

    // =========================================================================
    printSectionHeader("1. SYSTEM INITIALIZATION & FACTORY PATTERN");
    // =========================================================================

    cout << "── Configure Food Kiosk ─────────────────────────────────────\n";
    string foodKioskId  = promptString("  Food Kiosk ID",       "FD-S1");
    string foodLocation = promptString("  Food Kiosk Location", "Central Metro Station");

    cout << "\n── Configure Pharmacy Kiosk ─────────────────────────────────\n";
    string pharmaKioskId  = promptString("  Pharmacy Kiosk ID",       "PH-H1");
    string pharmaLocation = promptString("  Pharmacy Kiosk Location", "City Hospital");

    CentralRegistry &registry = CentralRegistry::getInstance();

    // Use Factory Pattern to create kiosks
    auto foodKioskBase    = KioskFactory::createKiosk("food",     foodKioskId,  foodLocation);
    auto pharmacyKioskBase = KioskFactory::createKiosk("pharmacy", pharmaKioskId, pharmaLocation);

    registry.registerKiosk(foodKioskBase->getId());
    registry.registerKiosk(pharmacyKioskBase->getId());

    foodKioskBase->displayInfo();
    pharmacyKioskBase->displayInfo();

    // =========================================================================
    printSectionHeader("2. INVENTORY SETUP & COMPOSITE PATTERN");
    // =========================================================================

    cout << "── Configure Products ───────────────────────────────────────\n";

    // -- Pharmacy products --
    cout << "\n  [Pharmacy] Tablet product:\n";
    string crocinName  = promptString("    Name",  "Crocin 500mg");
    double crocinPrice = promptDouble("    Price", 50.0);

    cout << "\n  [Pharmacy] Cold-chain product (requires refrigeration):\n";
    string insulinName  = promptString("    Name",  "Insulin Pen");
    double insulinPrice = promptDouble("    Price", 850.0);

    cout << "\n  [Pharmacy] Bandage product:\n";
    string bandageName  = promptString("    Name",  "Bandage Roll");
    double bandagePrice = promptDouble("    Price", 30.0);

    // -- Food products --
    cout << "\n  [Food] Beverage product:\n";
    string waterName  = promptString("    Name",  "Mineral Water");
    double waterPrice = promptDouble("    Price", 20.0);
    int    waterQty   = promptInt   ("    Stock quantity", 50);

    cout << "\n  [Food] Snack product:\n";
    string sandwichName  = promptString("    Name",  "Veg Sandwich");
    double sandwichPrice = promptDouble("    Price", 120.0);
    int    sandwichQty   = promptInt   ("    Stock (set low to test out-of-stock)", 2);

    // -- Bundle discount --
    cout << "\n  [Bundle] First Aid Kit:\n";
    string bundleId   = promptString("    Bundle ID",           "B-001");
    string bundleName = promptString("    Bundle Name",         "Basic First Aid Kit");
    double bundleDisc = promptDouble("    Discount percentage", 10.0);

    // Creating leaf products
    auto crocin   = make_shared<Product>("P-101", crocinName,   crocinPrice);
    auto insulin  = make_shared<Product>("P-102", insulinName,  insulinPrice, true); // needs refrigeration
    auto bandage  = make_shared<Product>("P-103", bandageName,  bandagePrice);
    auto water    = make_shared<Product>("P-201", waterName,    waterPrice);
    auto sandwich = make_shared<Product>("P-202", sandwichName, sandwichPrice, true);

    // Creating composite bundle
    auto firstAidKit = make_shared<Bundle>(bundleId, bundleName, bundleDisc);
    firstAidKit->add(crocin);
    firstAidKit->add(bandage);

    // Display Composite structure
    cout << "\nComposite Bundle Structure:\n";
    firstAidKit->display();

    // Adding to Kiosks (via SecureInventory Proxy)
    foodKioskBase->addProduct(water,    waterQty);
    foodKioskBase->addProduct(sandwich, sandwichQty);

    int insulinQty   = promptInt("\n  Pharmacy — Insulin stock quantity",        5);
    int firstAidQty  = promptInt("  Pharmacy — First Aid Kit stock quantity",   10);
    pharmacyKioskBase->addProduct(insulin,     insulinQty);
    pharmacyKioskBase->addProduct(firstAidKit, firstAidQty);

    cout << "\nPharmacy Kiosk Catalogue (Proxy logs access):\n";
    pharmacyKioskBase->getInventory()->displayCatalogue();

    // =========================================================================
    printSectionHeader("3. HARDWARE & DECORATOR PATTERN");
    // =========================================================================

    cout << "── Configure Refrigeration Unit ─────────────────────────────\n";
    double refTemp = promptDouble("  Refrigeration temperature (°C)", 4.0);

    pharmacyKioskBase->setDispenser(make_unique<RefrigeratedDispenser>(refTemp));

    // Wrap the kiosk in decorators
    unique_ptr<Kiosk> activePharmacyKiosk =
        make_unique<RefrigerationModule>(std::move(pharmacyKioskBase), refTemp);
    activePharmacyKiosk =
        make_unique<NetworkModule>(std::move(activePharmacyKiosk));

    cout << "\nUpdated Pharmacy Kiosk Info (After Decoration):\n";
    activePharmacyKiosk->displayInfo();
    if (activePharmacyKiosk->describeModules() != "None") {
        cout << "  🔧 Active Modules:\n"
             << activePharmacyKiosk->describeModules() << "\n";
    } else {
        cout << "  🔧 Active Modules: None\n";
    }

    // =========================================================================
    printSectionHeader("4. PAYMENT STRATEGY & ADAPTER PATTERN");
    // =========================================================================

    // Payment strategy for pharmacy — first purchase
    cout << "── Payment for Pharmacy Purchase 1 (Insulin) ───────────────\n";
    activePharmacyKiosk->setPaymentStrategy(choosePaymentStrategy("Pharmacy Purchase 1"));

    Transaction tx1 = activePharmacyKiosk->purchaseItem("P-102"); // Buy Insulin
    if (tx1.status == TransactionStatus::SUCCESS) {
        registry.recordTransaction(tx1);
    }

    // Switch payment strategy at runtime — second purchase
    cout << "\n── Payment for Pharmacy Purchase 2 (First Aid Kit) ─────────\n";
    activePharmacyKiosk->setPaymentStrategy(choosePaymentStrategy("Pharmacy Purchase 2"));

    Transaction tx2 = activePharmacyKiosk->purchaseItem("B-001"); // Buy First Aid Kit
    if (tx2.status == TransactionStatus::SUCCESS) {
        registry.recordTransaction(tx2);
    }

    // =========================================================================
    printSectionHeader("5. SYSTEM CONSTRAINTS (PROXY VALIDATION)");
    // =========================================================================

    cout << "\n--- Scenario A: Hardware Dependency Failure ---\n";
    cout << "Attempting to buy Insulin (Requires Refrigeration) from Food Kiosk...\n";

    // Add Insulin to Food Kiosk first (proxy allows adding, dispensing fails)
    foodKioskBase->addProduct(insulin, 1);
    Transaction tx_temp = foodKioskBase->purchaseItem("P-102");
    if (tx_temp.status == TransactionStatus::SUCCESS) {
        registry.recordTransaction(tx_temp);
    }

    cout << "\n--- Scenario B: Out-of-Stock Validation ---\n";
    cout << "── Payment for Food Kiosk purchases ────────────────────────\n";
    foodKioskBase->setPaymentStrategy(choosePaymentStrategy("Food Kiosk"));

    cout << "\n  Attempting " << (sandwichQty + 1) << " purchases of \""
         << sandwichName << "\" (stock is " << sandwichQty << ")...\n";
    for (int i = 0; i < sandwichQty + 1; i++) {
        Transaction tx = foodKioskBase->purchaseItem("P-202");
        if (tx.status == TransactionStatus::SUCCESS) {
            registry.recordTransaction(tx);
        }
    }

    cout << "\n--- Scenario C: Hardware Failure After Payment ---\n";
    string brokenName  = promptString("\n  Jammed product name",  "Jammed Snack");
    double brokenPrice = promptDouble("  Jammed product price", 10.0);

    auto brokenItem = make_shared<Product>("P-ERROR", brokenName, brokenPrice);
    foodKioskBase->addProduct(brokenItem, 5);
    Transaction tx_err = foodKioskBase->purchaseItem("P-ERROR");
    if (tx_err.status == TransactionStatus::SUCCESS) {
        registry.recordTransaction(tx_err);
    } else {
        cout << "  🚫 Transaction cancelled\n";
    }

    // =========================================================================
    printSectionHeader("6. SINGLETON REGISTRY REPORT");
    // =========================================================================
    registry.displayGlobalReport();

    // =========================================================================
    printSectionHeader("7. SHUTDOWN & PERSISTENCE");
    // =========================================================================
    PersistenceManager::saveInventoryToFile(foodProxy.get(), "inventory.json");

    cout << "\n📌 System successfully demonstrates:\n";
    cout << "   ✔ Modular architecture\n";
    cout << "   ✔ Design patterns\n";
    cout << "   ✔ Constraint handling\n";
    cout << "   ✔ Persistent storage\n\n";

    return 0;
}
