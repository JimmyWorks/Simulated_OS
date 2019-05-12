//   Simulating an Operating System using Multiple Processes and IPC
//
//   Author: Jimmy Nguyen
//   Email:  Jimmy@JimmyWorks.net
//
//   Description:
//   Develop a multi-process program which emulates a
//   basic operating system where the processor is the
//   parent process and main memory is a child process.
//   The processor process will communicate with the 
//   main memory process through signals and pipes for
//   read/write, I/O operations.  The processor contains
//   an array to emulate registers (PC, SP, IR, AC, X, Y)
//   while main memory contains an array of 2000 elements
//   to emulate memory space.  The processor process
//   will simulate the execution cycle (fetch, decode, and
//   execute), interrupt handling, mode switching (user and
//   kernel mode), user and system stack, timeout timer, and
//   implement over 30 different operations for the
//   instruction set.
//
//   Processor
//   Implementation below is executed only by the processor
//   process which handles all the responsiblities of the OS
//   described above.  Any access to main memory involves a
//   system call which pipes the I/O operation to the
//   main memory process.

#include <iostream>
#include <string>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include <stdexcept>
#include "program.h"
using namespace std;

// Methods
void fetchInstruction();
void executeInstruction();
void run_execution_cycle();
void verifyAccess(int address);
void endProcess(int errorCode);
int  readMemory(int address);
void writeMemory(int address, int value);
void syscall(int address);
void return_syscall();
void checkInterrupt();
void setTimer(int argc, char *argv[]);
void pushRegistersOnStack();
void popRegistersOnStack();
void debugProgram();
void printRegistersAndStack();
void pushStack(int value);
int  popStack();

// Timer, counters, and inactive stack values
int interrupt_timer;
int instruction_counter;
int inactive_sys_stack, inactive_proc_stack;

// Registers
int registers[REGCOUNT];

// Process Ids
int *process;

// Pipes to Main Memory
int *writeToMem;
int *readFromMem;

// Interrupt Enabled and Kernel Mode Flags
bool interruptEnabledFlag;
bool kernelMode;

/* Run Processor
 * Main processor routine which executes once the main memory has initialized.
 * Declares and initializes all variables needed by the process.
 * Creates and initializes registers.
 * Runs execution loop.
 *
 * <timer> instruction count till timeout
 * <pid> process id array
 * <wToMem> write pipe to main memory
 * <rFromMem> read pipe from main memory
 * <debugMode> debug mode flag
 * <exit> returns program exit status
 */
void run_processor(int timer, int *pid, int wToMem[], int rFromMem[], bool debugMode)
{
   // Assign variables
   process = pid;
   writeToMem = wToMem;
   readFromMem = rFromMem;

   // Set timer 
   interrupt_timer = timer;

   // Initialize registers and flags 
   registers[PC] = 0;
   inactive_sys_stack = MEMORY_SIZE;
   registers[SP] = inactive_proc_stack = SYS_INDEX;
   instruction_counter = 0;
   interruptEnabledFlag = true;
   kernelMode = false;

   // Run debug output or run execution loop
   if(debugMode)
      debugProgram();
   else
      run_execution_cycle();

   // Program should never reach this point
   endProcess(PROGRAM_PATH_FAILURE);
}

/* Run Execution Cycle
 * Fetch instruction, increment program counter,
 * execute instruction, increment instruction counter,
 * and check for interrupts.  Repeat.
 */
void run_execution_cycle()
{
   while(true)
   {
      fetchInstruction();
      registers[PC]++;
      executeInstruction();
      instruction_counter++;
      checkInterrupt();
   }
}

/* Push Stack
 * Pushes a value onto the stack
 *
 * <value> value to push onto stack
 */
void pushStack(int value)
{
   writeMemory(--registers[SP], value);
}

/* Pop Stack
 * Pop a value form the stack
 *
 * <return> value popped from stack
 */
int popStack()
{
   return readMemory(registers[SP]++);
}

/* Push Registers on Stack
 * Push all registers excluding the SP register onto the stack
 */
void pushRegistersOnStack()
{
   // For each register except the last one (SP)
   for(int i=1; i < REGCOUNT; i++)
   {
      // Write it to the stack at offset below stack pointer
      writeMemory(registers[SP]-i, registers[i-1]);
   }

   // Update the stack pointer
   registers[SP] -= (REGCOUNT-1);
}

/* Pop Registers on Stack
 * Pop all registers excluding the SP register off the stack
 */
void popRegistersOnStack()
{
   // For each register execpt the last one (SP)
   for(int i=1; i < REGCOUNT; i++)
   {
      // Pop it off the stack onto the register
      registers[i-1] = readMemory(registers[SP]+REGCOUNT-1-i);
   }

   // Update the stack pointer
   registers[SP] += (REGCOUNT-1);
}


/* Verify Access
 * Check if address is valid and if the mode allows
 * access to that memory space.  Restrict access
 * to system memory to kernel mode only.
 *
 * <address> address being accessed
 */
void verifyAccess(int address)
{
   // Throw exception if out of bounds
   if(address < 0 || address >= MEMORY_SIZE)
   {
      endProcess(MEMORY_OUT_OF_BOUNDS);
   }
   // Throw exception if user mode accessing
   // system memory
   if(address >= SYS_INDEX && !kernelMode)
   {
      endProcess(KERNEL_MEM_ACCESS_DENIED);
   }
   // Throw exception if kernel mode is
   // modifying user program
   if(address < SYS_INDEX && kernelMode)
   {
      endProcess(USER_MEM_ACCESS_DENIED);
   }
}

/* Read Memory
 * Fetch value from main memory.
 *
 * <address> address to fetch value from
 */
int readMemory(int address)
{
   // Verify permissions and valid address
   verifyAccess(address);
   
   // Write I/O operation and address to pipe
   // Send the SIGINT to execute the call
   int action = READ;
   write(writeToMem[1], &action, sizeof(int));
   write(writeToMem[1], &address, sizeof(int));
   kill(process[MAIN_MEMORY], SIGINT);
   
   // Verify if successful
   int status;
   read(readFromMem[0], &status, sizeof(int));
   if(status != SUCCESS)
   {
      endProcess(status);
   }
   
   // Return the value
   read(readFromMem[0], &status, sizeof(int));
   return status;
   
}

/* Write Memory
 * Write value to address in main memory.
 *
 * <address> address to write value to
 * <value> value to write
 */
void writeMemory(int address, int value)
{
   // Verify permissions and valid address
   verifyAccess(address);

   // Write I/O operation, address, and value to pipe
   // Send the SIGINT to execute the call
   int action = WRITE;
   write(writeToMem[1], &action, sizeof(int));
   write(writeToMem[1], &address, sizeof(int));
   write(writeToMem[1], &value, sizeof(int));
   kill(process[MAIN_MEMORY], SIGINT);

   // Verify if successful
   int status;
   read(readFromMem[0], &status, sizeof(int));
   if(status != SUCCESS)
   {
      endProcess(status);      
   }
}

/* Check interrupt
 * Make syscall for timeout if instruction counter
 * exceeds the interrupt timer set.
 */
void checkInterrupt()
{
   if(instruction_counter % interrupt_timer == 0)
      syscall(SYS_INDEX);
}

/* System Call
 * If interrupts are enabled, checks if the system
 * is in kernel mode.  If not, a mode switch is
 * executed and the system goes to the interrupt
 * handler specified address.
 *
 * <address> interrupt handler address
 */
void syscall(int address)
{
   // If interrupt enabled
   if(interruptEnabledFlag)
   {
      // If not kernel mode, mode switch
      if(!kernelMode)
      {
         // Mode switch
         kernelMode = true;
	 //Disable recursive interrupts
         interruptEnabledFlag = false;
	 // Switch stack pointers
	 inactive_proc_stack = registers[SP]; 
	 registers[SP] = inactive_sys_stack;
	 // Push registers onto stack
         pushRegistersOnStack();
	 // Execute interrupt handler
         registers[PC] = address;
      }
      else
      {
         // Recursive interrupt (not implemented)
      }
   }
}

/* Return from System Call
 * Revert all register values and mode switch back to user mode.
 */
void return_syscall()
{
   // Pop register values from stack
   popRegistersOnStack();
   // Switch stack pointers
   inactive_sys_stack = registers[SP];
   registers[SP] = inactive_proc_stack;
   // Enable interrupts and mode switch to user mode
   interruptEnabledFlag = true;
   kernelMode = false;
}

/* End Process
 * Exits the program after killing the main memory process.
 * Exit code is printed to the console and returned.
 *
 * <exitCode> Exit status value
 */
void endProcess(int exitCode)
{
   // Terminate the main memory process
   kill(process[MAIN_MEMORY], SIGKILL);
   
   // Print the exit status
   cout << "EXIT CODE: ";
   switch(exitCode)
   {
      case SUCCESS: cout << "SUCCESS"; break;
      case CLI_FAILURE: cout << "CLI FAILURE"; break;
      case FORK_FAILURE: cout << "FORK FAILURE"; break;
      case PIPE_FAILURE: cout << "PIPE FAILURE"; break;
      case FILE_PARSE_FAILURE: cout << "FILE PARSE FAILURE"; break;
      case INVALID_OPCODE: cout << "INVALID OPCODE"; break;
      case PROGRAM_PATH_FAILURE: cout << "PROGRAM PATH FAILURE"; break;
      case READ_FAILURE: cout << "READ FAILURE"; break;
      case WRITE_FAILURE: cout << "WRITE FAILURE"; break;
      case INVALID_MEM_ACTION: cout << "INVALID MEM ACTION"; break;
      case MEMORY_OUT_OF_BOUNDS: cout << "MEMORY OUT OF BOUNDS"; break;
      case KERNEL_MEM_ACCESS_DENIED: cout << "KERNEL_MEM_ACCESS_DENIED"; break;
      case USER_MEM_ACCESS_DENIED: cout << "USER_MEM_ACCESS_DENIED"; break;
      case INVALID_PORT_CALL: cout << "INVALID PORT CALL"; break;
      default: cout << "MISSING EXIT CODE"; break;
   }
   
   cout << endl << endl;
 
   // Return exit status value and end program
   exit(exitCode);
}

/* Fetch Instruction
 * Simply read the next instruction from main memory based on PC register
 * and store in IR register.
 */
void fetchInstruction()
{
   registers[IR] = readMemory(registers[PC]);
}

/* Execute Instruction
 * Decode the value in IR register using switch statement.
 * Execute based on case.
 */
void executeInstruction()
{
      int temp; // Temp value for instruction execution

      // Decode instruction in IR register
      switch(registers[IR])
      {
         case  LOAD_VAL:      
	         // Load value on next line into AC register
                 registers[AC] = readMemory(registers[PC]++);
	         break;
	 case  LOAD_ADDR: 
	         // Load value at address into AC register
	         temp = readMemory(registers[PC]++);
		 registers[AC] = readMemory(temp);
	 	 break;
	 case  LOAD_IND_ADDR: 
	         // Load value from address found in given address into AC register
		 temp = readMemory(registers[PC]++);
		 temp = readMemory(temp);
		 registers[AC] = readMemory(temp);
	 	 break;
	 case  LOAD_IDX_X_ADDR: 
	         // Load Idx X Addr: Load value at address + offset X into AC register
	 	 temp = readMemory(registers[PC]++);
		 registers[AC] = readMemory(temp + registers[X]);
	 	 break;
	 case  LOAD_IDX_Y_ADDR: 
	         // Load Idx Y Addr: Load value at 
	 	 temp = readMemory(registers[PC]++);
		 registers[AC] = readMemory(temp + registers[Y]);
	 	 break;
	 case  LOAD_SPX: 
	         // Load SP + X into AC 
		 registers[AC] = readMemory(registers[SP] + registers[X]);
	 	 break;
	 case  STORE: 
	         // Store AC into address on next line
	 	 temp = readMemory(registers[PC]++);
		 writeMemory(temp, registers[AC]);
	 	 break;
	 case  GET: 
	         // Get random value between 1-100
	 	 registers[AC] = (rand() % 100) + 1;
	 	 break;
	 case  PUT: 
	         // Put command based on port value in next line
	 	 temp = readMemory(registers[PC]++);
		 switch(temp)
		 {
		    case 1: // Print int in AC as int
		           cout << registers[AC];
			   break;
	            case 2: // Print int in AC as char
		           cout << (char)registers[AC];
			   break;
		    default:
		           endProcess(INVALID_PORT_CALL);
			   break;
		 }
	 	 break;
	 case ADDX:
	         // Add X to AC
	 	 registers[AC] += registers[X];
	 	 break;
	 case ADDY: 
	         // Add Y to AC
	 	 registers[AC] += registers[Y];
	 	 break;
	 case SUBX: 
	         // Subtract X from AC
	 	 registers[AC] -= registers[X];
	 	 break;
	 case SUBY: 
	         // Subtract Y from AC
	 	 registers[AC] -= registers[Y];
	 	 break;
	 case COPY_TO_X: 
	         // Copy AC to X
	 	 registers[X] = registers[AC];
	 	 break;
	 case COPY_FR_X:
	         // Copy X to AC
	 	 registers[AC] = registers[X];
	 	 break;
	 case COPY_TO_Y: 
	         // Copy AC to Y
	 	 registers[Y] = registers[AC];
	 	 break;
	 case COPY_FR_Y:  
                 // Copy Y to AC
	 	 registers[AC] = registers[Y];
	 	 break;
	 case COPY_TO_SP: 
	         // Copy AC to SP
	 	 registers[SP] = registers[AC];
	 	 break;
	 case COPY_FR_SP: 
	         // Copy SP to AC
	 	 registers[AC] = registers[SP];
	 	 break;
         case JUMP: 
	         // Jump to address
	 	 registers[PC] = readMemory(registers[PC]);
	 	 break;
	 case JUMP_IF_EQ: 
	         // Jump to address only if value in AC is zero
		 temp = readMemory(registers[PC]++);
	 	 if(!registers[AC])
		    registers[PC] = temp;
		 break;
	 case JUMP_IF_NEQ: 
	         // Jump to address only if value in AC is not zero
		 temp = readMemory(registers[PC]++);
	 	 if(registers[AC])
		    registers[PC] = temp;
	 	 break;
	 case JUMP_RETURN: 
	         // Push return address onto stack, jump to the address
	 	 pushStack(registers[PC] + 1);
		 registers[PC] = readMemory(registers[PC]);
		 break;
	 case RETURN: 
	         // Pop return address from the stack, jump to the address
		 registers[PC] = popStack();
	 	 break;
	 case INCX: 
	         // Increment value in X
	 	 registers[X]++;
	 	 break;
	 case DECX: 
	         // Decrement value in X
	 	 registers[X]--;
	 	 break;
	 case PUSH: 
	         // Push: Push AC onto stack
	 	 pushStack(registers[AC]);
	 	 break;
	 case POP: 
	         // Pop from stack into AC
	 	 registers[AC] = popStack();
	 	 break;
	 case SYSCALL: 
	         // Perform system call
                 syscall(INT_INDEX);	 	 
	 	 break;
	 case SYSRETURN: 
	         // Return from system call
	 	 return_syscall(); 
	 	 break;
	 case END: 
	         // End execution
	 	 endProcess(SUCCESS); 
	 	 break;
	 default: 
	         // Invalid Op Code
	 	 endProcess(INVALID_OPCODE); 
      }
}

/* Print Registers and Stack
 * Prints values in the registers and stack for debugging
 */
void printRegistersAndStack()
{
   cout << "Registers: " << endl;
   cout << "PC: " << registers[PC] << endl;
   cout << "SP: " << registers[SP] << endl;
   cout << "IR: " << registers[IR] << endl;
   cout << "AC: " << registers[AC] << endl;
   cout << "X: " << registers[X] << endl;
   cout << "Y: " << registers[Y] << endl;

   for(int i =0; i < 10; i++)
      cout << "Mem Address " << 1999-i << ": " << readMemory(1999-i) << endl;

}

/* Debug Program
 * Tests the routines for pushing registers to the stack
 * and popping registers from the stack.  Shows if values
 * are retained and if stack pointer updates properly.
 */
void debugProgram()
{
   cout << "TESTING MEMORY READ/WRITE" << endl;
   cout << "Read address 10: " << readMemory(10) << endl;
   cout << "Write 1337 to address 10" << endl;
   writeMemory(10, 1337);
   cout << "Read address 10: " << readMemory(10) << endl << endl << endl;
   
   cout << "TESTING STACK PUSH/POP" << endl << endl;
   cout << "INITIAL:" << endl;
   kernelMode = true;
   registers[SP] = inactive_sys_stack;
   registers[IR] = 10;
   registers[AC] = 20;
   registers[X] = 30;
   registers[Y] = 40;
   
   printRegistersAndStack();

   cout << endl <<"Pushing stack..." << endl;
   pushRegistersOnStack();
   
   printRegistersAndStack();

   cout << endl << "Overwriting register values..." << endl;
   registers[IR] = 99;
   registers[AC] = 88;
   registers[X] = 77;
   registers[Y] = 66;


   printRegistersAndStack();

   cout << endl << "Popping stack..." << endl;
   popRegistersOnStack();

   printRegistersAndStack();

   cout << endl << "END OF TESTING SECTION" << endl << endl;
   endProcess(SUCCESS);
}
