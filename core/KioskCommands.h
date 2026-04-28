/*
 * ============================================================================
 *  AURA RETAIL OS — Kiosk Commands (Command Pattern)
 * ============================================================================
 */

#ifndef KIOSK_COMMANDS_H
#define KIOSK_COMMANDS_H

#include "Command.h"
#include "Kiosk.h"
#include "../payment/Transaction.h"
#include <memory>

// ─── PurchaseItemCommand ────────────────────────────────────────────────────
class PurchaseItemCommand : public Command {
private:
    Kiosk* kiosk_;
    std::string itemId_;
    int quantity_;
    std::string phone_;
    Transaction result_; // Store result to retrieve later

public:
    PurchaseItemCommand(Kiosk* kiosk, const std::string& itemId, int quantity, const std::string& phone)
        : kiosk_(kiosk), itemId_(itemId), quantity_(quantity), phone_(phone),
          result_("", "", "", "", 0, 0.0, "", "", TransactionStatus::FAILED) {}

    void execute() override {
        std::cout << "📜 [Command] Executing PurchaseItemCommand for item: " << itemId_ << "\n";
        result_ = kiosk_->purchaseItem(itemId_, quantity_, phone_);
    }

    std::string getDescription() const override {
        return "PurchaseItemCommand [Item: " + itemId_ + ", Qty: " + std::to_string(quantity_) + "]";
    }

    Transaction getResult() const { return result_; }
};

// ─── RefundCommand ─────────────────────────────────────────────────────────
class RefundCommand : public Command {
private:
    Kiosk* kiosk_;
    Transaction tx_;
    bool success_;

public:
    RefundCommand(Kiosk* kiosk, const Transaction& tx)
        : kiosk_(kiosk), tx_(tx), success_(false) {}

    void execute() override {
        std::cout << "📜 [Command] Executing RefundCommand for TX: " << tx_.transactionId << "\n";
        success_ = kiosk_->refundTransaction(tx_);
    }

    std::string getDescription() const override {
        return "RefundCommand [TX: " + tx_.transactionId + ", Amount: " + std::to_string(tx_.amount) + "]";
    }

    bool wasSuccessful() const { return success_; }
};

// ─── RestockCommand ────────────────────────────────────────────────────────
class RestockCommand : public Command {
private:
    Kiosk* kiosk_;
    std::string itemId_;
    int quantity_;

public:
    RestockCommand(Kiosk* kiosk, const std::string& itemId, int quantity)
        : kiosk_(kiosk), itemId_(itemId), quantity_(quantity) {}

    void execute() override {
        std::cout << "📜 [Command] Executing RestockCommand for item: " << itemId_ << "\n";
        kiosk_->restockInventory(itemId_, quantity_);
    }

    std::string getDescription() const override {
        return "RestockCommand [Item: " + itemId_ + ", Qty: " + std::to_string(quantity_) + "]";
    }
};

#endif // KIOSK_COMMANDS_H
