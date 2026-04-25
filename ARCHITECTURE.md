# Aura Retail OS — Architecture

> A layered, pattern-driven C++ architecture for a smart-city kiosk network.

---

## Table of Contents

1. [High-Level Overview](#high-level-overview)
2. [Layer Breakdown](#layer-breakdown)
3. [Modules](#modules)
4. [Data Flow](#data-flow)
5. [Persistence](#persistence)

---

## High-Level Overview

```
╔═══════════════════════════════════════════════════════════╗
║                      AURA RETAIL OS                       ║
╠═══════════════════════════════════════════════════════════╣
║                      [ main.cpp ]                         ║
║                     (User Interface)                      ║
║                           │                               ║
║                           ▼                               ║
║                      [ core/ ]                            ║
║                 (Kiosk Management)                        ║
║                           │                               ║
║       ┌───────────────────┼───────────────────┐           ║
║       ▼                   ▼                   ▼           ║
║  [ inventory/ ]     [ payment/ ]       [ hardware/ ]      ║
║  (Stock Mgmt)       (Transactions)     (Dispensing)       ║
║       │                   │                   │           ║
║       └───────────────┬───┴───────────────┬───┘           ║
║                       ▼                   ▼               ║
║            [ registry/ ]          [ persistence/ ]        ║
║              (Ledger)              (JSON Storage)         ║
╚═══════════════════════════════════════════════════════════╝
```

---

## Layer Breakdown

| Layer | Module | Responsibility |
|-------|--------|----------------|
| **Presentation** | `main.cpp` | CLI prompts, simulation orchestration |
| **Kiosk Domain** | `core/` | Kiosk types, factory, decorator chain |
| **Inventory Domain** | `inventory/` | Product/Bundle tree, proxy security layer |
| **Payment Domain** | `payment/` | Strategy context, external gateway adapters |
| **Hardware Domain** | `hardware/` | Dispenser abstraction + external hardware adapter |
| **Registry** | `registry/` | Global singleton ledger for kiosks & transactions |
| **Persistence** | `persistence/` | JSON serialise/deserialise for inventory & transactions |
| **Security/External** | `main.cpp` | Twilio SMS API integration (OTP & Receipts) |

---

## Modules

### Core System
Handles kiosk creation, type selection, and user interaction. Uses Factory Pattern to create kiosks and Decorator Pattern to attach optional hardware modules (Refrigeration, Network) at runtime.

### Inventory System
Manages products, bundles, and stock levels. Uses Composite Pattern for uniform treatment of individual products and bundled kits. Proxy Pattern secures all inventory access with logging and validation.

### Payment System
Processes transactions using different payment methods (UPI, Card, Wallet). Uses Strategy Pattern for runtime method switching and Adapter Pattern to bridge external gateway APIs.

### Hardware System
Handles item dispensing through Standard and Refrigerated dispensers. Uses Adapter Pattern to integrate external dispenser hardware.

### Central Registry
Maintains a global record of all registered kiosks and completed transactions using Singleton Pattern.

### Persistence Layer
Stores inventory and transaction data in JSON files. Loads data on startup and saves on every transaction and shutdown.

---

## Data Flow

1. User selects kiosk and product
2. System checks inventory via SecureInventory (Proxy)
3. Payment is authenticated via SMS OTP (2FA)
4. Payment is processed through selected strategy
5. Inventory is updated (stock decremented)
6. Hardware dispenses item
7. SMS receipt sent to customer
8. Transaction recorded in CentralRegistry
9. Data saved to JSON files

**On failure at any step:** Full rollback — stock is restored and payment is refunded automatically.

---

## Persistence

- Inventory data is loaded from JSON files on startup and saved on shutdown
- Transaction history is appended after every successful transaction
- Three separate inventory files: `food_inventory.json`, `pharmacy_inventory.json`, `emergency_inventory.json`
- One shared transaction log: `transactions.json`

---

*Aura Retail OS — OOP Project, April 2026*
