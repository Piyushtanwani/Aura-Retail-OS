/*
 * ============================================================================
 *  AURA RETAIL OS — Payment Processor Interface (Strategy Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Strategy Pattern (Strategy Interface)
 *
 *  This abstract class defines the interface for all payment strategies.
 *  Each concrete strategy (UPIAdapter, CardAdapter, WalletAdapter) implements
 *  this interface. The PaymentContext holds a pointer to a PaymentProcessor
 *  and can switch strategies at runtime.
 *
 * ============================================================================
 */

#ifndef PAYMENT_PROCESSOR_H
#define PAYMENT_PROCESSOR_H

#include <string>

class PaymentProcessor {
public:
    virtual ~PaymentProcessor() = default;

    // Process a payment of the given amount. Returns true on success.
    virtual bool processPayment(double amount) = 0;

    // Refund a previously processed payment. Returns true on success.
    virtual bool processRefund(double amount) = 0;

    // Get the name of this payment method (for logging/display)
    virtual std::string getMethodName() const = 0;
};

#endif // PAYMENT_PROCESSOR_H
