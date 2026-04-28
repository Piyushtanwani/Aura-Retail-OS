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
Manages products, bundles, and stock levels. Uses **Composite Pattern** for uniform treatment of individual products and bundled kits. This layer implements **dynamic stock calculation**, where bundle availability is derived in real-time from its leaf nodes. **Proxy Pattern** secures all inventory access with logging and validation.

### Payment System
Processes transactions using different payment methods (UPI, Card, Wallet). Uses **Strategy Pattern** for runtime method switching and **Adapter Pattern** to bridge external gateway APIs. Supports automated receipt generation and refund processing.

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
4. A **PurchaseItemCommand** is created and executed
5. Payment is processed through selected strategy
6. Inventory is updated (stock decremented, including bundle components)
7. Hardware dispenses item
8. SMS receipt sent to customer (including Transaction ID)
9. Transaction recorded in CentralRegistry
10. Data saved to JSON files

**On failure at any step:** Full rollback — stock is restored and payment is refunded automatically.

---

## Persistence

- Inventory data is loaded from JSON files on startup and saved on shutdown
- Transaction history is appended after every successful transaction
- Three separate inventory files: `food_inventory.json`, `pharmacy_inventory.json`, `emergency_inventory.json`
- One shared transaction log: `transactions.json`

---

## Path B: Modular Hardware Platform

Aura Retail OS is specifically architected as a long-term hardware platform, satisfying the requirements of **Path B**. The design focuses on extreme modularity, hardware extensibility, and secure inventory management.

### 1. Hardware Abstraction & Extensibility
The system decouples high-level kiosk logic from specific hardware implementations using the **Adapter Pattern** and **Interface Abstraction**.
- **Proven via:** `StandardDispenser` and `RefrigeratedDispenser`.
- **Designed for:** The architecture allows seamless integration of `SpiralDispenser`, `RoboticArmDispenser`, or `ConveyorDispenser` by simply implementing the `Dispenser` interface. No changes to the `Kiosk` core are required.
- **Runtime Swapping:** The `Kiosk::setDispenser()` method allows the system to swap physical dispensing hardware while the application is running, satisfying the "Hardware Replacement" constraint.

### 2. Transactional Command Pattern (Implemented)
The system fully implements the **Command Pattern** to encapsulate all primary kiosk operations.
- **Commands:** `PurchaseItemCommand`, `RefundCommand`, and `RestockCommand` inherit from a base `Command` interface.
- **Decoupling:** This design decouples the request invocation (User UI/Admin Panel) from the execution logic (Kiosk core), enabling advanced features like operation logging, audit trails, and transactional integrity.
- **Execution:** Each command supports a standardized `execute()` method and provides descriptive metadata for system logging.

### 3. Secure Inventory Access (RBAC Ready)
The **Proxy Pattern** implemented in `SecureInventory` serves as the primary security gateway.
- **Current State:** Implements access logging, stock validation, and state integrity.
- **Path B Extension:** The architecture is designed to intercept every call to include **Role-Based Access Control (RBAC)**, allowing the system to restrict specific inventory drawers or bundles based on the operator's authorization level.

### 4. Hierarchical Inventory (Dynamic Logic)
The **Composite Pattern** allows for complex, recursive inventory structures.
- **Modular Bundles:** Individual products can be grouped into bundles, and bundles can contain other bundles recursively.
- **Dynamic Calculation (Implemented):** The system implements advanced inventory logic where **Bundle stock is never independent**. It is calculated in real-time by finding the minimum availability of all required components. Decrementing a bundle's stock automatically decrements its underlying leaf products.

### 5. Pluggable Payment Integration
Using a combination of **Strategy** and **Adapter** patterns, the system treats payment providers as modular components.
- **Flexibility:** Adding a new provider (e.g., a city-specific digital wallet) only requires creating a new `PaymentProcessor` adapter.
- **No Core Changes:** The `Kiosk` remains agnostic of the payment implementation details, ensuring the platform stays compatible with future financial technologies.

---

*Aura Retail OS — OOP Project, April 2026*
