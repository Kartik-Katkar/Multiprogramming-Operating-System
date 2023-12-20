# Multiprogramming Operating System (MOS) Project

## Overview
This project implements a simple Multiprogramming Operating System (MOS) in its first version. The system assumes certain conditions and provides basic functionalities for job execution, input/output handling, and program loading in a non-multiprogramming environment.

## Assumptions
- Jobs are entered without errors in the input file.
- There is no physical separation between jobs in memory.
- Job outputs are separated in the output file by two blank lines.
- The program is loaded in memory starting at location 00.
- No multiprogramming is supported; the system loads and runs one program at a time.
- System Interrupt (SI) is used for service requests.

## Notation
- **M:** Memory
- **IR:** Instruction Register (4 bytes)
  - IR[1,2]: Bytes 1, 2 of IR/Operation Code
  - IR[3,4]: Bytes 3, 4 of IR/Operand Address
- **IC:** Instruction Counter Register (2 bytes)
- **R:** General Purpose Register (4 bytes)
- **C:** Toggle (1 byte)

## MOS (Master Mode)
The MOS in master mode handles service requests based on the SI interrupt:

- **SI = 1 (Initialization):** Read operation
- **SI = 2 (Initialization):** Write operation
- **SI = 3 (Initialization):** Terminate operation

### READ
```assembly
IR[4] ← 0
Read the next (data) card from the input file into memory locations IR[3,4] through IR[3,4] + 9.
If M[IR[3,4]] equals $END, abort (out-of-data).
```

### WRITE
```assembly
IR[4] ← 0
Write one block (10 words of memory) from memory locations IR[3,4] through IR[3,4] + 9 to the output file.
```

### TERMINATE
```assembly
Write 2 blank lines in the output file.
```

### MOS/LOAD
#### LOAD
```assembly
m ← 0
While not end-of-file (e-o-f)
  Read the next (program or control) card from the input file into a buffer.
  Control card: $AMJ, end-while
    $DTA, MOS/STARTEXECUTION
    $END, end-while
  Program Card:
    If m = 100, abort (memory exceeded).
    Store the buffer in memory locations m through m + 9.
    m ← m + 10
STOP
```

#### MOS/STARTEXECUTION
```assembly
IC ← 00
Execute user program.
```

## EXECUTEUSERPROGRAM (SLAVE MODE)
```assembly
Loop
  IR ← M[IC]
  IC ← IC + 1
  Examine IR[1,2]
    LR: R ← M[IR[3,4]]
    SR: R → M[IR[3,4]]
    CR: Compare R and M[IR[3,4]]
      If equal, C ← T, else C ← F
    BT: If C = T, then IC ← IR[3,4]
    GD: SI = 1
    PD: SI = 2
    H: SI = 3
  End-Examine
End-Loop
```

## Usage
This version of MOS provides basic functionalities for program execution, input/output, and job termination. Users are advised to adhere to the specified assumptions for proper system functioning.

Feel free to explore and modify the code to suit your specific needs.