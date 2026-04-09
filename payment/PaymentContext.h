/*
 * ============================================================================
 *  AURA RETAIL OS — Payment Context (Strategy Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Strategy Pattern (Context)
 *
 *  PaymentContext is the context class in the Strategy pattern. It holds
 *  a reference to the current payment strategy (PaymentProcessor) and
 *  delegates payment/refund operations to it. The strategy can be swapped
 *  at runtime using setStrategy(), enabling dynamic payment method switching.
 *
 *  Usage:
 *    PaymentContext ctx;
 *    ctx.setStrategy(std::make_unique<UPIAdapter>("user@upi"));
 *    ctx.pay(500.0);
 *
 *    // Switch to card at runtime
 *    ctx.setStrategy(std::make_unique<CardAdapter>("4111111111111111"));
 *    ctx.pay(300.0);
 *
 * ============================================================================
 */

#ifndef PAYMENT_CONTEXT_H
#define PAYMENT_CONTEXT_H

#include "PaymentProcessor.h"
#include <memory>
#include <iostream>

class PaymentContext {
private:
    std::unique_ptr<PaymentProcessor> strategy_;  // current payment strategy

public:
    PaymentContext() : strategy_(nullptr) {}

    explicit PaymentContext(std::unique_ptr<PaymentProcessor> strategy)
        : strategy_(std::move(strategy)) {}

    // --- Switch payment strategy at runtime ---
    void setStrategy(std::unique_ptr<PaymentProcessor> strategy) {
        strategy_ = std::move(strategy);
        std::cout << "  🎯 [PaymentContext] Strategy switched to: "
                  << strategy_->getMethodName() << "\n";
    }

    // --- Execute payment using current strategy ---
    bool pay(double amount) {
        if (!strategy_) {
            std::cout << "  ❌ [PaymentContext] No payment strategy set!\n";
            return false;
        }
        std::cout << "  🎯 [PaymentContext] Processing payment of ₹" << amount
                  << " via " << strategy_->getMethodName() << "\n";
        return strategy_->processPayment(amount);
    }

    // --- Execute refund using current strategy ---
    bool refund(double amount) {
        if (!strategy_) {
            std::cout << "  ❌ [PaymentContext] No payment strategy set!\n";
            return false;
        }
        std::cout << "  🎯 [PaymentContext] Processing refund of ₹" << amount
                  << " via " << strategy_->getMethodName() << "\n";
        return strategy_->processRefund(amount);
    }

    // --- Get current method name ---
    std::string getCurrentMethod() const {
        return strategy_ ? strategy_->getMethodName() : "None";
    }
};

#endif // PAYMENT_CONTEXT_H
