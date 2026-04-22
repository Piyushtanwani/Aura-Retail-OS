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
#include <cstdlib> // for system()
#include <ctime>   // for time()

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

// ─── Twilio Configuration (IMPORTANT: Replace with your actual credentials) ──
static const string TWILIO_ACCOUNT_SID = "YOUR_TWILIO_SID";
static const string TWILIO_AUTH_TOKEN  = "YOUR_TWILIO_AUTH_TOKEN";
static const string TWILIO_FROM_NUMBER = "+14782150594";
static const string ADMIN_PHONE        = "+919574713600";

// ─── SMS Helper ──────────────────────────────────────────────────────────────
void sendSMS(const string &to, const string &body) {
  // Construct curl command to send SMS via Twilio API
  // Using -s for silent, -k if needed (but twilio is https), and redirection to nul/dev/null
  string command = "curl -s -X POST \"https://api.twilio.com/2010-04-01/Accounts/" +
                   TWILIO_ACCOUNT_SID + "/Messages.json\" " +
                   "--data-urlencode \"To=" + to + "\" " +
                   "--data-urlencode \"From=" + TWILIO_FROM_NUMBER + "\" " +
                   "--data-urlencode \"Body=" + body + "\" " +
                   "-u " + TWILIO_ACCOUNT_SID + ":" + TWILIO_AUTH_TOKEN;

#ifdef _WIN32
  command += " > nul 2>&1";
#else
  command += " > /dev/null 2>&1";
#endif

  system(command.c_str());
}

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

// ─── Payment Validation Helpers ──────────────────────────────────────────────

bool isValidUPI(const string &upi) {
  size_t atPos = upi.find('@');
  return (atPos != string::npos && atPos > 0 && atPos < upi.length() - 1);
}

bool isValidCard(const string &card) {
  if (card.length() != 16)
    return false;
  for (char c : card) {
    if (!isdigit(c))
      return false;
  }
  return true;
}

bool isValidCVV(const string &cvv) {
  if (cvv.length() != 3)
    return false;
  for (char c : cvv) {
    if (!isdigit(c))
      return false;
  }
  return true;
}

bool isValidExpiry(const string &expiry) {
  if (expiry.length() != 5 || expiry[2] != '/')
    return false;

  try {
    int month = stoi(expiry.substr(0, 2));
    int year = stoi(expiry.substr(3, 2)) + 2000;

    if (month < 1 || month > 12)
      return false;

    // Get current time
    time_t t = time(0);
    tm *now = localtime(&t);
    int currMonth = now->tm_mon + 1;
    int currYear = now->tm_year + 1900;

    if (year < currYear)
      return false;
    if (year == currYear && month < currMonth)
      return false;

    return true;
  } catch (...) {
    return false;
  }
}

bool isValidPhone(const string &phone) {
  if (phone.length() != 10)
    return false;
  for (char c : phone) {
    if (!isdigit(c))
      return false;
  }
  return true;
}

bool isValidWallet(const string &wallet) { return wallet.length() >= 3; }

// ─── Show numbered product catalogue (customer view — out-of-stock included) ─
// Returns parallel vector of item IDs.
vector<string> showProductMenu(Kiosk *kiosk, bool showOutOfStock = true) {
  vector<string> ids = kiosk->getInventory()->getAllItemIds();
  vector<string> shown;

  ostringstream table;
  table << "\n";
  table << "  "
           "┌──────┬───────────────────────────────────────────────────────────"
           "────────────────┬──────────┬────────────────────┐\n";
  table << "  │  No. │  Product Name                                           "
           "                  │  Price   │    Stock           │\n";
  table << "  "
           "├──────┼───────────────────────────────────────────────────────────"
           "────────────────┼──────────┼────────────────────┤\n";

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
    int namePad = 73 - nameVisLen;
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
    int stockPad = 12 - (int)stockStr.length();
    if (stockPad > 0)
      stockStr.append(stockPad, ' ');

    table << "  │ [" << left << setw(2) << num << "] │  " << paddedName << "│  "
          << priceStr << "│    " << stockStr << "    │\n";

    shown.push_back(id);
    num++;
  }

  table << "  "
           "└──────┴───────────────────────────────────────────────────────────"
           "────────────────┴──────────┴────────────────────┘\n";

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
    while (true) {
      string vpa = readNonEmpty("\n  UPI ID: ");
      if (isValidUPI(vpa))
        return make_unique<UPIAdapter>(vpa);
      cout << "  ⚠  Invalid UPI ID format. Please use format: name@bank\n";
    }
  } else if (choice == 2) {
    while (true) {
      string card = readNonEmpty("\n  Card Number (16 digits): ");
      if (!isValidCard(card)) {
        cout << "  ⚠  Invalid Card. Please enter exactly 16 digits.\n";
        continue;
      }
      
      string expiry = readNonEmpty("  Expiry Date (MM/YY): ");
      if (!isValidExpiry(expiry)) {
        cout << "  ⚠  Invalid or Expired date. Use MM/YY format (e.g., 12/28).\n";
        continue;
      }

      string cvv = readNonEmpty("  CVV (3 digits): ");
      if (!isValidCVV(cvv)) {
        cout << "  ⚠  Invalid CVV. Please enter exactly 3 digits.\n";
        continue;
      }

      return make_unique<CardAdapter>(card, expiry, cvv);
    }
  } else {
    while (true) {
      string wallet = readNonEmpty("\n  Wallet Name: ");
      if (isValidWallet(wallet))
        return make_unique<WalletAdapter>(wallet);
      cout << "  ⚠  Invalid Wallet. Please enter a valid wallet name (min 3 "
              "chars).\n";
    }
  }
}

// ─── Admin Panel ─────────────────────────────────────────────────────────────
void runAdminPanel(Kiosk *foodKiosk, Kiosk *pharmacyKiosk,
                   Kiosk *emergencyKiosk) {

  // --- OTP Authentication ---
  printBanner("  ADMIN AUTHENTICATION (OTP)  ");
  cout << "\n  OTP will be sent to the registered Admin mobile number (" << ADMIN_PHONE << ").\n";
  
  bool authenticated = false;

  // Generate 6-digit OTP
  int otpVal = 100000 + (rand() % 900000);
  string otpStr = to_string(otpVal);

  cout << "  🚀 Sending OTP...";
  sendSMS(ADMIN_PHONE, "Your Aura Retail OS Admin OTP is: " + otpStr);
  cout << " Done!\n\n";

  for (int attempt = 1; attempt <= 3; attempt++) {
    cout << "  Enter 6-digit OTP (attempt " << attempt << "/3): ";
    string inputOtp;
    getline(cin, inputOtp);
    if (inputOtp == otpStr) {
      authenticated = true;
      break;
    }
    if (attempt < 3)
      cout << "  ❌ Invalid OTP. Please check your messages.\n\n";
  }

  if (!authenticated) {
    cout << "\n  🚫 Access denied. Authentication failed.\n";
    pressEnterToContinue();
    return;
  }

  cout << "\n  ✅ Access granted. Welcome, Admin.\n";
  pressEnterToContinue();

  // --- Admin Menu Loop ---
  bool adminRunning = true;
  while (adminRunning) {
    printBanner("  ADMIN PANEL — INVENTORY MANAGEMENT  ");
    cout << "\n";
    cout << "  [1]  Manage Food Kiosk        (Central Metro Station)\n";
    cout << "  [2]  Manage Pharmacy Kiosk    (City Hospital)\n";
    cout << "  [3]  Manage Emergency Kiosk   (Highway Outpost)\n";
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

    // Manage flow
    Kiosk *targetKiosk;
    string targetLabel;
    string saveFile;
    if (adminChoice == 1) {
      targetKiosk = foodKiosk;
      targetLabel = "Food Kiosk \u2014 Central Metro Station";
      saveFile = FOOD_INV_FILE;
    } else if (adminChoice == 2) {
      targetKiosk = pharmacyKiosk;
      targetLabel = "Pharmacy Kiosk \u2014 City Hospital";
      saveFile = PHARMA_INV_FILE;
    } else {
      targetKiosk = emergencyKiosk;
      targetLabel = "Emergency Kiosk \u2014 Highway Outpost";
      saveFile = EMERGENCY_INV_FILE;
    }

    bool manageRunning = true;
    while (manageRunning) {
      printBanner("  MANAGE: " + targetLabel + "  ", '-');
      cout << "\n";
      cout << "  [1]  Restock Existing Product\n";
      cout << "  [2]  Add New Product\n";
      cout << "  [3]  Manage Kiosk Bundle(s)\n";
      cout << "  [0]  Back to Kiosk Selection\n\n";

      int manageChoice = readChoice("  Enter choice [0/1/2/3]: ", 0, 3);
      if (manageChoice == 0) {
        manageRunning = false;
        continue;
      }

      if (manageChoice == 1) {
        printBanner("  RESTOCK: " + targetLabel + "  ", '-');
        vector<string> itemIds = showProductMenu(targetKiosk);

        if (itemIds.empty()) {
          cout << "\n  ⚠  No items found in this kiosk.\n";
          pressEnterToContinue();
          continue;
        }

        cout << "\n  [0]  Cancel — go back\n\n";

        int itemChoice = readChoice(
            "  Select item to restock [0 to cancel]: ", 0, (int)itemIds.size());

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
        PersistenceManager::saveInventoryToFile(targetKiosk->getInventory(),
                                                saveFile);

        int newStock = targetKiosk->getInventory()->getStock(selectedId);
        cout << "\n  ✅ Restock complete! Inventory saved to disk.\n";
        cout << "     \"" << selectedItem->getName()
             << "\" — New stock: " << newStock << " unit(s)\n";
        pressEnterToContinue();
      } else if (manageChoice == 2) {
        printBanner("  ADD NEW PRODUCT TO KIOSK  ", '-');
        string newId = readNonEmpty("  Enter Product ID (e.g. P-999): ");

        if (targetKiosk->getInventory()->getItem(newId) != nullptr) {
          cout << "\n  ❌ Product ID already exists in this kiosk.\n";
          pressEnterToContinue();
          continue;
        }

        string newName = readNonEmpty("  Enter Product Name: ");
        cout << "  Enter Price: ";
        string rawPrice;
        getline(cin, rawPrice);
        double price = 0.0;
        try {
          price = stod(rawPrice);
        } catch (...) {
          price = 10.0;
        }

        cout << "  Requires Refrigeration? [1] Yes [2] No: ";
        int refChoice = readChoice("", 1, 2);
        bool requiresRefrigeration = (refChoice == 1);

        int initialQty = readPositiveInt("  Enter Initial Quantity: ");

        auto newProduct =
            make_shared<Product>(newId, newName, price, requiresRefrigeration);
        targetKiosk->addProduct(newProduct, initialQty);
        PersistenceManager::saveInventoryToFile(targetKiosk->getInventory(),
                                                saveFile);

        cout << "\n  ✅ Product added successfully!\n";
        pressEnterToContinue();
      } else if (manageChoice == 3) {
        printBanner("  MANAGE BUNDLES  ", '-');
        vector<string> allIds = targetKiosk->getInventory()->getAllItemIds();
        vector<shared_ptr<Bundle>> bundles;

        for (const string &id : allIds) {
          auto item = targetKiosk->getInventory()->getItem(id);
          auto bundleOpt = std::dynamic_pointer_cast<Bundle>(item);
          if (bundleOpt) {
            bundles.push_back(bundleOpt);
          }
        }

        if (bundles.empty()) {
          cout << "\n  ⚠  No bundles exist in this kiosk.\n";
          pressEnterToContinue();
          continue;
        }

        for (size_t i = 0; i < bundles.size(); ++i) {
          cout << "  [" << (i + 1) << "]  " << bundles[i]->getName()
               << " (ID: " << bundles[i]->getId() << ")\n";
        }
        cout << "  [0]  Cancel\n\n";

        int bChoice = readChoice("  Select bundle to manage [0 to cancel]: ", 0,
                                 (int)bundles.size());
        if (bChoice == 0)
          continue;

        auto selectedBundle = bundles[bChoice - 1];

        cout << "\n  [1] Restock Bundle\n";
        cout << "  [2] Add Product to Bundle\n";
        cout << "  [0] Cancel\n\n";
        int bAct = readChoice("  Select action [0/1/2]: ", 0, 2);
        if (bAct == 0)
          continue;

        if (bAct == 1) {
          int qty =
              readPositiveInt("  Enter quantity to add to bundle stock: ");
          targetKiosk->restockInventory(selectedBundle->getId(), qty);
          PersistenceManager::saveInventoryToFile(targetKiosk->getInventory(),
                                                  saveFile);
          cout << "\n  ✅ Bundle restocked successfully!\n";
          pressEnterToContinue();
        } else if (bAct == 2) {
          cout << "\n  Select a product from the kiosk to add to the bundle:\n";
          vector<string> prodIds = showProductMenu(targetKiosk);
          if (prodIds.empty()) {
            cout << "\n  ⚠  No products available.\n";
            pressEnterToContinue();
            continue;
          }
          cout << "\n  [0]  Cancel\n\n";
          int pChoice = readChoice("  Select item [0 to cancel]: ", 0,
                                   (int)prodIds.size());
          if (pChoice == 0)
            continue;

          string pIdToAdd = prodIds[pChoice - 1];
          auto itemToAdd = targetKiosk->getInventory()->getItem(pIdToAdd);
          if (std::dynamic_pointer_cast<Bundle>(itemToAdd)) {
            cout << "\n  ❌ Nested bundles are better avoided in this UI. "
                    "Please add a Product.\n";
            pressEnterToContinue();
            continue;
          }

          selectedBundle->add(itemToAdd);
          PersistenceManager::saveInventoryToFile(targetKiosk->getInventory(),
                                                  saveFile);
          cout << "\n  ✅ Item \"" << itemToAdd->getName()
               << "\" added to bundle \"" << selectedBundle->getName()
               << "\"!\n";
          pressEnterToContinue();
        }
      }
    }
  }

  cout << "\n  👋 Admin session ended.\n";
  pressEnterToContinue();
}

// ─── Main ────────────────────────────────────────────────────────────────────

int main() {

  // ── SYSTEM SETUP ─────────────────────────────────────────────────────
  srand(static_cast<unsigned int>(time(0))); // Seed for OTP generation

#ifdef _WIN32
  // Force Windows console to use UTF-8 so emojis and box characters render
  // correctly
  SetConsoleOutputCP(CP_UTF8);
#endif

  // Pre-configured kiosks and products. Users never enter these manually.

  // --- Food Kiosk ---
  auto foodKioskBase =
      KioskFactory::createKiosk("food", "FD-S1", "Central Metro Station");
  foodKioskBase->setDispenser(make_unique<RefrigeratedDispenser>(8.0));

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

  auto snackCombo = make_shared<Bundle>("B-002", "Snack Combo", 15.0);
  snackCombo->add(sandwich);
  snackCombo->add(cola);
  snackCombo->add(chips);
  foodKioskBase->addProduct(snackCombo, 5);

  unique_ptr<Kiosk> foodKiosk =
      make_unique<RefrigerationModule>(std::move(foodKioskBase), 8.0);

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
  firstAid->add(vitamin);

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

  auto survivalPack = make_shared<Bundle>("B-003", "Survival Pack", 20.0);
  survivalPack->add(flashlight);
  survivalPack->add(blanket);
  survivalPack->add(matches);
  emergencyKioskBase->addProduct(survivalPack, 10);

  unique_ptr<Kiosk> emergencyKiosk =
      make_unique<NetworkModule>(std::move(emergencyKioskBase));

  // Load historical inventory data (overwrites default stocks)
  PersistenceManager::loadInventoryFromFile(foodKiosk->getInventory(),
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
      runAdminPanel(foodKiosk.get(), pharmacyKiosk.get(), emergencyKiosk.get());
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
        selectedKiosk = foodKiosk.get();
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

      // --- STEP 3: Payment Authentication & Method ---
      printBanner("  STEP 3 — PAYMENT  ", '-');
      
      string customerPhone;
      while (true) {
        customerPhone = readNonEmpty("\n  Enter your 10-digit mobile number for Payment OTP: ");
        if (isValidPhone(customerPhone)) break;
        cout << "  ⚠  Invalid mobile number. Please enter exactly 10 numeric digits.\n";
      }
      string fullCustomerPhone = "+91" + customerPhone;
      
      int payOtp = 100000 + (rand() % 900000);
      string payOtpStr = to_string(payOtp);
      
      cout << "  🚀 Sending payment OTP to " << fullCustomerPhone << "...";
      sendSMS(fullCustomerPhone, "Aura Retail OS - Payment OTP: " + payOtpStr + " for amount Rs." + to_string((int)totalCost));
      cout << " Done!\n";
      
      bool payAuth = false;
      for (int att = 1; att <= 3; att++) {
        cout << "  Enter 6-digit OTP (attempt " << att << "/3): ";
        string inPayOtp;
        getline(cin, inPayOtp);
        if (inPayOtp == payOtpStr) {
          payAuth = true;
          break;
        }
        if (att < 3) cout << "  ❌ Invalid OTP. Please check your messages.\n";
      }
      
      if (!payAuth) {
        cout << "\n  ❌ Payment authentication failed. Transaction cancelled.\n";
        pressEnterToContinue();
        continue;
      }
      cout << "  ✅ Authentication successful!\n\n";

      unique_ptr<PaymentProcessor> payProc = choosePayment();
      selectedKiosk->setPaymentStrategy(std::move(payProc));

      // STEP 4: Execute purchase — batch in a single transaction
      printBanner("  PROCESSING YOUR ORDER...  ");
      int successCount = 0;
      int failCount = 0;
      string lastPayMethod;

      Transaction tx = selectedKiosk->purchaseItem(chosenItemId, wantedQty);
      if (tx.status == TransactionStatus::SUCCESS) {
        successCount = wantedQty;
        lastPayMethod = tx.paymentMethod;
        registry.recordTransaction(tx);
        cout << "    \u2705 " << wantedQty
             << " unit(s) dispensed and grouped in a single transaction.\n";
      } else {
        // If transaction failed, we assume 0 units dispensed since it's atomic
        // now.
        failCount = wantedQty;
        cout << "    \u274c Order failed (out of stock, payment declined, or "
                "hardware error).\n";
      }

      // Save all inventories immediately after the order
      PersistenceManager::saveInventoryToFile(foodKiosk->getInventory(),
                                              FOOD_INV_FILE);
      PersistenceManager::saveInventoryToFile(pharmacyKiosk->getInventory(),
                                              PHARMA_INV_FILE);
      PersistenceManager::saveInventoryToFile(emergencyKiosk->getInventory(),
                                              EMERGENCY_INV_FILE);

      // STEP 5: Order Summary
      cout << "\n";
      cout << "  "
              "╔═══════════════════════════════════════════════════════════════"
              "══"
              "════"
              "════════╗"
              "\n";
      if (successCount == wantedQty && successCount > 0) {
        cout << "  ║              ✅   PAYMENT SUCCESSFUL   ✅                 "
                "                  ║\n";
        cout << "  ║      ✅   ORDER COMPLETE — ALL UNITS DISPENSED   ✅       "
                "                  ║\n";
      } else if (successCount > 0) {
        cout << "  ║         ⚠   PAYMENT PARTIALLY SUCCESSFUL   ⚠              "
                "                  ║\n";
        cout << "  ║         ⚠   ORDER PARTIAL — SOME DISPENSED FAILED  ⚠      "
                "                  ║\n";
      } else {
        cout << "  ║                ❌   PAYMENT FAILED   ❌                   "
                "                  ║\n";
        cout << "  ║             ❌  ORDER FAILED — NO UNITS DISPENSED  ❌     "
                "                  ║\n";
      }
      cout << "  "
              "╠═══════════════════════════════════════════════════════════════"
              "══"
              "════"
              "════════╣"
              "\n";
      cout << "  ║  Item            : " << left << setw(55)
           << chosenItem->getName() << "  ║\n";
      cout << "  ║  Units Requested : " << left << setw(55) << wantedQty
           << "  ║\n";
      cout << "  ║  Units Dispensed : " << left << setw(55) << successCount
           << "  ║\n";
      cout << "  ║  Units Failed    : " << left << setw(55) << failCount
           << "  ║\n";
      {
        ostringstream oss;
        oss << fixed << setprecision(2)
            << (successCount * chosenItem->getPrice());
        cout << "  ║  Amount Charged  : Rs." << left << setw(52) << oss.str()
             << "  ║\n";
      }
      if (!lastPayMethod.empty())
        cout << "  ║  Payment Method  : " << left << setw(55) << lastPayMethod
             << "  ║\n";
      cout << "  ║  Inventory File  : Updated ✔                                "
              "                ║\n";
      if (successCount == 0) {
        cout << "  ║                                                           "
                "                  ║\n";
        cout << "  ║  ℹ  You have NOT been charged.                            "
                "                  ║\n";
        cout << "  ║     Please try again or choose a different item.          "
                "                  ║\n";
      }
      cout << "  "
              "╚═══════════════════════════════════════════════════════════════"
              "══"
              "════"
              "════════╝"
              "\n";

      if (successCount > 0) {
        cout << "  🎉 Please collect your item(s) from the dispenser slot.\n";

        // --- Mandatory SMS Order Summary ---
        ostringstream smsBody;
        smsBody << "Aura Retail OS - ORDER SUMMARY\n"
                << "Item: " << chosenItem->getName() << "\n"
                << "Qty: " << successCount << "\n"
                << "Total: Rs." << fixed << setprecision(2)
                << (successCount * chosenItem->getPrice()) << "\n"
                << "Thank you for using Aura Retail!";

        cout << "\n  📨 Sending mandatory order summary to " << fullCustomerPhone
             << "...";
        sendSMS(fullCustomerPhone, smsBody.str());
        cout << " Sent!\n";
      }

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
  PersistenceManager::saveInventoryToFile(foodKiosk->getInventory(),
                                          FOOD_INV_FILE);
  PersistenceManager::saveInventoryToFile(pharmacyKiosk->getInventory(),
                                          PHARMA_INV_FILE);
  PersistenceManager::saveInventoryToFile(emergencyKiosk->getInventory(),
                                          EMERGENCY_INV_FILE);

  cout << "\n";
  cout
      << "  ╔══════════════════════════════════════════════════════════════╗\n";
  cout
      << "  ║   🙏  Thank you for using Aura Retail OS!                    ║\n";
  cout
      << "  ║       Inventory saved. Session data recorded.                ║\n";
  cout
      << "  "
         "╚══════════════════════════════════════════════════════════════╝\n\n";

  return 0;
}