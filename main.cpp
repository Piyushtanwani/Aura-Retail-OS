/*
 * ============================================================================
 *  AURA RETAIL OS — Interactive Kiosk Simulation (Entry Point)
 * ============================================================================
 *
 *  Role-based entry:
 *    [1] Customer — browse products, choose item, pay, see result
 *    [2] Admin    — PIN-protected panel to restock kiosk inventory
 *
 *  Customer Flow:
 *    Step 1 — Choose kiosk    : [1] Food  [2] Pharmacy
 *    Step 2 — Browse products : Numbered catalogue with price & stock
 *    Step 3 — Pick a product  : Enter the number shown
 *    Step 4 — Choose payment  : [1] UPI  [2] Card  [3] Wallet
 *    Step 5 — See result      : PAYMENT SUCCESSFUL / PAYMENT FAILED banner
 *    Step 6 — Continue?       : [1] Buy another  [2] Exit
 *
 *  Admin Flow:
 *    PIN check → Choose kiosk → View full stock →
 *    Pick item → Enter restock quantity → Confirm
 *
 * ============================================================================
 */

#include "persistence/PersistenceManager.h"
#include "registry/CentralRegistry.h"
#include <filesystem> // for absolute path resolution (C++17)
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h> // for SetConsoleOutputCP
#endif

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

// ─── Constants ──────────────────────────────────────────────────────────────
static const string ADMIN_PIN = "1234"; // Change this to any PIN you like
static const string FOOD_INV_FILE =
    "food_inventory.json"; // food kiosk inventory
static const string PHARMA_INV_FILE =
    "pharmacy_inventory.json"; // pharmacy kiosk inventory
static const string EMERGENCY_INV_FILE =
    "emergency_inventory.json"; // emergency kiosk inventory

// Forward declarations (saveAll uses Kiosk*)
class Kiosk;

// ─── UI Helpers ─────────────────────────────────────────────────────────────

void clearScreen() {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

void pressEnterToContinue() {
  cout << "\n  [ Press ENTER to continue... ]";
  cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void printBanner(const string &title, char border = '=') {
  string line(62, border);
  cout << "\n" << line << "\n";
  int pad = max(0, (62 - (int)title.size()) / 2);
  cout << string(pad, ' ') << title << "\n";
  cout << line << "\n";
}

// Safe integer input within [lo, hi]. Re-prompts on invalid input.
int readChoice(const string &prompt, int lo, int hi) {
  string raw;
  while (true) {
    cout << prompt;
    getline(cin, raw);
    try {
      int val = stoi(raw);
      if (val >= lo && val <= hi)
        return val;
    } catch (...) {
    }
    cout << "  ⚠  Please enter a number between " << lo << " and " << hi
         << ".\n";
  }
}

// Safe positive integer input. Re-prompts on invalid or non-positive input.
int readPositiveInt(const string &prompt) {
  string raw;
  while (true) {
    cout << prompt;
    getline(cin, raw);
    try {
      int val = stoi(raw);
      if (val > 0)
        return val;
    } catch (...) {
    }
    cout << "  ⚠  Please enter a positive whole number (e.g. 5, 10, 50).\n";
  }
}

// Safe non-empty string input.
string readNonEmpty(const string &prompt) {
  string val;
  while (true) {
    cout << prompt;
    getline(cin, val);
    if (!val.empty())
      return val;
    cout << "  ⚠  This field cannot be empty. Please try again.\n";
  }
}

// ─── Show numbered product catalogue (customer view — out-of-stock included) ─
// Returns parallel vector of item IDs.
vector<string> showProductMenu(Kiosk *kiosk, bool showOutOfStock = true) {
  vector<string> ids = kiosk->getInventory()->getAllItemIds();
  vector<string> shown;

  ostringstream table;
  table << "\n";
  table
      << "  ┌──────┬──────────────────────────────┬──────────┬─────────────┐\n";
  table
      << "  │  No. │  Product Name                │  Price   │  Stock      │\n";
  table
      << "  ├──────┼──────────────────────────────┼──────────┼─────────────┤\n";

  int num = 1;
  bool hasRefrigerated = false;
  for (const string &id : ids) {
    // These calls will trigger SecureInventory cout logging immediately
    auto item = kiosk->getInventory()->getItem(id);
    int stock = kiosk->getInventory()->getStock(id);
    if (!item)
      continue;

    string name = item->getName();
    string paddedName = name;
    int nameVisLen = name.length();
    if (item->requiresRefrigeration()) {
      paddedName += " \xE2\x9D\x84"; // ❄
      nameVisLen += 2;               // " " + emoji visually takes 2 columns
      hasRefrigerated = true;
    }
    int namePad = 28 - nameVisLen;
    if (namePad > 0)
      paddedName.append(namePad, ' ');

    ostringstream priceStream;
    priceStream << fixed << setprecision(0) << item->getPrice();
    string priceStr = "Rs." + priceStream.str();
    int pricePad = 8 - priceStr.length();
    if (pricePad > 0)
      priceStr.append(pricePad, ' ');

    string stockStr =
        (stock > 0) ? to_string(stock) + " unit(s)" : "OUT OF STOCK";
    int stockPad = 11 - stockStr.length();
    if (stockPad > 0)
      stockStr.append(stockPad, ' ');

    table << "  │ [" << left << setw(2) << num << "] │  " << paddedName << "│  "
          << priceStr << "│  " << stockStr << "│\n";

    shown.push_back(id);
    num++;
  }

  table
      << "  └──────┴──────────────────────────────┴──────────┴─────────────┘\n";

  // Now that all inline proxy logs have printed sequentially, print the clean
  // table grid
  cout << table.str();

  if (hasRefrigerated) {
    cout << "  ❄ = Refrigerated item\n";
  }

  (void)
      showOutOfStock; // both modes show full list; stock column tells the story
  return shown;
}

// ─── Payment method selection ────────────────────────────────────────────────
unique_ptr<PaymentProcessor> choosePayment() {
  printBanner("  STEP 3 — SELECT PAYMENT METHOD  ", '-');
  cout << "\n";
  cout << "  [1]  UPI    — Pay with your UPI ID\n";
  cout << "               Format: yourname@bank  (e.g. ram@sbi, "
          "priya@paytm)\n\n";
  cout << "  [2]  Card   — Debit or Credit card\n";
  cout << "               Enter 16 digits, no spaces  (e.g. "
          "4111111111111111)\n\n";
  cout << "  [3]  Wallet — Digital wallet\n";
  cout << "               Enter wallet name  (e.g. AuraWallet, PhonePe, "
          "Paytm)\n";
  cout << "\n";

  int choice = readChoice("  Your payment choice [1/2/3]: ", 1, 3);

  if (choice == 1) {
    string vpa = readNonEmpty("\n  UPI ID: ");
    return make_unique<UPIAdapter>(vpa);
  } else if (choice == 2) {
    string card = readNonEmpty("\n  Card Number: ");
    return make_unique<CardAdapter>(card);
  } else {
    string wallet = readNonEmpty("\n  Wallet Name: ");
    return make_unique<WalletAdapter>(wallet);
  }
}

// ─── Print large SUCCESS / FAILED result banner ──────────────────────────────
void printTransactionResult(const Transaction &tx) {
  cout << "\n";
  if (tx.status == TransactionStatus::SUCCESS) {
    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout << "  ║                                                          ║\n";
    cout << "  ║          ✅   PAYMENT SUCCESSFUL   ✅                   ║\n";
    cout << "  ║                                                          ║\n";
    cout << "  ╠══════════════════════════════════════════════════════════╣\n";
    cout << "  ║  Transaction ID : " << left << setw(38) << tx.transactionId
         << "║\n";
    cout << "  ║  Item Purchased : " << left << setw(38) << tx.itemName
         << "║\n";
    cout << "  ║  Amount Paid    : Rs." << left << setw(35) << fixed
         << setprecision(2) << tx.amount << "║\n";
    cout << "  ║  Payment Method : " << left << setw(38) << tx.paymentMethod
         << "║\n";
    cout << "  ║  Timestamp      : " << left << setw(38)
         << tx.getTimestampString() << "║\n";
    cout << "  ║                                                          ║\n";
    cout << "  ║  🎉 Thank you for shopping at Aura Retail OS!           ║\n";
    cout << "  ║     Please collect your item from the dispenser slot.   ║\n";
    cout << "  ╚══════════════════════════════════════════════════════════╝\n";
  } else {
    cout << "  ╔══════════════════════════════════════════════════════════╗\n";
    cout << "  ║                                                          ║\n";
    cout << "  ║            ❌   PAYMENT FAILED   ❌                     ║\n";
    cout << "  ║                                                          ║\n";
    cout << "  ╠══════════════════════════════════════════════════════════╣\n";
    cout << "  ║  Transaction ID : " << left << setw(38) << tx.transactionId
         << "║\n";
    cout << "  ║  Item           : " << left << setw(38) << tx.itemName
         << "║\n";
    cout << "  ║  Reason         : Item out of stock, payment declined,  ║\n";
    cout << "  ║                   or required hardware not available.   ║\n";
    cout << "  ║                                                          ║\n";
    cout << "  ║  ℹ  You have NOT been charged.                          ║\n";
    cout << "  ║     Please try again or choose a different item.        ║\n";
    cout << "  ╚══════════════════════════════════════════════════════════╝\n";
  }
}

// ─── Admin Panel ─────────────────────────────────────────────────────────────
void runAdminPanel(Kiosk *foodKiosk, Kiosk *pharmacyKiosk,
                   Kiosk *emergencyKiosk) {

  // --- PIN Authentication ---
  printBanner("  ADMIN LOGIN  ");
  cout << "\n  This panel is restricted to authorised personnel only.\n";
  cout << "  You have 3 attempts to enter the correct PIN.\n\n";

  bool authenticated = false;
  for (int attempt = 1; attempt <= 3; attempt++) {
    cout << "  Enter Admin PIN (attempt " << attempt << "/3): ";
    string pin;
    getline(cin, pin);
    if (pin == ADMIN_PIN) {
      authenticated = true;
      break;
    }
    if (attempt < 3)
      cout << "  ❌ Incorrect PIN. Try again.\n\n";
  }

  if (!authenticated) {
    cout << "\n  🚫 Access denied. Too many incorrect attempts.\n";
    pressEnterToContinue();
    return;
  }

  cout << "\n  ✅ PIN accepted. Welcome, Admin.\n";
  pressEnterToContinue();

  // --- Admin Menu Loop ---
  bool adminRunning = true;
  while (adminRunning) {
    printBanner("  ADMIN PANEL — INVENTORY MANAGEMENT  ");
    cout << "\n";
    cout << "  [1]  Restock Food Kiosk        (Central Metro Station)\n";
    cout << "  [2]  Restock Pharmacy Kiosk    (City Hospital)\n";
    cout << "  [3]  Restock Emergency Kiosk   (Highway Outpost)\n";
    cout << "  [4]  View all kiosk stock\n";
    cout << "  [5]  Exit Admin Panel\n";
    cout << "\n";

    int adminChoice = readChoice("  Admin choice [1/2/3/4/5]: ", 1, 5);

    if (adminChoice == 5) {
      adminRunning = false;
      break;
    }

    if (adminChoice == 4) {
      // View stock report for both kiosks
      printBanner("  FOOD KIOSK \u2014 CURRENT STOCK  ", '-');
      showProductMenu(foodKiosk);
      printBanner("  PHARMACY KIOSK \u2014 CURRENT STOCK  ", '-');
      showProductMenu(pharmacyKiosk);
      printBanner("  EMERGENCY KIOSK \u2014 CURRENT STOCK  ", '-');
      showProductMenu(emergencyKiosk);
      pressEnterToContinue();
      continue;
    }

    // Restock flow
    Kiosk *targetKiosk;
    string targetLabel;
    if (adminChoice == 1) {
      targetKiosk = foodKiosk;
      targetLabel = "Food Kiosk \u2014 Central Metro Station";
    } else if (adminChoice == 2) {
      targetKiosk = pharmacyKiosk;
      targetLabel = "Pharmacy Kiosk \u2014 City Hospital";
    } else {
      targetKiosk = emergencyKiosk;
      targetLabel = "Emergency Kiosk \u2014 Highway Outpost";
    }

    printBanner("  RESTOCK: " + targetLabel + "  ", '-');
    vector<string> itemIds = showProductMenu(targetKiosk);

    if (itemIds.empty()) {
      cout << "\n  ⚠  No items found in this kiosk.\n";
      pressEnterToContinue();
      continue;
    }

    cout << "\n  [0]  Cancel — go back\n\n";

    int itemChoice = readChoice("  Select item to restock [0 to cancel]: ", 0,
                                (int)itemIds.size());

    if (itemChoice == 0)
      continue;

    string selectedId = itemIds[itemChoice - 1];
    auto selectedItem = targetKiosk->getInventory()->getItem(selectedId);
    int currentStock = targetKiosk->getInventory()->getStock(selectedId);

    cout << "\n  ─────────────────────────────────────────────────────\n";
    cout << "  Item          : " << selectedItem->getName() << "\n";
    cout << "  Current Stock : " << currentStock << " unit(s)\n";
    cout << "  ─────────────────────────────────────────────────────\n\n";

    int qty = readPositiveInt("  Enter quantity to add (e.g. 10, 50): ");

    // Confirm before applying
    cout << "\n  Confirm: Add " << qty << " unit(s) of \""
         << selectedItem->getName() << "\" to stock?\n";
    cout << "  [1]  Yes — apply restock\n";
    cout << "  [2]  No  — cancel\n\n";

    int confirm = readChoice("  Confirm [1/2]: ", 1, 2);
    if (confirm == 2) {
      cout << "  Restock cancelled.\n";
      pressEnterToContinue();
      continue;
    }

    targetKiosk->restockInventory(selectedId, qty);

    // Save immediately \u2014 admin changes must not be lost on unexpected
    // close
    string saveFile;
    if (adminChoice == 1)
      saveFile = FOOD_INV_FILE;
    else if (adminChoice == 2)
      saveFile = PHARMA_INV_FILE;
    else
      saveFile = EMERGENCY_INV_FILE;

    PersistenceManager::saveInventoryToFile(targetKiosk->getInventory(),
                                            saveFile);

    int newStock = targetKiosk->getInventory()->getStock(selectedId);
    cout << "\n  ✅ Restock complete! Inventory saved to disk.\n";
    cout << "     \"" << selectedItem->getName()
         << "\" — New stock: " << newStock << " unit(s)\n";
    pressEnterToContinue();
  }

  cout << "\n  👋 Admin session ended.\n";
  pressEnterToContinue();
}

// ─── Main ────────────────────────────────────────────────────────────────────

int main() {

  // ── SYSTEM SETUP ─────────────────────────────────────────────────────
#ifdef _WIN32
  // Force Windows console to use UTF-8 so emojis and box characters render
  // correctly
  SetConsoleOutputCP(CP_UTF8);
#endif

  // Pre-configured kiosks and products. Users never enter these manually.

  // --- Food Kiosk ---
  auto foodKioskBase =
      KioskFactory::createKiosk("food", "FD-S1", "Central Metro Station");

  auto water = make_shared<Product>("P-201", "Mineral Water", 20.0);
  auto sandwich = make_shared<Product>("P-202", "Veg Sandwich", 120.0, true);
  auto cola = make_shared<Product>("P-203", "Cold Cola Can", 40.0);
  auto chips = make_shared<Product>("P-204", "Potato Chips", 30.0);
  auto coffee = make_shared<Product>("P-205", "Hot Coffee", 60.0);

  foodKioskBase->addProduct(water, 20);
  foodKioskBase->addProduct(sandwich, 5);
  foodKioskBase->addProduct(cola, 15);
  foodKioskBase->addProduct(chips, 10);
  foodKioskBase->addProduct(coffee, 8);

  // --- Pharmacy Kiosk (refrigerated) ---
  auto pharmacyKioskBase =
      KioskFactory::createKiosk("pharmacy", "PH-H1", "City Hospital");
  pharmacyKioskBase->setDispenser(make_unique<RefrigeratedDispenser>(4.0));

  auto insulin = make_shared<Product>("P-102", "Insulin Pen", 850.0, true);
  auto vitamin = make_shared<Product>("P-104", "Vitamin C Tabs", 80.0);
  auto crocin = make_shared<Product>("P-101", "Crocin 500mg", 50.0);
  auto bandage = make_shared<Product>("P-103", "Bandage Roll", 30.0);

  auto firstAid = make_shared<Bundle>("B-001", "Basic First Aid Kit", 10.0);
  firstAid->add(crocin);
  firstAid->add(bandage);

  pharmacyKioskBase->addProduct(insulin, 5);
  pharmacyKioskBase->addProduct(firstAid, 10);
  pharmacyKioskBase->addProduct(vitamin, 12);

  // Wrap pharmacy kiosk with decorator modules
  unique_ptr<Kiosk> pharmacyKiosk =
      make_unique<RefrigerationModule>(std::move(pharmacyKioskBase), 4.0);
  pharmacyKiosk = make_unique<NetworkModule>(std::move(pharmacyKiosk));

  // --- Emergency Kiosk (isolated network) ---
  auto emergencyKioskBase =
      KioskFactory::createKiosk("emergency", "EM-10", "Highway Outpost");

  auto flashlight = make_shared<Product>("P-301", "LED Flashlight", 250.0);
  auto blanket = make_shared<Product>("P-302", "Survival Blanket", 400.0);
  auto matches = make_shared<Product>("P-303", "Waterproof Matches", 50.0);

  emergencyKioskBase->addProduct(flashlight, 15);
  emergencyKioskBase->addProduct(blanket, 10);
  emergencyKioskBase->addProduct(matches, 30);

  unique_ptr<Kiosk> emergencyKiosk =
      make_unique<NetworkModule>(std::move(emergencyKioskBase));

  // Load historical inventory data (overwrites default stocks)
  PersistenceManager::loadInventoryFromFile(foodKioskBase->getInventory(),
                                            FOOD_INV_FILE);
  PersistenceManager::loadInventoryFromFile(pharmacyKiosk->getInventory(),
                                            PHARMA_INV_FILE);
  PersistenceManager::loadInventoryFromFile(emergencyKiosk->getInventory(),
                                            EMERGENCY_INV_FILE);

  // Registry
  CentralRegistry &registry = CentralRegistry::getInstance();
  registry.registerKiosk("FD-S1");
  registry.registerKiosk("PH-H1");
  registry.registerKiosk("EM-10");

  // ── WELCOME SCREEN ─────────────────────────────────────────────────────
  clearScreen();
  cout << "\n";
  cout
      << "  ╔══════════════════════════════════════════════════════════════╗\n";
  cout
      << "  ║                                                              ║\n";
  cout
      << "  ║           🌟   AURA RETAIL KIOSK SYSTEM   🌟                 ║\n";
  cout
      << "  ║                                                              ║\n";
  cout
      << "  ║   Welcome! Browse products and pay with UPI, Card or Wallet. ║\n";
  cout
      << "  ║   Admins can log in to manage inventory stock levels.        ║\n";
  cout
      << "  ║                                                              ║\n";
  cout
      << "  ╚══════════════════════════════════════════════════════════════╝\n";

  // ── ROLE SELECTION LOOP ────────────────────────────────────────────────
  bool running = true;
  while (running) {

    printBanner("  WHO ARE YOU?  ");
    cout << "\n";
    cout << "  [1]  🛒  Customer  — Browse products and make a purchase\n";
    cout << "  [2]  🔐  Admin     — Manage kiosk inventory (PIN required)\n";
    cout << "  [3]  🚪  Exit\n";
    cout << "\n";

    int role = readChoice("  Select your role [1/2/3]: ", 1, 3);

    // ── EXIT ─────────────────────────────────────────────────────────
    if (role == 3) {
      running = false;
      break;
    }

    // ── ADMIN PANEL ───────────────────────────────────────────────────
    if (role == 2) {
      runAdminPanel(foodKioskBase.get(), pharmacyKiosk.get(),
                    emergencyKiosk.get());
      continue;
    }

    // ── CUSTOMER SHOPPING LOOP ────────────────────────────────────────
    bool keepShopping = true;
    while (keepShopping) {

      // STEP 1: Choose Kiosk
      printBanner("  STEP 1 — SELECT A KIOSK  ");
      cout << "\n";
      cout << "  [1]  🍔  Food Kiosk\n";
      cout << "           Snacks, beverages & more\n";
      cout << "           Location: Central Metro Station\n\n";
      cout << "  [2]  💊  Pharmacy Kiosk\n";
      cout << "           Medicines & health products (refrigerated storage)\n";
      cout << "           Location: City Hospital\n\n";
      cout << "  [3]  🚨  Emergency Kiosk\n";
      cout << "           Survival tools & basic first aid\n";
      cout << "           Location: Highway Outpost\n\n";
      cout << "  [0]  ← Back to role selection\n";
      cout << "\n";

      int kioskChoice = readChoice("  Select kiosk [0/1/2/3]: ", 0, 3);
      if (kioskChoice == 0)
        break; // back to role menu

      Kiosk *selectedKiosk;
      string kioskLabel;
      if (kioskChoice == 1) {
        selectedKiosk = foodKioskBase.get();
        kioskLabel = "🍔  Food Kiosk";
      } else if (kioskChoice == 2) {
        selectedKiosk = pharmacyKiosk.get();
        kioskLabel = "💊  Pharmacy Kiosk";
      } else {
        selectedKiosk = emergencyKiosk.get();
        kioskLabel = "🚨  Emergency Kiosk";
      }

      // STEP 2: Show Products
      printBanner("  STEP 2 — " + kioskLabel + "  ");

      vector<string> productIds = showProductMenu(selectedKiosk);

      if (productIds.empty()) {
        cout << "\n  ⚠  This kiosk has no products at the moment.\n";
        pressEnterToContinue();
        continue;
      }

      cout << "\n  [0]  ← Back to kiosk selection\n\n";

      int productChoice =
          readChoice("  Enter product number to purchase [0 to go back]: ", 0,
                     (int)productIds.size());

      if (productChoice == 0)
        continue;

      string chosenItemId = productIds[productChoice - 1];
      auto chosenItem = selectedKiosk->getInventory()->getItem(chosenItemId);
      int chosenStock = selectedKiosk->getInventory()->getStock(chosenItemId);

      // Confirm selection
      cout << "\n  ─────────────────────────────────────────────────────\n";
      cout << "  You selected:\n";
      cout << "    Product      : " << chosenItem->getName() << "\n";
      cout << "    Price        : Rs." << fixed << setprecision(2)
           << chosenItem->getPrice() << "\n";
      cout << "    In Stock     : " << chosenStock << " unit(s)\n";
      if (chosenItem->requiresRefrigeration())
        cout << "    Storage Note : ❄  Refrigerated — dispensed from cold "
                "chain\n";
      cout << "  ─────────────────────────────────────────────────────\n";
      cout << "\n  Confirm purchase?\n";
      cout << "  [1]  Yes — proceed to payment\n";
      cout << "  [2]  No  — go back to product list\n\n";

      int confirm = readChoice("  Enter your choice [1/2]: ", 1, 2);
      if (confirm == 2)
        continue;

      // STEP 2b: How many units?
      cout << "\n  ─────────────────────────────────────────────────────\n";
      cout << "  How many units of \"" << chosenItem->getName()
           << "\" do you want?\n";
      cout << "  (Available: " << chosenStock
           << " unit(s)  |  Price per unit: Rs." << fixed << setprecision(2)
           << chosenItem->getPrice() << ")\n\n";

      int wantedQty = readPositiveInt("  Enter quantity: ");

      // Cap to available stock
      if (wantedQty > chosenStock) {
        cout << "\n  ⚠  Only " << chosenStock << " unit(s) available."
             << " Adjusting quantity to " << chosenStock << ".\n";
        wantedQty = chosenStock;
      }

      double totalCost = chosenItem->getPrice() * wantedQty;
      cout << "\n  Total for " << wantedQty << " unit(s): Rs." << fixed
           << setprecision(2) << totalCost << "\n";
      cout << "  ─────────────────────────────────────────────────────\n";

      // STEP 3: Payment (asked once for the whole order)
      unique_ptr<PaymentProcessor> payProc = choosePayment();
      selectedKiosk->setPaymentStrategy(std::move(payProc));

      // STEP 4: Execute purchase — one unit at a time
      printBanner("  PROCESSING YOUR ORDER...  ");
      int successCount = 0;
      int failCount = 0;
      string lastPayMethod;

      for (int u = 0; u < wantedQty; u++) {
        cout << "  Unit " << (u + 1) << " of " << wantedQty << "...\n";
        Transaction tx = selectedKiosk->purchaseItem(chosenItemId);
        if (tx.status == TransactionStatus::SUCCESS) {
          successCount++;
          lastPayMethod = tx.paymentMethod;
          registry.recordTransaction(tx);
          cout << "    \u2705 Unit " << (u + 1) << " dispensed.\n";
        } else {
          failCount++;
          cout << "    \u274c Unit " << (u + 1)
               << " failed (out of stock / hardware error).\n";
          break; // stock exhausted, stop trying
        }
      }

      // Save all inventories immediately after the order
      PersistenceManager::saveInventoryToFile(foodKioskBase->getInventory(),
                                              FOOD_INV_FILE);
      PersistenceManager::saveInventoryToFile(pharmacyKiosk->getInventory(),
                                              PHARMA_INV_FILE);
      PersistenceManager::saveInventoryToFile(emergencyKiosk->getInventory(),
                                              EMERGENCY_INV_FILE);

      // STEP 5: Order Summary
      cout << "\n";
      cout
          << "  ╔══════════════════════════════════════════════════════════╗\n";
      if (successCount == wantedQty) {
        cout << "  ║          ✅   ORDER COMPLETE — ALL UNITS DISPENSED   ✅ "
                "║\n";
      } else if (successCount > 0) {
        cout << "  ║      ⚠   ORDER PARTIAL — SOME UNITS DISPENSED        ⚠  "
                "║\n";
      } else {
        cout << "  ║             ❌   ORDER FAILED — NO UNITS DISPENSED   ❌ "
                "║\n";
      }
      cout
          << "  ╠══════════════════════════════════════════════════════════╣\n";
      cout << "  ║  Item            : " << left << setw(37)
           << chosenItem->getName() << "║\n";
      cout << "  ║  Units Requested : " << left << setw(37) << wantedQty
           << "║\n";
      cout << "  ║  Units Dispensed : " << left << setw(37) << successCount
           << "║\n";
      cout << "  ║  Units Failed    : " << left << setw(37) << failCount
           << "║\n";
      {
        ostringstream oss;
        oss << fixed << setprecision(2)
            << (successCount * chosenItem->getPrice());
        cout << "  ║  Amount Charged  : Rs." << left << setw(34) << oss.str()
             << "║\n";
      }
      if (!lastPayMethod.empty())
        cout << "  ║  Payment Method  : " << left << setw(37) << lastPayMethod
             << "║\n";
      cout << "  ║  Inventory File  : Updated ✔                            ║\n";
      cout
          << "  ╚══════════════════════════════════════════════════════════╝\n";
      if (successCount > 0)
        cout << "  🎉 Please collect your item(s) from the dispenser slot.\n";

      // STEP 6: Continue or exit
      cout << "\n\n  What would you like to do next?\n\n";
      cout << "  [1]  Buy another item\n";
      cout << "  [2]  Exit to main menu\n\n";

      int nextAction = readChoice("  Enter your choice [1/2]: ", 1, 2);
      if (nextAction == 2)
        keepShopping = false;
    }
  }

  // ── SHUTDOWN ──────────────────────────────────────────────────────────
  printBanner("  SESSION SUMMARY  ");
  registry.displayGlobalReport();

  // Save all kiosk inventories on clean exit
  PersistenceManager::saveInventoryToFile(foodKioskBase->getInventory(),
                                          FOOD_INV_FILE);
  PersistenceManager::saveInventoryToFile(pharmacyKiosk->getInventory(),
                                          PHARMA_INV_FILE);
  PersistenceManager::saveInventoryToFile(emergencyKiosk->getInventory(),
                                          EMERGENCY_INV_FILE);

  cout << "\n";
  cout
      << "  ╔══════════════════════════════════════════════════════════════╗\n";
  cout << "  ║   🙏  Thank you for using Aura Retail OS!                   ║\n";
  cout
      << "  ║       Inventory saved. Session data recorded.                ║\n";
  cout
      << "  "
         "╚══════════════════════════════════════════════════════════════╝\n\n";

  return 0;
}
