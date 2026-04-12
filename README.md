# 🌟 Aura Retail OS

> **A Smart-City Retail Kiosk Simulation — Built with C++ & 7 Classic Design Patterns**

Aura Retail OS is a modular, extensible C++ simulation of an intelligent retail kiosk network deployed across a smart city. It models food stalls, pharmacy dispensaries, and emergency supply points — all connected through a central registry — and demonstrates how Object-Oriented Design Patterns solve real-world engineering problems elegantly.

---

## 🚀 Quick Start

### Prerequisites

| Tool | Version |
|------|---------|
| C++ Compiler | GCC / MSVC / Clang (C++17 or later) |
| Make / CMake | Any modern version |

### Build & Run


# Compile (GCC example)
```bash
g++ -std=c++17 main.cpp inventory/RealInventory.cpp inventory/SecureInventory.cpp persistence/PersistenceManager.cpp registry/CentralRegistry.cpp -o aura_retail_os
```
# Run
```bash
./aura_retail_os        # Linux / macOS
```
```bash
aura_retail_os.exe      # Windows
```

> **Interactive Mode:** On startup the simulation asks for kiosk IDs, locations, product names, prices, stock quantities, refrigeration temperature, and payment method. Press **Enter** at any prompt to accept the shown default value.

---

## ✨ Feature Overview

| # | Feature | Description |
|---|---------|-------------|
| 1 | **Multi-type Kiosks** | Food, Pharmacy, and Emergency kiosks created via Factory |
| 2 | **Composite Inventory** | Products and Bundles nestable in any depth |
| 3 | **Secure Proxy Layer** | Every inventory access is logged and validated |
| 4 | **Runtime Decorators** | Attach/detach Refrigeration, Network |
| 5 | **Pluggable Payments** | Swap UPI, Card, Wallet strategies at runtime |
| 6 | **Atomic Transactions** | Stock and payment roll back automatically on failure |
| 7 | **Global Registry** | Singleton tracks all kiosks and transactions network-wide |
| 8 | **JSON Persistence** | Inventory and transaction history survive process restarts |

---

## 🗂 Project Structure

```
OOP Project/
│
├── main.cpp                        # Entry point + interactive prompts
│
├── core/                           # Kiosk abstractions & decorators
│   ├── Kiosk.h                     # Abstract base — purchaseItem logic
│   ├── FoodKiosk.h                 # Concrete: food & beverages
│   ├── PharmacyKiosk.h             # Concrete: medicines & cold-chain
│   ├── EmergencyKiosk.h            # Concrete: survival supplies
│   ├── KioskFactory.h              # Factory Pattern — creates kiosks by type
│   ├── KioskDecorator.h            # Decorator base — wraps any Kiosk
│   ├── RefrigerationModule.h       # Decorator: temperature monitoring
│   └── NetworkModule.h             # Decorator: connectivity & logging
│
├── inventory/                      # Inventory domain (Composite + Proxy)
│   ├── InventoryComponent.h        # Abstract component (Composite root)
│   ├── InventoryInterface.h        # Interface for Proxy pattern
│   ├── Product.h                   # Leaf node — single SKU
│   ├── Bundle.h                    # Composite node — group of components
│   ├── RealInventory.h / .cpp      # Real subject holding the data map
│   └── SecureInventory.h / .cpp    # Proxy — logging, validation, security
│
├── payment/                        # Payment domain (Strategy + Adapter)
│   ├── PaymentProcessor.h          # Abstract strategy interface
│   ├── PaymentContext.h            # Context that holds the active strategy
│   ├── UPIAdapter.h                # Adapter: LegacyUPIGateway → PaymentProcessor
│   ├── CardAdapter.h               # Adapter: CardPaymentSystem → PaymentProcessor
│   ├── WalletAdapter.h             # Adapter: WalletService → PaymentProcessor
│   └── Transaction.h               # Transaction record struct
│
├── hardware/                       # Dispenser domain (Adapter)
│   ├── Dispenser.h                 # Abstract dispenser interface
│   ├── StandardDispenser.h         # Default dispenser
│   ├── RefrigeratedDispenser.h     # Cold-chain dispenser
│   └── DispenserAdapter.h          # Adapter: ExternalDispenserAPI → Dispenser
│
├── registry/                       # Singleton global registry
│   ├── CentralRegistry.h
│   └── CentralRegistry.cpp         # Meyer's Singleton + transaction ledger
│
├── persistence/                    # JSON file I/O
│   ├── PersistenceManager.h
│   └── PersistenceManager.cpp      # Save/load inventory & transactions
│
├── inventory.json                  # Persisted inventory data
└── transactions.json               # Persisted transaction log
```

---

## 🎭 Design Patterns Used

| Pattern | Location | Role |
|---------|----------|------|
| **Factory** | `KioskFactory` | Creates correct kiosk subclass from a string key |
| **Decorator** | `KioskDecorator`, `RefrigerationModule`, `NetworkModule` | Adds hardware/software modules without subclassing |
| **Proxy** | `SecureInventory` → `RealInventory` | Guards inventory with logging and validation |
| **Strategy** | `PaymentContext` + `PaymentProcessor` | Swaps payment method at runtime without if/else chains |
| **Adapter** | `UPIAdapter`, `CardAdapter`, `WalletAdapter`, `DispenserAdapter` | Bridges incompatible external APIs |
| **Composite** | `Product` (leaf) + `Bundle` (composite) | Uniform treatment of single SKUs and bundled kits |
| **Singleton** | `CentralRegistry` | One global transaction + kiosk ledger across all modules |

---

## 🔄 Transaction Lifecycle

Every purchase flows through four atomic steps. All steps roll back on failure:

```
1. Stock Check     → SecureInventory (Proxy) validates availability
2. Payment         → PaymentContext delegates to active Strategy via Adapter
3. Stock Decrement → RealInventory atomically reduces count
4. Dispensing      → Dispenser (Standard / Refrigerated / Adapter) releases item
         └─ Any failure → full rollback (stock restored, payment refunded)
```

---

## 🖥 Interactive Prompts (Dynamic Inputs)

The simulation collects the following inputs at runtime:

| Section | Prompt | Default |
|---------|--------|---------|
| Section 1 | Food Kiosk ID | `FD-S1` |
| Section 1 | Food Kiosk Location | `Central Metro Station` |
| Section 1 | Pharmacy Kiosk ID | `PH-H1` |
| Section 1 | Pharmacy Kiosk Location | `City Hospital` |
| Section 2 | Product names & prices (all products) | Pre-filled defaults |
| Section 2 | Stock quantities (water, sandwich, insulin, kit) | Pre-filled defaults |
| Section 2 | First Aid Kit bundle ID, name, discount % | `B-001`, `Basic First Aid Kit`, `10%` |
| Section 3 | Refrigeration temperature (°C) | `4.0` |
| Section 4 | Payment method (UPI / Card / Wallet) | `1` (UPI) |
| Section 4 | UPI VPA or card number | `user@aura-upi` |
| Section 5 | Jammed product name & price | `Jammed Snack`, `10.0` |

---

## 🔒 Constraint Scenarios Demonstrated

| Scenario | What Happens |
|----------|-------------|
| **Hardware Mismatch** | Buying a refrigerated item from a non-refrigerated kiosk fails before payment |
| **Out-of-Stock** | Proxy blocks purchase when stock hits 0; payment never triggered |
| **Dispenser Jam** | Hardware failure after payment → full rollback (stock + payment restored) |

---

## 💾 Persistence

Two JSON files store state between runs:

- **`food_inventory.json`** — Saved at shutdown; loaded at startup so stock persists.
- **`pharmacy_inventory.json`** — Saved at shutdown; loaded at startup so stock persists.
- **`emergency_inventory.json`** — Saved at shutdown; loaded at startup so stock persists.
- **`transactions.json`** — Appended after every successful transaction; provides an audit trail.

---

## 🧩 Extending the System

| Goal | How |
|------|-----|
| Add a new kiosk type | Create a subclass of `Kiosk`, register string key in `KioskFactory` |
| Add a new payment method | Implement `PaymentProcessor`, give it to `PaymentContext::setStrategy()` |
| Add a new hardware module | Subclass `KioskDecorator`, override `purchaseItem()` and `describeModules()` |
| Add a new dispenser | Implement `Dispenser` interface |

---

## 📄 License

This project was built as an academic OOP demonstration. Feel free to use it for learning purposes.
