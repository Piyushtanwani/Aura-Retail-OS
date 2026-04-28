/*
 * ============================================================================
 *  AURA RETAIL OS — Kiosk Base Class (Abstract)
 * ============================================================================
 *
 *  This is the abstract base class for all kiosk types. It defines the
 *  core interface and shared functionality that every kiosk must support:
 *    - Inventory management (via SecureInventory proxy)
 *    - Payment processing (via PaymentContext strategy)
 *    - Item dispensing (via Dispenser hardware)
 *    - Transaction recording (via CentralRegistry singleton)
 *
 *  Concrete kiosks (FoodKiosk, PharmacyKiosk, EmergencyKiosk) extend
 *  this class with domain-specific behavior.
 *
 *  The Decorator pattern wraps Kiosk objects to add optional modules
 *  (refrigeration, network, AI recommendation) dynamically.
 *
 * ============================================================================
 */

#ifndef KIOSK_H
#define KIOSK_H

#include <string>
#include <iostream>
#include <memory>
#include "../inventory/SecureInventory.h"
#include "../payment/PaymentContext.h"
#include "../payment/Transaction.h"
#include "../hardware/Dispenser.h"
#include "../hardware/StandardDispenser.h"

class Kiosk {
protected:
    std::string kioskId_;
    std::string kioskType_;
    std::string location_;
    std::unique_ptr<SecureInventory> inventory_;
    PaymentContext paymentContext_;
    std::unique_ptr<Dispenser> dispenser_;
    int transactionCounter_;

    // Generate a unique transaction ID
    std::string generateTransactionId() {
        return kioskId_ + "-TX-" + std::to_string(++transactionCounter_);
    }

public:
    Kiosk(const std::string& id, const std::string& type, const std::string& location)
        : kioskId_(id), kioskType_(type), location_(location),
          inventory_(std::make_unique<SecureInventory>()),
          dispenser_(std::make_unique<StandardDispenser>()),
          transactionCounter_(0) {}

    virtual ~Kiosk() = default;

    // ─── Core Accessors ─────────────────────────────────────────────────────
    std::string getId() const { return kioskId_; }
    std::string getType() const { return kioskType_; }
    std::string getLocation() const { return location_; }

    // ─── Inventory Access ───────────────────────────────────────────────────
    virtual SecureInventory* getInventory() { return inventory_.get(); }

    virtual void addProduct(std::shared_ptr<InventoryComponent> item, int qty) {
        inventory_->addItem(item, qty);
    }

    // ─── Payment Strategy ───────────────────────────────────────────────────
    virtual void setPaymentStrategy(std::unique_ptr<PaymentProcessor> strategy) {
        paymentContext_.setStrategy(std::move(strategy));
    }

    // ─── Hardware (Dispenser) ───────────────────────────────────────────────
    virtual void setDispenser(std::unique_ptr<Dispenser> disp) {
        std::cout << "  ⚙️  [SYSTEM] Hot-swapping hardware module: " << disp->getType() << " initialized.\n";
        dispenser_ = std::move(disp);
    }

    virtual std::string getDispenserType() const {
        return dispenser_ ? dispenser_->getType() : "None";
    }

    // ─── Core Operations ────────────────────────────────────────────────────

    // Purchase an item: check stock → pay → dispense → record
    virtual Transaction purchaseItem(const std::string& itemId, int quantity, const std::string& phone);

    // Refund a transaction
    virtual bool refundTransaction(const Transaction& tx);

    // Restock inventory
    virtual void restockInventory(const std::string& itemId, int quantity) {
        std::cout << "\n📦 [Kiosk:" << kioskId_ << "] Restocking item \"" << itemId << "\"...\n";
        inventory_->incrementStock(itemId, quantity);
    }

    // Dispense an item via current hardware
    virtual bool dispenseItem(const std::string& itemId, const std::string& itemName) {
        if (!dispenser_) {
            std::cout << "  ❌ [Kiosk] No dispenser attached!\n";
            return false;
        }
        return dispenser_->dispense(itemId, itemName);
    }

    // ─── Display Kiosk Info ─────────────────────────────────────────────────
    virtual void displayInfo() const {
        std::cout << "\n╔══════════════════════════════════════════════╗\n";
        std::cout << "║  🏪 KIOSK: " << kioskId_ << "\n";
        std::cout << "║  Type:     " << kioskType_ << "\n";
        std::cout << "║  Location: " << location_ << "\n";
        std::cout << "║  Dispenser: " << getDispenserType() << "\n";
        std::cout << "║  Payment:   " << paymentContext_.getCurrentMethod() << "\n";
        std::cout << "╚══════════════════════════════════════════════╝\n";
    }

    // ─── Decorator Hook: describe active modules ────────────────────────────
    virtual std::string describeModules() const {
        return "None";
    }
};

// ─── purchaseItem Implementation ────────────────────────────────────────────
// Implements atomic transaction: stock check → payment → dispense → record
inline Transaction Kiosk::purchaseItem(const std::string& itemId, int quantity, const std::string& phone) {
    if (quantity <= 0) quantity = 1;
    
    std::cout << "\n🛒 ═══════════════════════════════════════════════\n";
    std::cout << "🛒  COMMAND INVOCATION — Kiosk: " << kioskId_ << "\n";
    std::cout << "🛒  Action: PurchaseItemRequest\n";
    std::cout << "🛒 ═══════════════════════════════════════════════\n";

    auto item = inventory_->getItem(itemId);
    if (!item) {
        std::cout << "  ❌ Item not found in catalogue.\n";
        return Transaction(generateTransactionId(), kioskId_, itemId, "UNKNOWN",
                           quantity, 0.0, "N/A", phone, TransactionStatus::FAILED);
    }

    std::string txId = generateTransactionId();
    double unitPrice = item->getPrice();
    double totalPrice = unitPrice * quantity;
    std::string name = item->getName();

    // Step 1: Check stock (via proxy)
    std::cout << "\n  📋 Step 1: Checking stock...\n";
    int avail = inventory_->getStock(itemId);
    if (avail < quantity) {
        std::cout << "  ❌ INSUFFICIENT STOCK! Requested: " << quantity << ", Available: " << avail << "\n";
        std::cout << "  🚫 Skipping payment process. Purchase aborted.\n";
        return Transaction(txId, kioskId_, itemId, name, quantity, totalPrice,
                           paymentContext_.getCurrentMethod(), phone, TransactionStatus::FAILED);
    } else {
        std::cout << "  ✅ Stock available: " << avail << " units\n";
    }

    // Step 1.5: Check hardware dependency
    if (item->requiresRefrigeration() && getDispenserType().find("Refrigerated") == std::string::npos) {
        std::cout << "  ❄️  Required refrigeration module NOT ACTIVE\n";
        std::cout << "  ❌ Cannot dispense item\n";
        std::cout << "  🚫 Transaction aborted\n";
        return Transaction(txId, kioskId_, itemId, name, quantity, totalPrice,
                           paymentContext_.getCurrentMethod(), phone, TransactionStatus::FAILED);
    }

    // Step 2: Process payment (via strategy)
    std::cout << "\n  💰 Step 2: Processing payment of Rs." << totalPrice << "...\n";
    bool paymentOk = paymentContext_.pay(totalPrice);
    if (!paymentOk) {
        std::cout << "  ❌ Payment FAILED! Purchase aborted.\n";
        return Transaction(txId, kioskId_, itemId, name, quantity, totalPrice,
                           paymentContext_.getCurrentMethod(), phone, TransactionStatus::FAILED);
    }

    // Step 3: Decrement stock (atomic — rollback payment if this fails)
    std::cout << "\n  📦 Step 3: Updating inventory...\n";
    bool stockOk = inventory_->decrementStock(itemId, quantity);
    if (!stockOk) {
        std::cout << "  ⚠️  Stock update failed! Rolling back payment...\n";
        paymentContext_.refund(totalPrice);
        return Transaction(txId, kioskId_, itemId, name, quantity, totalPrice,
                           paymentContext_.getCurrentMethod(), phone, TransactionStatus::FAILED);
    }

    // Step 4: Dispense item(s) (rollback both if this fails)
    std::cout << "\n  ⚙️  Step 4: Dispensing item(s)...\n";
    int dispensedCount = 0;
    bool dispenseOk = true;
    for (int i = 0; i < quantity; ++i) {
        if (!dispenseItem(itemId, name)) {
            dispenseOk = false;
            break;
        }
        dispensedCount++;
    }
    
    if (!dispenseOk) {
        std::cout << "  ⚠️  Hardware dispensing failed after " << dispensedCount << " units!\n";
        std::cout << "  🔄 Rolling back transaction...\n";
        inventory_->incrementStock(itemId, quantity);
        std::cout << "  💰 Refund initiated...\n";
        paymentContext_.refund(totalPrice);
        return Transaction(txId, kioskId_, itemId, name, quantity, totalPrice,
                           paymentContext_.getCurrentMethod(), phone, TransactionStatus::FAILED);
    }

    // Success!
    std::cout << "\n  ✅ PURCHASE COMPLETE (" << quantity << " units)! Transaction: " << txId << "\n";
    return Transaction(txId, kioskId_, itemId, name, quantity, totalPrice,
                       paymentContext_.getCurrentMethod(), phone, TransactionStatus::SUCCESS);
}

// ─── refundTransaction Implementation ───────────────────────────────────────
inline bool Kiosk::refundTransaction(const Transaction& tx) {
    std::cout << "\n🔁 ═══════════════════════════════════════════════\n";
    std::cout << "🔁  REFUND INITIATED — Transaction: " << tx.transactionId << "\n";
    std::cout << "🔁 ═══════════════════════════════════════════════\n";

    if (tx.status != TransactionStatus::SUCCESS) {
        std::cout << "  ❌ Cannot refund a non-successful transaction.\n";
        return false;
    }

    // Step 1: Refund payment
    std::cout << "\n  💰 Step 1: Refunding payment of Rs." << tx.amount << "...\n";
    bool refundOk = paymentContext_.refund(tx.amount);
    if (!refundOk) {
        std::cout << "  ❌ Refund FAILED!\n";
        return false;
    }

    // Step 2: Restore stock
    std::cout << "\n  📦 Step 2: Restoring stock (" << tx.quantity << " units)...\n";
    inventory_->incrementStock(tx.itemId, tx.quantity);

    std::cout << "\n  ✅ REFUND COMPLETE for transaction " << tx.transactionId << "\n";
    return true;
}

#endif // KIOSK_H
