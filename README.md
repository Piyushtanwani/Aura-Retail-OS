# Aura Retail OS

> A Smart-City Retail Kiosk Simulation built with C++ and 7 Classic Design Patterns

Aura Retail OS is a modular C++ simulation of an intelligent retail kiosk network for a smart city. It models Food Stalls, Pharmacy Dispensaries, and Emergency Supply Points — all connected through a Central Registry — demonstrating real-world Object-Oriented Design Patterns.

---

## Objective

To design and implement a fully functional retail kiosk simulation using Object-Oriented Programming principles in C++. The system showcases seven classic design patterns working together to create a scalable, maintainable, and secure point-of-sale platform with real-time SMS authentication, atomic transactions, and persistent storage.

---

## Features

- **Modular Multi-type Kiosks** — Food, Pharmacy, and Emergency kiosks created via a scalable **Factory**
- **Hierarchical Inventory** — **Recursive Composite structure** for products and bundles, nestable to any depth
- **Secure Proxy Gateway** — Intercepts inventory access for **logging, validation, and future RBAC**
- **Pluggable Hardware Decorators** — Dynamically attach/detach modules (Refrigeration, Network) at runtime
- **Agnostic Payment Integration** — Swap between UPI, Card, and Wallet adapters without core modification
- **Command-Driven Operations** — Purchase, Refund, and Restock actions encapsulated as **Command objects** for auditability
- **Dynamic Bundle Availability** — Bundle stock is **dynamically calculated** from component quantities in real-time
- **Atomic Transaction Integrity** — Fail-safe state restoration (rollback) for both stock and payments
- **Global System Registry** — Singleton-based ledger tracking kiosks and city-wide transactions
- **Persistent State Management** — JSON-based persistence ensures system state survives power cycles
- **External API Bridge** — Real-time Twilio SMS integration for OTP security, receipts, and **refund notifications**

---

## Security Features

- **Admin OTP Authentication** — 6-digit OTP sent via SMS to the registered admin phone before granting access
- **Customer Payment OTP** — 2FA mobile verification before processing any payment
- **Card Security Validation** — Full card number, expiry date, and CVV verification for card payments
- **Secure Inventory Proxy** — All inventory operations pass through a validated, logged proxy layer
- **Payment Gateway Verification** — UPI ID format checks, card detail validation, and wallet app verification
- **Credential Protection** — Twilio credentials are kept out of the repository; environment-level configuration required

---

## Design Patterns Used

| Pattern | Location | Role |
|---------|----------|------|
| **Factory** | `KioskFactory` | Creates correct kiosk subclass from a string key |
| **Decorator** | `KioskDecorator`, `RefrigerationModule`, `NetworkModule` | Adds hardware/software modules without subclassing |
| **Proxy** | `SecureInventory` → `RealInventory` | Guards inventory with logging and validation |
| **Strategy** | `PaymentContext` + `PaymentProcessor` | Swaps payment method at runtime |
| **Adapter** | `UPIAdapter`, `CardAdapter`, `WalletAdapter`, `DispenserAdapter` | Bridges incompatible external APIs |
| **Composite** | `Product` (leaf) + `Bundle` (composite) | Uniform treatment of single SKUs and bundled kits |
| **Singleton** | `CentralRegistry` | One global transaction + kiosk ledger across all modules |
| **Command** | `Command`, `PurchaseItemCommand`, `RestockCommand` | Encapsulates requests as objects for logging and decoupling |

---

## Project Structure

```
OOP Project/
├── main.cpp                        # Entry point + interactive CLI
│
├── core/                           # Kiosk abstractions & decorators
│   ├── Kiosk.h                     # Abstract base
│   ├── FoodKiosk.h                 # Food & beverages
│   ├── PharmacyKiosk.h             # Medicines & cold-chain
│   ├── EmergencyKiosk.h            # Survival supplies
│   ├── KioskFactory.h              # Factory Pattern
│   ├── KioskDecorator.h            # Decorator base
│   ├── RefrigerationModule.h       # Temperature monitoring
│   └── NetworkModule.h             # Connectivity & logging
│
├── inventory/                      # Composite + Proxy
│   ├── InventoryComponent.h        # Abstract component
│   ├── InventoryInterface.h        # Proxy interface
│   ├── Product.h                   # Leaf node (single SKU)
│   ├── Bundle.h                    # Composite node (group)
│   ├── RealInventory.h / .cpp      # Real subject
│   └── SecureInventory.h / .cpp    # Proxy with logging
│
├── payment/                        # Strategy + Adapter
│   ├── PaymentProcessor.h          # Strategy interface
│   ├── PaymentContext.h            # Active strategy holder
│   ├── UPIAdapter.h                # UPI gateway adapter
│   ├── CardAdapter.h               # Card payment adapter
│   ├── WalletAdapter.h             # Wallet service adapter
│   └── Transaction.h               # Transaction record
│
├── hardware/                       # Dispenser Adapter
│   ├── Dispenser.h                 # Abstract dispenser
│   ├── StandardDispenser.h         # Default dispenser
│   ├── RefrigeratedDispenser.h     # Cold-chain dispenser
│   └── DispenserAdapter.h          # External API adapter
│
├── registry/                       # Singleton
│   ├── CentralRegistry.h
│   └── CentralRegistry.cpp
│
├── persistence/                    # JSON file I/O
│   ├── PersistenceManager.h
│   └── PersistenceManager.cpp
│
├── food_inventory.json             # Persisted food stock
├── pharmacy_inventory.json         # Persisted pharmacy stock
├── emergency_inventory.json        # Persisted emergency stock
└── transactions.json               # Transaction audit log
```

---

## Architecture

For a detailed breakdown of the modules, data flow, persistence layers, and a high-level system diagram, please refer to the **[Architecture Documentation](ARCHITECTURE.md)**.

---

## How to Run

### Prerequisites

| Tool | Version |
|------|---------|
| C++ Compiler | GCC / MSVC / Clang (C++17 or later) |
| Make / CMake | Any modern version |

### Compile

```bash
g++ -std=c++17 main.cpp inventory/RealInventory.cpp inventory/SecureInventory.cpp persistence/PersistenceManager.cpp registry/CentralRegistry.cpp -o aura_retail_os
```

### Run

**Linux / macOS:**
```bash
./aura_retail_os
```

**Windows:**
```bash
./aura_retail_os.exe
```

### Twilio Setup (for SMS features)

1. Open `main.cpp` and locate the Twilio Configuration section (~line 70)
2. Replace placeholders with your actual Twilio credentials:
   - `TWILIO_ACCOUNT_SID`
   - `TWILIO_AUTH_TOKEN`
   - `TWILIO_FROM_NUMBER`
   - `ADMIN_PHONE`

> **Note:** Never commit real credentials to a public repository.

---

## Simulation Steps

1. **Launch the program** — Run the compiled executable
2. **Select Role** — Choose Customer or Admin mode
3. **Admin Flow:**
    - Receive OTP on registered phone
    - Enter OTP to authenticate
    - View kiosks, restock items, manage bundles (add/remove items), or process transaction refunds
4. **Customer Flow:**
   - Select a kiosk (Food / Pharmacy / Emergency)
   - Browse the product catalogue
   - Select item and enter quantity
   - Verify mobile number via OTP
   - Choose payment method (UPI / Card / Wallet)
   - Enter payment details (UPI ID, Card Number + Expiry + CVV, or Wallet App)
   - Transaction completes with automatic stock update
   - SMS receipt sent to verified number
5. **Exit** — Inventory and transactions are saved to JSON files automatically

---

## Screenshots

### Welcome Screen & Role Selection
![Main Menu](screenshots/Main%20Menu.png)

### Product Catalogue
![Product Selection](screenshots/Product%20Selection.png)

### Payment Success & Order Summary
![Payment Success](screenshots/Payment%20Success.png)

### Admin Panel with OTP Authentication
![Admin Panel](screenshots/Admin%20Panel.png)

### Session Summary
![Session Summary](screenshots/Session%20Summary.png)

---

## System Capabilities

- **Out-of-Stock Handling** — Proxy blocks purchase when stock is 0; payment is never triggered
- **Automatic Rollback** — Any failure (hardware jam, payment error) triggers full rollback of stock and payment
- **Data Persistence** — Inventory and transactions survive restarts via JSON file storage
- **Hardware Validation** — Refrigerated items require a refrigerated kiosk; mismatch is caught before payment
- **Dispenser Failure Recovery** — Hardware failure after payment restores stock and refunds automatically
- **Real-time SMS Notifications** — OTP codes and order receipts delivered via Twilio

---

## Path B: Modular Hardware Platform (Project Focus)

This project was developed with a specific focus on **Path B (Modular Hardware Platform)**, emphasizing hardware extensibility, secure modular integration, and long-term architectural stability.

### Key Path B Achievements:
- **Hardware Abstraction:** Decoupled dispensing logic from physical hardware via the `Dispenser` interface, proven with `Standard` and `Refrigerated` implementations.
- **Dynamic Extensibility:** Demonstrated through **Decorator modules** that can be attached to any kiosk type at runtime without modifying the base class.
- **Gateway Interoperability:** Implemented a unified payment interface that converts incompatible external APIs (UPI, Card, Wallet) into a common transaction protocol.
- **Smart Inventory Logic:** Engineered a **Composite-based inventory** system where **Bundle stock is derived dynamically** from underlying components, preventing ghost sales.
- **Command Architecture:** Fully implemented the **Command Pattern** for all primary operations (Purchase, Restock, Refund), enabling a clean separation between the UI and system core.
- **Operational Security:** Established a **Proxy-based security layer** that acts as a single point of enforcement for access logging and state validation.

---

## GitHub Repository

[https://github.com/Piyushtanwani/Aura-Retail-OS](https://github.com/Piyushtanwani/Aura-Retail-OS)

---

## Team Members

| Name | Student ID | Role |
|------|-----------|------|
| Afif Momin | 202512063 | Kiosk Core & Overall Integration — Designed Kiosk abstract base class, implemented FoodKiosk, PharmacyKiosk, EmergencyKiosk, managed Inventory-Payment-Hardware interaction, contributed to system architecture |
| Deep Soni | 202512089 | Inventory System (Composite + Proxy) — Implemented InventoryComponent, Product, Bundle classes, hierarchical inventory with Composite Pattern, SecureInventory & RealInventory with Proxy Pattern, stock & bundle logic |
| Ismail Mansuri | 202512075 | Payment System (Strategy + Adapter) — Implemented PaymentProcessor interface & multiple methods, Strategy Pattern for dynamic selection, UPIAdapter/CardAdapter/WalletAdapter via Adapter Pattern, transaction processing & validation |
| Piyush Tanwani | 202512021 | Hardware Layer & Supporting Modules — Implemented Dispenser, StandardDispenser, RefrigeratedDispenser, DispenserAdapter for Hardware Abstraction, Decorator modules (RefrigerationModule, NetworkModule), CentralRegistry via Singleton |

---

## Conclusion

Aura Retail OS demonstrates how seven classic OOP design patterns can be combined to build a production-grade retail kiosk simulation. The system handles real-world scenarios including secure authentication, atomic transactions with rollback, pluggable payment gateways, and persistent storage — all within a clean, modular C++ architecture.
