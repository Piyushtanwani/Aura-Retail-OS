/*
 * ============================================================================
 *  AURA RETAIL OS — AI Recommendation Module (Decorator Pattern)
 * ============================================================================
 *
 *  DESIGN PATTERN: Decorator Pattern (Concrete Decorator)
 *
 *  Analyzes purchases to suggest add-on items (e.g., "People who bought X
 *  also bought Y"). This is purely informational post-purchase.
 *
 * ============================================================================
 */

#ifndef AI_RECOMMENDATION_MODULE_H
#define AI_RECOMMENDATION_MODULE_H

#include "KioskDecorator.h"
#include <iostream>

class AIRecommendationModule : public KioskDecorator {
public:
    explicit AIRecommendationModule(std::unique_ptr<Kiosk> wrappee)
        : KioskDecorator(std::move(wrappee)) {
        std::cout << "  🎨 [Decorator] Attached AI Recommendation Engine to Kiosk " << kioskId_ << "\n";
    }

    Transaction purchaseItem(const std::string& itemId) override {
        Transaction tx = KioskDecorator::purchaseItem(itemId);

        if (tx.status == TransactionStatus::SUCCESS) {
            std::cout << "  🤖 [AI Module] Analyzing purchase of \"" << tx.itemName << "\"...\n";
            std::cout << "  🤖 [AI Module] Suggestion: Customers often buy \"Water\" with this item.\n";
        }
        return tx;
    }

    std::string describeModules() const override {
        std::string base = KioskDecorator::describeModules();
        std::string me = "[AI Smart Engine]";
        return (base == "None") ? me : base + " + " + me;
    }
};

#endif // AI_RECOMMENDATION_MODULE_H
