#include <stdio.h>
#include <stdint.h> // For int
#include <string.h>

#define MEMORY_SIZE 2048
#define INSTRUCTION_MEMORY_SIZE 1024
#define INSTRUCTION_LENGTH 32 // Maximum length of an instruction

int jumpFlag = 999;

typedef struct
{
    const int R0;
    int GPRS[31]; // 32-bit word array of size 2048
    int PC;
} Registers;

// GPRS are registers from 1 to 32 but the array starts from 0
int readGPR(Registers *regs, int regNumber)
{
    if (regNumber >= 1 && regNumber < 32)
    {
        return regs->GPRS[regNumber - 1];
    }
    else
    {
        printf("Error: Invalid GPR number\n");
        return -1; // Return an error value
    }
}

int writeGPR(Registers *regs, int regNumber, int value)
{
    if (regNumber >= 1 && regNumber < 32)
    {
        regs->GPRS[regNumber - 1] = value;
        return 0;
    }
    else
    {
        printf("Error: Invalid GPR number\n");
        return -1;
    }
}

typedef struct
{
    int words[MEMORY_SIZE]; // 32-bit word array of size 2048
    int instructionCount;
} Memory;

// Initialize each word to 0
void initializeMemory(Memory *mem)
{
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        mem->words[i] = 0;
    }
    mem->instructionCount = 0;
}

void initializeInstructions(Memory *mem)
{
    for (int i = 0; i < 1024; i++)
    {
        mem->words[i] = 0;
    }
    mem->instructionCount = 0;
}
void initializeData(Memory *mem)
{
    for (int i = 1024; i < 2048; i++)
    {
        mem->words[i] = 0;
    }
}

// Function to read from memory
// takes as input &mem and the address of the word you want to read
int readMemory(Memory *mem, int address)
{
    if (address >= 0 && address < MEMORY_SIZE)
    {
        return mem->words[address];
    }
    else
    {
        printf("Error: Invalid memory address\n");
        return -1; // Return an error value
    }
}

int readInstruction(Memory *mem, int address)
{
    if (address >= 0 && address < mem->instructionCount && address < 1024)
    {
        return mem->words[address];
    }
    else
    {
        printf("Error: Invalid memory address\n");
        return -1; // Return an error value
    }
}

int readData(Memory *mem, int address)
{
    if (address >= 1024 && address < 2048)
    {
        return mem->words[address];
    }
    else
    {
        printf("Error: Invalid memory address\n");
        return -1; // Return an error value
    }
}

// Function to write to memory
int writeMemory(Memory *mem, int address, int value)
{
    if (address >= 0 && address < MEMORY_SIZE)
    {
        mem->words[address] = value;
        return 0;
    }
    else
    {
        printf("Error: Invalid memory address\n");
        return -1;
    }
}

int writeInstruction(Memory *mem, int address, int value)
{
    if (address >= 0 && address < 1024)
    {
        mem->words[address] = value;
        return 0;
    }
    else
    {
        printf("Error: Invalid memory address\n");
        return -1;
    }
}

int writeData(Memory *mem, int address, int value)
{
    if (address >= 1024 && address < 2048)
    {
        mem->words[address] = value;
        return 0;
    }
    else
    {
        printf("Error: Invalid memory address\n");
        return -1;
    }
}

int addInstruction(Memory *mem, int instruction)
{

    if (writeInstruction(mem, mem->instructionCount, instruction) == 0)
    {
        // writting done right
        mem->instructionCount++;
        return 0;
    }
    else
    {

        printf("instruction memory full\n");
        return -1;
    }
}

void registerNumbers(char *instruction, int register_nums[], int max_numbers)
{
    int count = 0;
    char *ptr = instruction;

    while (*ptr != '\0' && count < max_numbers)
    {
        if (*ptr == '$') // Skip register names
        {
            ptr++;
            while (*ptr >= 'A' && *ptr <= 'Z')
                ptr++;
        }
        else if ((*ptr >= '0' && *ptr <= '9') || *ptr == '-') // Found a number
        {
            char *endptr;
            register_nums[count++] = strtol(ptr, &endptr, 10);
            ptr = endptr; // Move to the next character after the number
        }
        else // Move to the next character
        {
            ptr++;
        }
    }
}

// fetch decode excute memory write back

// FETCH TESTED
int fetch(Memory *mem, Registers *regs)
{

    int instruction = readInstruction(mem, regs->PC);
    if (instruction != -1)
    {
        // read instruction returns -1 when PC is >= instruction count
        // thus , this is a correct instrucion
        regs->PC++;
        return instruction;
    }
    else
    {

        // pc exceeded allowed instructions
        regs->PC++;
        printf("no more instructions to be fetched.\n");
        return -1; // no more instructions to be fetched
    }
}

// DECODE IS TESTED
char decodeHelper(int instruction)
{
    int opcodeHelper = 0;
    opcodeHelper = (instruction & 0b11110000000000000000000000000000) >> 28;
    if (opcodeHelper == 0b0000 || opcodeHelper == 0b0001 || opcodeHelper == 0b0010 || opcodeHelper == 0b0101 ||
        opcodeHelper == 0b1000 || opcodeHelper == 0b1001)
    { // case R
        return 'R';
    }
    else if (opcodeHelper == 7)
    { // case J
        return 'J';
    }
    else
    { // case I
        return 'I';
    }
}
// intiliazing the variables to keep

void *decode(Registers *regs, int instruction, int instructionData[10])
{

    int opcode = 0;
    int R1 = 0;
    int R2 = 0;
    int R3 = 0;
    int SHAMT = 0;
    int address = 0;
    int immediate = 0;
    int valueR1 = 0;
    int valueR2 = 0;
    int valueR3 = 0;

    switch (decodeHelper(instruction))
    {
    case 'R':
        // get opcode 4 bits
        opcode = (instruction & 0b11110000000000000000000000000000) >> 28;
        // get R1 5 bits

        R1 = (instruction & 0b00001111100000000000000000000000) >> 23;
        // get R2 5 bits

        R2 = (instruction & 0b00000000011111000000000000000000) >> 18;
        // get R3 5 bits

        R3 = (instruction & 0b00000000000000111110000000000000) >> 13;
        // get SHAMT 13 bits
        SHAMT = instruction & 0b00000000000000000001111111111111;
        // get Values R1,R2,R3
        valueR1 = readGPR(regs, R1);
        valueR2 = readGPR(regs, R2);
        valueR3 = readGPR(regs, R3);

        break;
    case 'J':
        // get opcode 4 bits
        opcode = (instruction & 0b11110000000000000000000000000000) >> 28;
        // get address 28 bits
        address = instruction & 0b00001111111111111111111111111111;
        break;

    case 'I':
        // get opcode 4 bits
        opcode = (instruction & 0b11110000000000000000000000000000) >> 28;
        // get R1 5 bits
        R1 = (instruction & 0b00001111100000000000000000000000) >> 23;
        // get R2 5 bits
        R2 = (instruction & 0b00000000011111000000000000000000) >> 18;
        // get Immediate 18 bits
        immediate = instruction & 0b00000000000000111111111111111111;
        int imm = 0;
        imm = (immediate & 0b00000000000000100000000000000000) >> 17;

        if (imm == 1)
        { // checking if sign bit = 1 (negative) to maintain it during sign extension
            immediate = immediate | 0b11111111111111000000000000000000;
        }
        // get value R1,R2
        valueR1 = readGPR(regs, R1);
        valueR2 = readGPR(regs, R2);

        break;
    }
    instructionData[0] = opcode;
    instructionData[1] = R1;
    instructionData[2] = R2;
    instructionData[3] = R3;
    instructionData[4] = SHAMT;
    instructionData[5] = address;
    instructionData[6] = immediate;
    instructionData[7] = valueR1;
    instructionData[8] = valueR2;
    instructionData[9] = valueR3;
}

int RinstructionExecute(Registers *regs, int *instructionData)
{
    int opcode = instructionData[0];
    int R1 = instructionData[1];
    int R2 = instructionData[2];
    int R3 = instructionData[3];
    int SHAMT = instructionData[4];
    int address = instructionData[5];
    int immediate = instructionData[6];
    int valueR1 = instructionData[7];
    int valueR2 = instructionData[8];
    int valueR3 = instructionData[9];
    int result;
    switch (opcode)
    {

    case 0b0000: // 0 // add
        result = valueR2 + valueR3;
        break;
    case 0b0001: // 1 // subtract
        result = valueR2 - valueR3;
        break;
    case 0b0010: // 2 // multiply
        result = valueR2 * valueR3;
        break;
    case 0b0101: // 5 // and
        result = valueR2 & valueR3;
        break;
    case 0b1000: // 8 // lsl
        result = valueR2 << SHAMT;
        break;
    case 0b1001: // 9 // lsr
        result = valueR2 >> SHAMT;
        break;
    }
    return result;
}

int JinstructionExecute(Registers *regs, int *instructionData)
{
    int opcode = instructionData[0];
    int R1 = instructionData[1];
    int R2 = instructionData[2];
    int R3 = instructionData[3];
    int SHAMT = instructionData[4];
    int address = instructionData[5];
    int immediate = instructionData[6];
    int valueR1 = instructionData[7];
    int valueR2 = instructionData[8];
    int valueR3 = instructionData[9];
    int result;

    jumpFlag = 0;
    regs->PC = (regs->PC & 0b11110000000000000000000000000000) | address;
    return regs->PC;
}

int IinstructionExecute(Registers *regs, Memory *mem, int *instructionData)
{
    int opcode = instructionData[0];
    int R1 = instructionData[1];
    int R2 = instructionData[2];
    int R3 = instructionData[3];
    int SHAMT = instructionData[4];
    int address = instructionData[5];
    int immediate = instructionData[6];
    int valueR1 = instructionData[7];
    int valueR2 = instructionData[8];
    int valueR3 = instructionData[9];
    int result = 0;
    int memAddress;
    switch (opcode)

    {
    case 0b0011: // 3 // move imm
        result = immediate;
        break;

    case 0b0100: // 4 //jump if equal
        if (valueR1 == valueR2)
        {
            regs->PC = regs->PC + 1 + immediate;
            jumpFlag = 0;
            return regs->PC;
        }
        break;
    case 0b0110: // 6 //exclusive or imm
        result = valueR2 ^ immediate;
        break;

    // Feehom moshkela 7ane3melha ezay ??
    case 0b1010: // 10 //move to reg
        memAddress = valueR2 + immediate;
        result = memAddress;
        break;
    case 0b1011: // 11 //move to mem
        memAddress = valueR2 + immediate;
        result = memAddress;
        break;
    }
    return result;
}

int execute(Registers *regs, Memory *mem, int *instructionData)
{

    int result;
    int opcode = instructionData[0];
    int destinationRegister = instructionData[1];
    if (opcode == 0b0000 || opcode == 0b0001 || opcode == 0b0010 || opcode == 0b0101 ||
        opcode == 0b1000 || opcode == 0b1001)
    { // case R
        result = RinstructionExecute(regs, instructionData);
    }
    else if (opcode == 7)
    { // case J
        result = JinstructionExecute(regs, instructionData);
    }
    else
    { // case I
        result = IinstructionExecute(regs, mem, instructionData);
    }
    return result;
}

int memoryAccess(Memory *mem, int memAddress, int RegisterValue, int *instructionData)
{
    int opcode = instructionData[0];
    int R1 = instructionData[1];
    int R2 = instructionData[2];
    int R3 = instructionData[3];
    int SHAMT = instructionData[4];
    int address = instructionData[5];
    int immediate = instructionData[6];
    int valueR1 = instructionData[7];
    int valueR2 = instructionData[8];
    int valueR3 = instructionData[9];
    int memoryValue;
    if (opcode == 0b1010)
    {
        return memoryValue = readMemory(mem, memAddress);
    }
    else if (opcode == 0b1011)
    {
        writeMemory(mem, memAddress, RegisterValue);
        return 0;
    }
    else
    {
        // when the operation is not a memory operation , i still pass the result to the memAccess using memAddress, so if i dont
        // want to access the memory , i want to pass the result to writeBack
        return memAddress;
    }
}

void writeBack(Registers *regs, int Value, int destinationRegister, int *instructionData)
{
    int opcode = instructionData[0];

    if (!(opcode == 7 || opcode == 11 || opcode == 4))
    {
        writeGPR(regs, destinationRegister, Value);
    }
}

void encodeInstructions(const char *filename, Memory *memory, int n)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file %s\n", filename);
        return;
    }

    char instruction[INSTRUCTION_LENGTH];
    int address = 0;

    while (fgets(instruction, INSTRUCTION_LENGTH, file) != NULL && address < n)
    {
        // Remove newline character
        instruction[strcspn(instruction, "\n")] = '\0';

        // Encode instruction into memory
        memory->words[address] = encodeInstruction(instruction);

        address++;
        memory->instructionCount++;
    }

    fclose(file);
    memory->instructionCount = address;
}

int encodeInstruction(const char *instruction)
{
    // Simplified encoding logic, you might need to adjust based on your instruction set architecture
    int opcode;
    int destinationRegister;
    int numberOfRegister1;
    int numberOfRegister2;
    int register_nums[4];
    int immediate;
    int shamt;
    int jumpAddress;

    if (strncmp(instruction, "ADD", 3) == 0)
    {
        // Encode ADD instruction
        printf("da5alt add\n");
        registerNumbers(instruction, register_nums, 3);
        // token is now the first word which is add
        // ADD $r1, $r2, $r15

        opcode = 0b00000000000000000000000000000000; // Example encoding for ADD
        // printf("destReg before shifting %d \n", register_nums[0]);
        destinationRegister = register_nums[0] << 23;
        // printf("destReg after shifting %d \n", destinationRegister);
        // printf("R1 before shifting %d \n", register_nums[1]);
        numberOfRegister1 = register_nums[1] << 18;
        // printf("R1 after shifting %d \n", numberOfRegister1);
        numberOfRegister2 = register_nums[2] << 13;
        // ADD R1 R2 R3

        int binaryInstruction = opcode + destinationRegister + numberOfRegister1 + numberOfRegister2;
        printf("7an add opcode:%d destRegister:%d R1:%d R2:%d  \n binaryInstruction: %d", opcode, destinationRegister, numberOfRegister1, numberOfRegister2, binaryInstruction);
        return binaryInstruction;
    }

    else if (strncmp(instruction, "SUB", 3) == 0)
    {
        // Subtract
        opcode = 0b00010000000000000000000000000000;
        registerNumbers(instruction, register_nums, 3);

        destinationRegister = register_nums[0] << 23;
        numberOfRegister1 = register_nums[1] << 18;
        numberOfRegister2 = register_nums[2] << 13;

        int binaryInstruction = opcode + destinationRegister + numberOfRegister1 + numberOfRegister2;
        printf("7an SUB opcode:%d destRegister:%d R1:%d R2:%d  \n binaryInstruction: %d", opcode, destinationRegister, numberOfRegister1, numberOfRegister2, binaryInstruction);
        return binaryInstruction;
    }
    else if (strncmp(instruction, "MUL", 3) == 0)
    {
        // Mul
        opcode = 0b00100000000000000000000000000000;
        registerNumbers(instruction, register_nums, 3);

        destinationRegister = register_nums[0] << 23;
        numberOfRegister1 = register_nums[1] << 18;
        numberOfRegister2 = register_nums[2] << 13;

        int binaryInstruction = opcode + destinationRegister + numberOfRegister1 + numberOfRegister2;
        printf("7an MUL opcode:%d destRegister:%d R1:%d R2:%d  \n binaryInstruction: %d", opcode, destinationRegister, numberOfRegister1, numberOfRegister2, binaryInstruction);
        return binaryInstruction;
    }
    else if (strncmp(instruction, "MOVI", 3) == 0)
    {
        // Move Immediate
        opcode = 0b00110000000000000000000000000000;
        registerNumbers(instruction, register_nums, 3);

        destinationRegister = register_nums[0] << 23;
        immediate = register_nums[1] << 18;

        int binaryInstruction = opcode + destinationRegister + immediate;
        printf("7an MOVI opcode:%d destRegister:%d immediate:%d   \n binaryInstruction: %d", opcode, destinationRegister, immediate, binaryInstruction);
        return binaryInstruction;
    }
    else if (strncmp(instruction, "JEQ", 3) == 0)
    {
        // Jump if Equal
        opcode = 0b01000000000000000000000000000000;
        registerNumbers(instruction, register_nums, 3);

        destinationRegister = register_nums[0] << 23;
        numberOfRegister1 = register_nums[1] << 18;
        immediate = register_nums[2] << 13;

        int binaryInstruction = opcode + destinationRegister + numberOfRegister1 + immediate;
        printf("7an JEQ opcode:%d destRegister:%d numberOfRegister1:%d immediate:%d   \n binaryInstruction: %d", opcode, destinationRegister, numberOfRegister1, immediate, binaryInstruction);
        return binaryInstruction;
    }
    else if (strncmp(instruction, "AND", 3) == 0)
    {
        // And
        opcode = 0b01010000000000000000000000000000;
        registerNumbers(instruction, register_nums, 3);

        destinationRegister = register_nums[0] << 23;
        numberOfRegister1 = register_nums[1] << 18;
        numberOfRegister2 = register_nums[2] << 13;

        int binaryInstruction = opcode + destinationRegister + numberOfRegister1 + numberOfRegister2;
        printf("7an AND opcode:%d destRegister:%d R1:%d R2:%d  \n binaryInstruction: %d", opcode, destinationRegister, numberOfRegister1, numberOfRegister2, binaryInstruction);
        return binaryInstruction;
    }
    else if (strncmp(instruction, "XORI", 3) == 0)
    {
        // Exclusive Or Immediate
        opcode = 0b01100000000000000000000000000000;
        registerNumbers(instruction, register_nums, 3);

        destinationRegister = register_nums[0] << 23;
        numberOfRegister1 = register_nums[1] << 18;
        immediate = register_nums[2] << 13;

        int binaryInstruction = opcode + destinationRegister + numberOfRegister1 + immediate;
        printf("7an XORI opcode:%d destRegister:%d immediate:%d   \n binaryInstruction: %d", opcode, destinationRegister, immediate, binaryInstruction);
        return binaryInstruction;
    }
    else if (strncmp(instruction, "JMP", 3) == 0)
    {
        // Jump
        opcode = 0b01110000000000000000000000000000;
        registerNumbers(instruction, register_nums, 3);

        jumpAddress = register_nums[0];

        int binaryInstruction = opcode + jumpAddress;
        printf("7an JMP opcode:%d jumpAddress:%d   \n binaryInstruction: %d", opcode, jumpAddress, binaryInstruction);
        return binaryInstruction;
    }
    else if (strncmp(instruction, "LSL", 3) == 0)
    {
        // Logical shift left
        opcode = 0b10000000000000000000000000000000;
        registerNumbers(instruction, register_nums, 3);

        destinationRegister = register_nums[0] << 23;
        numberOfRegister1 = register_nums[1] << 18;
        shamt = register_nums[2] << 13;

        int binaryInstruction = opcode + destinationRegister + numberOfRegister1 + shamt;
        printf("7an LSL opcode:%d destRegister:%d R1:%d shamt:%d  \n binaryInstruction: %d", opcode, destinationRegister, numberOfRegister1, shamt, binaryInstruction);
        return binaryInstruction;
    }
    else if (strncmp(instruction, "LSR", 3) == 0)
    {
        // Logical shift right
        opcode = 0b10010000000000000000000000000000;
        registerNumbers(instruction, register_nums, 3);

        destinationRegister = register_nums[0] << 23;
        numberOfRegister1 = register_nums[1] << 18;
        shamt = register_nums[2] << 13;

        int binaryInstruction = opcode + destinationRegister + numberOfRegister1 + shamt;
        printf("7an LSR opcode:%d destRegister:%d R1:%d shamt:%d  \n binaryInstruction: %d", opcode, destinationRegister, numberOfRegister1, shamt, binaryInstruction);
        return binaryInstruction;
    }
    else if (strncmp(instruction, "MOVR", 3) == 0)
    {
        // Move to register
        opcode = 0b10100000000000000000000000000000;
        registerNumbers(instruction, register_nums, 3);

        destinationRegister = register_nums[0] << 23;
        numberOfRegister1 = register_nums[1] << 18;
        immediate = register_nums[2] << 13;

        int binaryInstruction = opcode + destinationRegister + numberOfRegister1 + immediate;
        printf("7an MOVR opcode:%d destRegister:%d R1:%d immediate:%d  \n binaryInstruction: %d", opcode, destinationRegister, numberOfRegister1, immediate, binaryInstruction);
        return binaryInstruction;
    }
    else if (strncmp(instruction, "MOVM", 3) == 0)
    {
        // Move to memory
        opcode = 0b10110000000000000000000000000000;
        registerNumbers(instruction, register_nums, 3);

        destinationRegister = register_nums[0] << 23;
        numberOfRegister1 = register_nums[1] << 18;
        immediate = register_nums[2] << 13;

        int binaryInstruction = opcode + destinationRegister + numberOfRegister1 + immediate;
        printf("7an MOVM opcode:%d destRegister:%d R1:%d R2:%d  \n binaryInstruction: %d", opcode, destinationRegister, numberOfRegister1, immediate, binaryInstruction);
        return binaryInstruction;
    }
    else
    {
        // Handle unknown instruction
        printf("Unknown instruction: %s\n", instruction);
        return 0;
    }
}

void pipeline(Memory *mem, Registers *regs)
{
    printf("\n-----------------------\n \n");
    int result;
    int memoryValue;
    int instruction;
    int oldInstructionData[10] = {0};
    int newInstructionData[10] = {0};
    int oldestInstructionData[10] = {0};
    int clock = 1;
    int counter = 10;
    int counterFlag = 0; // false
    int tempInstructionNum;
    // false
    // int opcode = instructionData[0];
    // int R1 = instructionData[1];
    // int R2 = instructionData[2];
    // int immediate = instructionData[6];

    // fetch--> PC
    // decode--> PC-1
    // excute --> PC-2
    // memoryAccess
    // WB

    // THE WHILE CONDITION MUST BE ADJUSTED SUCH THAT 1024 IS EXCHANGED WITH THE NUMBER OF CURRENT INSTRUCTIONS
    while (counter != 7)
    {

        printf("starting clock cycle number: %d \n", clock);

        if (clock >= 7 && clock % 2 == 1 && (jumpFlag < 4 || jumpFlag > 7))
        {

            if (jumpFlag == 1)
            {
                printf("Write back instruction: %d \n", tempInstructionNum);
            }
            else if (jumpFlag == 3)
            {
                printf("Write back instruction: %d \n", tempInstructionNum + 1);
            }
            else
            {
                printf("Write back instruction: %d \n", (regs->PC) - 3);
            }

            writeBack(regs, memoryValue, oldestInstructionData[1], oldestInstructionData);

            for (int i = 0; i < 10; i++)
            {
                oldestInstructionData[i] = oldInstructionData[i];
            }
        }

        if (clock >= 6 && clock % 2 == 0 && (jumpFlag < 3 || jumpFlag > 6))
        {

            if (jumpFlag == 0)
            {
                printf("MEM access instruction: %d \n", tempInstructionNum);
            }
            else if (jumpFlag == 2)
            {

                printf("MEM access instruction: %d \n", tempInstructionNum + 1);
            }
            else
            {
                printf("MEM access instruction: %d \n", (regs->PC) - 3);
            }

            memoryValue = memoryAccess(mem, result, oldInstructionData[1], oldInstructionData);
            for (int i = 0; i < 10; i++)
            {
                oldInstructionData[i] = newInstructionData[i];
            }
        }

        if (clock >= 4 && (jumpFlag == 0 || jumpFlag == 1 || jumpFlag > 5) && (counter < 5 || counter >= 10))
        {

            // PRINTING TWICE PER INSTRUCTION
            printf("Excuting instruction: %d \n", (regs->PC) - 2);
            if (clock % 2 == 0)
            {
                // ACTUAL CALL ONLY HAPPENS ONCE
                result = execute(regs, mem, newInstructionData);

                if (jumpFlag == 0)
                {
                    tempInstructionNum = regs->PC - 3;
                }
            }
        }

        if (clock >= 2 && jumpFlag > 3 && (counter < 3 || counter >= 10))
        {
            // DECODE DONE
            if (clock % 2 == 0)
            {
                if (clock == 2)
                {
                    decode(regs, instruction, oldestInstructionData);
                    decode(regs, instruction, oldInstructionData);
                    decode(regs, instruction, newInstructionData);
                }
                else
                {

                    decode(regs, instruction, newInstructionData);
                }
            }
            printf("Decoding instruction: %d \n", (regs->PC) - 1);
            // note that this could've caused printing the wrong number since the PC is incremented within the odd
            // cycles. However, fetching is done after decoding so the PC would not have been incremented by now
            // in the code
        }

        // FETCHING DONE
        if (clock >= 1 && clock % 2 == 1 && jumpFlag > 2 && counter >= 10)
        {
            if (regs->PC == mem->instructionCount)
            {
            }
            else
            {
                printf("Fetching instruction: %d \n", regs->PC);
            }
            instruction = fetch(mem, regs);
            printf("PC is now: %d \n", regs->PC);
        }

        // printMemory(mem);
        printRegisters(regs);
        printf("------------------------------------------- \n");
        clock++;
        jumpFlag++;

        counter++;
        if (regs->PC > mem->instructionCount && counterFlag == 0)
        {

            counter = 0;
            counterFlag = 1;
        }
    }

    printf("instructions done");
}

void printMemory(Memory *mem)
{

    for (int i = 0; i < mem->instructionCount; i++)
        printf("instruction %d has the value %d \n\n", i, mem->words[i]);

    for (int i = 1024; i < 2048; i++)
    {

        printf("Data at position %d has the value %d \n\n", i, mem->words[i]);
    }
}

void printRegisters(Registers *regs)
{
    for (int i = 0; i < 3; i++)
    {
        printf("GPR %d has the value %d\n \n ", i, regs->GPRS[i]);
    }

    printf("PC has the value %d\n \n ", regs->PC);
}

int main()
{
    Memory mem;
    initializeMemory(&mem);

    Registers regs = {
        .R0 = 0,     // R0 initialized to 0
        .GPRS = {0}, // All GPRS initialized to 0
        .PC = 0      // PC initialized to 0
    };

    regs.GPRS[0] = 0;
    regs.GPRS[1] = 1;
    regs.GPRS[2] = 2;
    regs.GPRS[3] = 3;
    regs.GPRS[4] = 4;

    // mem.words[0] = 0;
    // mem.words[1] = 1;
    // mem.words[2] = 2;
    // mem.words[3] = 3;
    // mem.instructionCount = 6;
    // int instructionData[10] = {0};
    // //     int *decode(Registers *regs, int instruction)
    // // {

    // //     int opcode = 0;
    // //     int R1 = 0;
    // //     int R2 = 0;
    // //     int R3 = 0;
    // //     int SHAMT = 0;
    // regs.GPRS[0] = 1;
    // regs.GPRS[1] = 2;
    // regs.GPRS[5] = 66;
    // decode(&regs, 0b00000000100010001100000000000110, instructionData);
    // // 1 2 6
    // printf("opcode is %d ,  %d %d %d \n", instructionData[0], instructionData[1], instructionData[2], instructionData[3]);
    // // 1 2 66
    // printf("%d %d %d", instructionData[7], instructionData[8], instructionData[9]);

    const char *commands = {"commands.txt"};
    encodeInstructions(commands, &mem, 15);
    pipeline(&mem, &regs);
    printf("\n \n%d", mem.instructionCount);
    // pipeline(&mem, &regs);
    return 0;
}