PUSH1 0x80
PUSH1 0x40
MSTORE
// Free memory Pointer 
CALLVALUE  
DUP1
ISZERO  
//[1,msg.value]
PUSH1 0x0e
//[14,1,msg.value]
JUMPI
//If Zero 
PUSH0   
DUP1     
//[0,0,msg.value]
REVERT   
// and revert with size Zero 
JUMPDEST  //IF not Zero we would be here. since we didn't pass payable constructor so we can not send value while contract construction
POP
PUSH1 0xa5
DUP1

PUSH2 0x001b
PUSH0
CODECOPY
// make sense copy code from this to that 
PUSH0
RETURN

INVALID

// Runtime Code starts
PUSH1 0x80
PUSH1 0x40
MSTORE
// normal Memory Pointer Thing
CALLVALUE
DUP1
ISZERO
PUSH1 0x0e
JUMPI
PUSH0
DUP1
REVERT
// check again for eth value sent since no Function is payable so you can't send Ether 
JUMPDEST
POP
//[]
PUSH1 0x04
CALLDATASIZE
LT
PUSH1 0x30   // this Destination or Programe counter is for Deployed bytes code (like only Run time)
JUMPI
// checking If some Funcion selecctor is called or not 
PUSH0
CALLDATALOAD
PUSH1 0xe0
SHR
DUP1
// Function Dispatcher is here 
PUSH4 0xcdfead2e
EQ
PUSH1 0x34
JUMPI
// check For the Update Number Dispatcher 
DUP1
PUSH4 0xe026c017
EQ
PUSH1 0x45
//for Read Number of Horses 
JUMPI
// Revert Jusmp dest in case of callDataSize is less than 4
JUMPDEST
PUSH0
DUP1
REVERT
// Dest for Updating Number 
JUMPDEST
PUSH1 0x43
PUSH1 0x3f
CALLDATASIZE
PUSH1 0x04
PUSH1 0x59
JUMP
// for sstsore 
JUMPDEST
PUSH0
SSTORE
JUMP
JUMPDEST
STOP
// 
//read 
JUMPDEST
PUSH0
SLOAD
PUSH1 0x40
MLOAD
SWAP1
DUP2
MSTORE
PUSH1 0x20
ADD
PUSH1 0x40
MLOAD
DUP1
SWAP2
SUB
SWAP1
RETURN

JUMPDEST
PUSH0
PUSH1 0x20
DUP3
DUP5
SUB
SLT
ISZERO
PUSH1 0x68
JUMPI
PUSH0
DUP1
REVERT
// cchecking have enogh data for SStore
JUMPDEST
POP
CALLDATALOAD
SWAP2
SWAP1
POP
JUMP
//Runtime till here 
INVALID
LOG2
PUSH5 0x6970667358
INVALID
SLT
KECCAK256
CODESIZE
DUP1
INVALID
OR
INVALID
ADDMOD
SDIV
CALLDATASIZE
BLOBHASH
SELFDESTRUCT
CREATE2
SWAP13
INVALID
DUP13
INVALID
SGT
INVALID
INVALID
RETURNDATASIZE
INVALID
INVALID
INVALID
INVALID
INVALID
BLOBHASH
SWAP11
CREATE
PUSH16 0x846a4c1e64736f6c63430008140033