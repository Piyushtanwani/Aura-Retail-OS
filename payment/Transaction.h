/*
 * ============================================================================
 *  AURA RETAIL OS — Transaction Record
 * ============================================================================
 *
 *  Represents a single transaction in the system. Each purchase or refund
 *  creates a Transaction object that is stored in the CentralRegistry.
 *
 * ============================================================================
 */

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>

enum class TransactionStatus {
    SUCCESS,
    FAILED,
    REFUNDED
};

struct Transaction {
    std::string transactionId;
    std::string kioskId;
    std::string itemId;
    std::string itemName;
    int quantity;
    double amount;
    std::string paymentMethod;
    std::string customerPhone;
    TransactionStatus status;
    std::time_t timestamp;

    Transaction(const std::string& txId,
                const std::string& kId,
                const std::string& iId,
                const std::string& iName,
                int qty,
                double amt,
                const std::string& payMethod,
                const std::string& phone,
                TransactionStatus st)
        : transactionId(txId), kioskId(kId), itemId(iId), itemName(iName),
          quantity(qty), amount(amt), paymentMethod(payMethod), 
          customerPhone(phone), status(st),
          timestamp(std::time(nullptr)) {}

    std::string getStatusString() const {
        switch (status) {
            case TransactionStatus::SUCCESS:  return "✅ SUCCESS";
            case TransactionStatus::FAILED:   return "❌ FAILED";
            case TransactionStatus::REFUNDED: return "🔁 REFUNDED";
            default: return "UNKNOWN";
        }
    }

    std::string getTimestampString() const {
        std::tm* lt = std::localtime(&timestamp);
        std::ostringstream oss;
        oss << std::put_time(lt, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    void display() const {
        std::cout << "  ┌─ Transaction [" << transactionId << "]\n"
                  << "  │  Kiosk:   " << kioskId << "\n"
                  << "  │  Item:    " << itemName << " (" << itemId << ")\n"
                  << "  │  Units:   " << quantity << "\n"
                  << "  │  Amount:  ₹" << amount << "\n"
                  << "  │  Payment: " << paymentMethod << "\n"
                  << "  │  Status:  " << getStatusString() << "\n"
                  << "  │  Time:    " << getTimestampString() << "\n"
                  << "  └──────────────────────────────\n";
    }
};

#endif // TRANSACTION_H
