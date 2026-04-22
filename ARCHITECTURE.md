# 🏛 Aura Retail OS — Architecture

> A layered, pattern-driven C++ architecture for a smart-city kiosk network.

---

## Table of Contents

1. [High-Level Overview](#1-high-level-overview)
2. [Layer Breakdown](#2-layer-breakdown)
3. [Design Patterns — Detailed](#3-design-patterns--detailed)
4. [Class Hierarchy Diagrams](#4-class-hierarchy-diagrams)
5. [Data Flow: Purchase Transaction](#5-data-flow-purchase-transaction)
6. [Module Dependency Graph](#6-module-dependency-graph)
7. [Persistence Architecture](#7-persistence-architecture)
8. [Design Decisions & Trade-offs](#8-design-decisions--trade-offs)

---

## 1. High-Level Overview

```
╔══════════════════════════════════════════════════════════════════════════╗
║                         AURA RETAIL OS                                  ║
╠══════════════════════════════════════════════════════════════════════════╣
║  ┌──────────────────────────────────────────────────────────────────┐   ║
║  │                    main.cpp  (Orchestrator)                       │   ║
║  │         Interactive CLI ─ drives all 7 design patterns           │   ║
║  └────────────────────────────┬─────────────────────────────────────┘   ║
║                               │                                          ║
║     ┌─────────────────────────┼──────────────────────────────┐          ║
║     ▼                         ▼                              ▼           ║
║  ┌──────────┐         ┌───────────────┐           ┌──────────────────┐  ║
║  │  core/   │         │  inventory/   │           │   payment/       │  ║
║  │  Kiosks  │◄───────►│  Proxy+       │           │   Strategy +     │  ║
║  │  Factory │         │  Composite    │           │   Adapter        │  ║
║  │  Decorat.│         └───────────────┘           └──────────────────┘  ║
║  └──────────┘                                                            ║
║       │                                                                  ║
║  ┌────▼─────────────────────────────────────────────────────────────┐   ║
║  │  registry/CentralRegistry  (Singleton)                           │   ║
║  │  hardware/Dispenser*  (Adapter)                                  │   ║
║  │  persistence/PersistenceManager  (JSON I/O)                      │   ║
║  │  Twilio SMS API (via curl) ─ OTP & Receipt Delivery              │   ║
║  └──────────────────────────────────────────────────────────────────┘   ║
╚══════════════════════════════════════════════════════════════════════════╝
```

---

## 2. Layer Breakdown

| Layer | Module | Responsibility |
|-------|--------|----------------|
| **Presentation** | `main.cpp` | CLI prompts, simulation orchestration |
| **Kiosk Domain** | `core/` | Kiosk types, factory, decorator chain |
| **Inventory Domain** | `inventory/` | Product/Bundle tree, proxy security layer |
| **Payment Domain** | `payment/` | Strategy context, external gateway adapters |
| **Hardware Domain** | `hardware/` | Dispenser abstraction + external hardware adapter |
| **Registry** | `registry/` | Global singleton ledger for kiosks & transactions |
| **Persistence** | `persistence/` | JSON serialise/deserialise for inventory & transactions |
| **Security/External**| `main.cpp` | Twilio SMS API integration (OTP & Receipts) |

---

## 3. Design Patterns — Detailed

### 3.1 Factory Pattern — `KioskFactory`

**Problem:** Client code should not depend on concrete kiosk classes.  
**Solution:** `KioskFactory::createKiosk(type, id, location)` maps a string key to the correct subclass.

```
KioskFactory
    └─ createKiosk("food")      → FoodKiosk
    └─ createKiosk("pharmacy")  → PharmacyKiosk
    └─ createKiosk("emergency") → EmergencyKiosk
```

**Extensibility:** Adding a new kiosk type requires:
1. Creating a new subclass of `Kiosk`
2. Adding one `else if` branch inside `KioskFactory`

---

### 3.2 Decorator Pattern — `KioskDecorator`

**Problem:** Hardware modules (refrigeration, network) need to be attached optionally and in any combination without an explosion of subclasses.  
**Solution:** `KioskDecorator` wraps any `Kiosk` and delegates all calls. Concrete decorators intercept calls to add behaviour.

```
Kiosk (abstract)
    ├── FoodKiosk
    ├── PharmacyKiosk
    ├── EmergencyKiosk
    └── KioskDecorator  (wraps a Kiosk)
            ├── RefrigerationModule  → checks temperature before purchase
            └── NetworkModule        → logs transactions over network
```

**Stacking example** (pharmacy kiosk in main):
```
NetworkModule
    └─ wraps RefrigerationModule
                └─ wraps PharmacyKiosk   ← actual kiosk
```

---

### 3.3 Proxy Pattern — `SecureInventory`

**Problem:** `RealInventory` is a sensitive data store; all access must be logged and validated.  
**Solution:** `SecureInventory` implements the same `InventoryInterface` as `RealInventory`. All clients talk to `SecureInventory` and never know `RealInventory` exists.

```
InventoryInterface
    ├── RealInventory          (subject — holds the data map)
    └── SecureInventory        (proxy — adds logging + validation)
            └─ has-a RealInventory
```

**Proxy responsibilities:**
- `logAccess()` — timestamps every read/write
- Validates negative quantities on `addItem()`
- Checks item existence before stock operations

---

### 3.4 Strategy Pattern — `PaymentContext` / `PaymentProcessor`

**Problem:** Payment method must be switchable at runtime (UPI → Card → Wallet).  
**Solution:** `PaymentContext` holds a `unique_ptr<PaymentProcessor>` and delegates `pay()` / `refund()` to it.

```
PaymentProcessor (abstract)
    ├── UPIAdapter
    ├── CardAdapter
    └── WalletAdapter

PaymentContext
    └─ has-a PaymentProcessor  ← swapped by setStrategy()
```

**Runtime swap example:**
```cpp
kiosk.setPaymentStrategy(make_unique<UPIAdapter>("user@sbi"));
// ... transaction 1 ...
kiosk.setPaymentStrategy(make_unique<CardAdapter>("4111222233334444"));
// ... transaction 2 ...
```

---

### 3.5 Adapter Pattern — Three Contexts

Used in **two domains**:

#### A. Payment Adapters
External payment gateways have incompatible APIs. Adapters bridge them to `PaymentProcessor`.

```
LegacyUPIGateway    → UPIAdapter    → PaymentProcessor
CardPaymentSystem   → CardAdapter   → PaymentProcessor
WalletService       → WalletAdapter → PaymentProcessor
```

#### B. Hardware Dispenser Adapter
Third-party dispenser hardware has its own API. `DispenserAdapter` bridges it.

```
ExternalDispenserAPI → DispenserAdapter → Dispenser
```

---

### 3.6 Composite Pattern — `InventoryComponent`

**Problem:** Inventory can contain individual products or bundles that contain other products/bundles. Clients must treat both uniformly.  
**Solution:** `InventoryComponent` is the abstract component with `getPrice()`, `display()`, `add()`, and `remove()`.

```
InventoryComponent (abstract)
    ├── Product   (Leaf)     — single SKU, no children
    └── Bundle    (Composite) — holds a list of InventoryComponent children
            └── can contain Products or other Bundles (recursive)
```

**Bundle pricing:** `Σ child.getPrice() × (1 − discount%/100)`

---

### 3.7 Singleton Pattern — `CentralRegistry`

**Problem:** Kiosk and transaction data must be globally accessible without passing objects everywhere.  
**Solution:** Meyer's Singleton — static local variable, thread-safe from C++11 onwards.

```cpp
CentralRegistry& CentralRegistry::getInstance() {
    static CentralRegistry instance; // created once, destroyed at shutdown
    return instance;
}
```

Copy and move constructors/assignment operators are `= delete` to prevent any duplication.

---

## 4. Class Hierarchy Diagrams

### 4.1 Kiosk Hierarchy

```
Kiosk  (abstract)
│   + purchaseItem(itemId) : Transaction
│   + refundTransaction(tx) : bool
│   + addProduct(item, qty)
│   + setPaymentStrategy(strategy)
│   + setDispenser(dispenser)
│   + displayInfo()
│   + describeModules() : string
│
├── FoodKiosk
├── PharmacyKiosk
├── EmergencyKiosk
└── KioskDecorator  (has-a Kiosk wrappee_)
        ├── RefrigerationModule
        └── NetworkModule
```

### 4.2 Inventory Hierarchy

```
InventoryInterface  (abstract)
│   + addItem / isInStock / getStock
│   + decrementStock / incrementStock
│   + getItem / displayCatalogue / getAllItemIds
│
├── RealInventory          (uses unordered_map<id, {component, qty}>)
└── SecureInventory        (proxy — wraps RealInventory)

InventoryComponent  (abstract — Composite root)
│   + getId / getName / getPrice
│   + requiresRefrigeration()
│   + add / remove / getChildIds (composite hooks)
│   + display / toJson
│
├── Product   (Leaf)
└── Bundle    (Composite — vector<shared_ptr<InventoryComponent>>)
```

### 4.3 Payment Hierarchy

```
PaymentProcessor  (abstract)
│   + processPayment(amount) : bool
│   + processRefund(amount)  : bool
│   + getMethodName()        : string
│
├── UPIAdapter      (wraps LegacyUPIGateway)
├── CardAdapter     (wraps CardPaymentSystem)
└── WalletAdapter   (wraps WalletService)

PaymentContext
    └─ strategy_ : unique_ptr<PaymentProcessor>
       + setStrategy / pay / refund / getCurrentMethod
```

### 4.4 Hardware Hierarchy

```
Dispenser  (abstract)
│   + dispense(itemId, itemName) : bool
│   + getType()                  : string
│
├── StandardDispenser
├── RefrigeratedDispenser
└── DispenserAdapter     (wraps ExternalDispenserAPI)
```

---

## 5. Data Flow: Purchase Transaction

```
User / main.cpp
    │
    ▼
activeKiosk.purchaseItem("P-102")
    │
    ├─ [NetworkModule.purchaseItem]          ← Decorator: logs entry
    │       │
    │       └─ [RefrigerationModule.purchaseItem]  ← Decorator: checks temp
    │               │
    │               └─ [PharmacyKiosk / Kiosk.purchaseItem]
    │                       │
    │                       ├─ Step 1: SecureInventory.isInStock("P-102")
    │                       │          └─ logAccess() → RealInventory.isInStock()
    │                       │
    │                       ├─ Step 1.5: Check requiresRefrigeration()
    │                       │            vs. dispenser type
    │                       │
    │                       ├─ Step 2: Payment Authentication (2FA)
    │                       │          └─ sendSMS(customerPhone, OTP)
    │                       │          └─ Customer Input → Verify OTP
    │                       │
    │                       ├─ Step 3: PaymentContext.pay(850.0)
    │                       │          └─ UPIAdapter.processPayment()
    │                       │              └─ LegacyUPIGateway.initiateUPIPayment()
    │                       │
    │                       ├─ Step 4: SecureInventory.decrementStock("P-102")
    │                       │
    │                       ├─ Step 5: RefrigeratedDispenser.dispense("P-102", "Insulin Pen")
    │                       │
    │                       └─ Step 6: Automated SMS Receipt
    │                                  └─ sendSMS(customerPhone, Summary)
    │
    └─ Returns Transaction { txId, kioskId, itemId, amount, status }
            │
            └─ CentralRegistry.recordTransaction(tx)
                    └─ PersistenceManager.saveTransactionsToFile(...)
```

**Rollback chain (any step fails):**
```
Step 4 fails → incrementStock("P-102", 1)  →  refund(850.0)
Step 3 fails → refund(850.0)
Step 2 fails → return FAILED (no stock touched)
Step 1 fails → return FAILED (skip payment entirely)
```

---

## 6. Module Dependency Graph

```
main.cpp
  ├── core/KioskFactory.h
  │       ├── core/FoodKiosk.h
  │       ├── core/PharmacyKiosk.h
  │       └── core/EmergencyKiosk.h
  │               └── core/Kiosk.h
  │                       ├── inventory/SecureInventory.h
  │                       │       └── inventory/RealInventory.h
  │                       │               └── inventory/InventoryInterface.h
  │                       │                       └── inventory/InventoryComponent.h
  │                       ├── payment/PaymentContext.h
  │                       │       └── payment/PaymentProcessor.h
  │                       ├── payment/Transaction.h
  │                       └── hardware/Dispenser.h
  │
  ├── core/KioskDecorator.h → core/Kiosk.h
  │       ├── core/RefrigerationModule.h
  │       └── core/NetworkModule.h
  │
  ├── inventory/Bundle.h    → inventory/InventoryComponent.h
  ├── inventory/Product.h   → inventory/InventoryComponent.h
  │
  ├── payment/UPIAdapter.h  → payment/PaymentProcessor.h
  ├── payment/CardAdapter.h → payment/PaymentProcessor.h
  ├── payment/WalletAdapter.h → payment/PaymentProcessor.h
  │
  ├── hardware/RefrigeratedDispenser.h → hardware/Dispenser.h
  │
  ├── registry/CentralRegistry.h → payment/Transaction.h
  └── persistence/PersistenceManager.h
```

---

## 7. Persistence Architecture

```
On Startup:
  PersistenceManager::loadInventoryFromFile(inventory, "inventory.json")
      └─ Reads JSON → hydrates SecureInventory with Products/Bundles + stock

On Each Successful Transaction:
  CentralRegistry::recordTransaction(tx)
      └─ PersistenceManager::saveTransactionsToFile(allTxs, "transactions.json")
             └─ Overwrites file with complete updated ledger

On Shutdown:
  PersistenceManager::saveInventoryToFile(inventory, "inventory.json")
      └─ Serialises all items + current stock levels to JSON
```

**JSON format — inventory.json:**
```json
{
  "inventory": [
    { "id": "P-201", "name": "Mineral Water", "price": 20.0,
      "requiresRefrigeration": false, "stock": 49 }
  ]
}
```

**JSON format — transactions.json:**
```json
{
  "transactions": [
    { "id": "PH-H1-TX-1", "kiosk": "PH-H1", "item": "P-102",
      "name": "Insulin Pen", "amount": 850.0,
      "method": "UPI (user88@oksbi)", "status": "SUCCESS",
      "timestamp": "2026-04-09 13:05:00" }
  ]
}
```

---

## 8. Design Decisions & Trade-offs

| Decision | Rationale | Trade-off |
|----------|-----------|-----------|
| **Header-only implementations** for `Kiosk.h` (inline methods) | Simplifies build for single-TU demos | Would need `.cpp` separation in larger projects |
| **`unique_ptr` for decorators** | Clear single ownership, prevents dangling references | Kiosk cannot be shared once wrapped |
| **`shared_ptr` for inventory items** | Products/Bundles shared across kiosks and bundles | Slight overhead vs raw pointers |
| **Meyer's Singleton** | Thread-safe from C++11, no manual locking needed | Static initialization order fiasco if misused cross-TU |
| **No external JSON library** | Zero dependencies, portable anywhere | Manual serialisation is verbose and error-prone |
| **`InventoryInterface` abstraction** | Allows `SecureInventory` proxy without touching `RealInventory` | One extra indirection layer |
| **`PaymentProcessor` as pure abstract class** | New adapters need no change to context or kiosk | Requires adapter boilerplate for each gateway |
| **Twilio integration via `curl`** | High portability and zero library dependencies | Requires `curl` to be installed on the host OS |
| **Mandatory SMS Receipts** | Ensures transaction traceability for all customers | Increases API usage vs optional opt-in |

---

*Generated for the Aura Retail OS OOP project — April 2026*
