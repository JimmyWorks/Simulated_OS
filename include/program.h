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
//   Header file 


#ifndef _PROGRAM_1_H_
#define _PROGRAM_1_H_

// Defined memory size and indices
#define MEMORY_SIZE 2000
#define SYS_INDEX 1000
#define INT_INDEX 1500

// Registers
enum register_values
{
   PC,
   IR,
   AC,
   X,
   Y,
   SP,
   REGCOUNT
};

// Parse operations
enum parse_op
{
   JUMP_AHEAD,
   LOAD,
   SKIP
};

// Memory I/O operations
enum mem_codes
{
   READ,
   WRITE
};

// Program return error codes
enum error_codes
{
   SUCCESS,
   CLI_FAILURE,
   FORK_FAILURE,
   PIPE_FAILURE,
   FILE_PARSE_FAILURE,
   INVALID_OPCODE,
   PROGRAM_PATH_FAILURE,
   READ_FAILURE,
   WRITE_FAILURE,
   INVALID_MEM_ACTION,
   MEMORY_OUT_OF_BOUNDS,
   KERNEL_MEM_ACCESS_DENIED,
   USER_MEM_ACCESS_DENIED,
   INVALID_PORT_CALL,
   ERRCOUNT
};

// Instruction Set
enum instruction
{
   LOAD_VAL = 1,
   LOAD_ADDR,
   LOAD_IND_ADDR,
   LOAD_IDX_X_ADDR,
   LOAD_IDX_Y_ADDR,
   LOAD_SPX,
   STORE,
   GET,
   PUT,
   ADDX,
   ADDY,
   SUBX,
   SUBY,
   COPY_TO_X,
   COPY_FR_X,
   COPY_TO_Y,
   COPY_FR_Y,
   COPY_TO_SP,
   COPY_FR_SP,
   JUMP,
   JUMP_IF_EQ,
   JUMP_IF_NEQ,
   JUMP_RETURN,
   RETURN,
   INCX,
   DECX,
   PUSH,
   POP,
   SYSCALL,
   SYSRETURN,
   END = 50,
};

// Process Id indices
enum pid_values
{
   PROCESSOR,
   MAIN_MEMORY,
   PIDCOUNT

};

// Methods
void run_main_memory(char* file, int readpipe[], int writepipe[], bool debugMode);
void run_processor(int timer, int *pid, int writeToMem[], int readFromMem[], bool debugMode);

#endif
