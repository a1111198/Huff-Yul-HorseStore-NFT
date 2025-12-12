# HorseStore: Low-Level EVM Implementation Study

> **Solidity → Yul → Huff** — Demonstrating bytecode-level optimization and EVM internals mastery through identical contract implementations.

[![Solidity](https://img.shields.io/badge/Solidity-0.8.20-363636?logo=solidity)](https://soliditylang.org/)
[![Huff](https://img.shields.io/badge/Huff-Assembly-orange)](https://huff.sh/)
[![Foundry](https://img.shields.io/badge/Foundry-Testing-red)](https://getfoundry.sh/)
[![Security](https://img.shields.io/badge/Security-Audited_Patterns-green)](.)

---

## Why This Exists

Same contract logic implemented at three abstraction levels:

- **Solidity** — High-level baseline
- **Yul** — Inline assembly optimization  
- **Huff** — Raw opcodes (738-line ERC721)

Useful for understanding how Solidity compiles to bytecode, identifying gas optimization opportunities, and recognizing patterns during security audits.

---

## Technical Highlights

| Metric | Solidity | Yul | Huff |
|--------|----------|-----|------|
| **V1 Runtime** | ~165 bytes | ~140 bytes | ~90 bytes |
| **V1 Deploy** | ~250 bytes | ~200 bytes | ~100 bytes |
| **Gas (write)** | Baseline | -12% | -25% |
| **Abstraction** | High | Medium | None |

### What's Implemented

**V1 — Minimal Storage Contract**
- `updateHorseNumber(uint256)` / `readNumberOfHorses()`
- Function dispatcher patterns
- Storage slot management

**V2 — Full ERC721 NFT (738 lines of Huff)**
- Complete ERC721Enumerable implementation
- Custom game mechanics (`feedHorse`, `isHappyHorse`)
- 24+ function selectors with manual dispatcher
- `safeTransferFrom` with `onERC721Received` callback handling
- Storage mapping calculations via keccak256


## Architecture

### Compilation Pipeline

```mermaid
flowchart LR
    subgraph Source["Source Code"]
        SOL["Solidity"]
        YUL["Yul Assembly"]
        HUFF["Huff Opcodes"]
    end

    subgraph Compilers
        SOLC["solc"]
        HUFFC["huffc"]
    end

    subgraph Output["Bytecode"]
        DEPLOY["Deploy Code"]
        RUNTIME["Runtime Code"]
    end

    SOL --> SOLC
    YUL --> SOLC
    HUFF --> HUFFC
    SOLC --> DEPLOY
    HUFFC --> DEPLOY
    DEPLOY --> RUNTIME
```

### V2 Contract Architecture

```mermaid
flowchart TB
    subgraph Entry["Transaction Entry"]
        CALLDATA["calldata"]
    end

    subgraph Dispatcher["Function Dispatcher (MAIN macro)"]
        SELECTOR["Extract selector calldataload(0) >> 0xe0"]
        JUMPTABLE["Jump Table 24+ selectors"]
    end

    subgraph HorseLogic["HorseStore Logic"]
        MINT["mintHorse() _safeMint(caller, totalSupply++)"]
        FEED["feedHorse(id) timestamp → mapping"]
        HAPPY["isHappyHorse(id) check 24hr window"]
    end

    subgraph ERC721["ERC721 Core"]
        TRANSFER["transferFrom"]
        SAFE["safeTransferFrom + onERC721Received"]
        APPROVE["approve / setApprovalForAll"]
        QUERY["balanceOf / ownerOf"]
    end

    subgraph Storage["Storage Layout"]
        S0[("0: totalSupply")]
        S1[("1: ownerOf[id]")]
        S2[("2: fedTimestamp[id]")]
        S3[("3: balanceOf[addr]")]
        S4[("4: approvals")]
    end

    CALLDATA --> SELECTOR
    SELECTOR --> JUMPTABLE
    JUMPTABLE --> HorseLogic
    JUMPTABLE --> ERC721
    
    MINT --> S0 & S1 & S3
    FEED --> S2
    HAPPY --> S2
    TRANSFER --> S1 & S3 & S4
```

### Huff Function Dispatcher (Low-Level)

```mermaid
flowchart TD
    START["MAIN()"] --> LOAD["0x00 calldataload\n0xe0 shr"]
    LOAD --> DUP["dup1"]
    
    DUP --> CMP1{"== totalSupply?"}
    CMP1 -->|Yes| TOTAL["GET_TOTAL_SUPPLY()"]
    CMP1 -->|No| CMP2{"== mintHorse?"}
    
    CMP2 -->|Yes| MINT["mintHorse_Macro()"]
    CMP2 -->|No| CMP3{"== feedHorse?"}
    
    CMP3 -->|Yes| FEED["feedHorse_Macro()"]
    CMP3 -->|No| CMP4{"== ERC721 method?"}
    
    CMP4 -->|Yes| ERC["ERC721 Handlers"]
    CMP4 -->|No| REVERT["revert(0,0)"]

    TOTAL --> RET["return(0x00, 0x20)"]
    MINT --> STOP["stop"]
    FEED --> STOP
```

---

## Implementation Deep-Dive

### Storage Slot Calculation (Mappings in Huff)

Solidity mappings use `keccak256(key . slot)` for storage location. In Huff, this is implemented manually:

```plaintext
#define macro horseIdToFedTimeStamp_Macro() = takes(0) returns(0) {
    0x04 calldataload              // [horseId]
    [Horse_Store_Location]         // [slot, horseId]
    0x20 mstore                    // [horseId] (slot at mem[0x20])
    0x00 mstore                    // [] (horseId at mem[0x00])
    0x40 0x00 sha3                 // [keccak256(horseId . slot)]
    sload                          // [fedTimestamp]
    0x00 mstore
    0x20 0x00 return
}
```

### ERC721 Safe Transfer Pattern

```mermaid
sequenceDiagram
    participant User
    participant HorseStore
    participant Recipient

    User->>HorseStore: safeTransferFrom(from, to, id)
    HorseStore->>HorseStore: TRANSFER_TAKE_FROM()
    HorseStore->>HorseStore: TRANSFER_GIVE_TO()
    HorseStore->>HorseStore: emit Transfer()
    
    HorseStore->>HorseStore: EXTCODESIZE(to)
    
    alt Contract Recipient
        HorseStore->>Recipient: onERC721Received(operator, from, id, "")
        Recipient-->>HorseStore: 0x150b7a02
        alt Valid Selector
            HorseStore-->>User: Success
        else Invalid
            HorseStore-->>User: UNSAFE_RECIPIENT revert
        end
    else EOA
        HorseStore-->>User: Success
    end
```

---

## Security Analysis

### Identified Issues

| Severity | Issue | Location | Status |
|----------|-------|----------|--------|
| 🔴 **Medium** | Feeding non-existent horses | `feedHorse()` | Documented |
| 🟡 **Low** | Unlimited public minting | `mintHorse()` | By Design |
| 🟢 **Info** | No tokenURI implementation | `tokenURI()` | Returns empty |

### Feed Function Vulnerability

```solidity
// Missing existence check allows feeding unminted horses
function feedHorse(uint256 horseId) external {
    horseIdToFedTimeStamp[horseId] = block.timestamp;
    // ❌ No: require(_exists(horseId), "NOT_MINTED");
}
```

**Impact**: Storage pollution, misleading `isHappyHorse()` returns for non-existent IDs.

### Security Invariants Maintained

1. `ownerOf(id) != address(0)` ⟹ token exists
2. `balanceOf` sum equals minted count
3. Approvals cleared on transfer
4. CEI pattern in all transfers (reentrancy safe)

---

## Quick Start

### Prerequisites

```bash
curl -L https://foundry.paradigm.xyz | bash && foundryup
curl -L https://huff.sh | bash && huffup
```

### Build & Test

```bash
git clone https://github.com/a1111198/Huff-Yul-HorseStore-NFT.git
cd Huff-Yul-HorseStore-NFT
forge install
forge build
forge test -vvv
```

### Gas Report

```bash
forge test --gas-report
```

---

## Project Structure

```
src/
├── HorseStoreV1/
│   ├── horseStore.sol       # Solidity baseline
│   ├── yulHorseStore.sol    # Yul optimization
│   └── horseStore.huff      # Huff implementation
└── HorseStoreV2/
    ├── horseStore.sol       # Full ERC721 (Solidity)
    ├── HorseStore.huff      # Full ERC721 (Huff) - 738 lines
    └── IhorseStore.sol      # Interface

breakdown/
└── Solc-OpCodes.c++         # Annotated bytecode analysis

test/
├── v1/                      # V1 test suite
└── v2/                      # V2 test suite (shared base)
```

---

## Tech Stack

| Component | Technology |
|-----------|------------|
| Smart Contracts | Solidity 0.8.20, Yul, Huff |
| Testing | Foundry (Forge) |
| Huff Libraries | huffmate (ERC721, CommonErrors) |
| Reference | OpenZeppelin Contracts |
| EVM Target | Cancun |

---

## Related Work

This project complements my other blockchain security research:

- **[Formal Verification with Halmos/Certora](https://github.com/a1111198/Formal-verification-and-symbolic-Execution)** — Symbolic execution for math functions

---

## License

GPL-3.0-only

---

## References

- [Huff Language Docs](https://docs.huff.sh/)
- [EVM Opcodes](https://www.evm.codes/)
- [EIP-721 Specification](https://eips.ethereum.org/EIPS/eip-721)
- [Solidity Yul Documentation](https://docs.soliditylang.org/en/latest/yul.html)
