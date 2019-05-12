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
//   Program Main
//   Initial setup which involves setting up the process for
//   running the processor and the process for running the
//   main memory.


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <math.h>
#include <cstdlib>
#include "program.h"
using namespace std;

// Methods
bool existingFile(const char *path);

/* Program Main
 * Verifies if commmand-line input is valid, forks the
 * process to allow for two processes, sets up the pipes
 * for IPC, and initializes both processes depending on
 * parent-child process.
 *
 * <argc> arg count
 * <argv> command-line arguments
 * <return> return code indicating final program state
 */
int main(int argc, char* argv[])
{
   // Timer value and Debug flag
   int timer;
   bool debugMode;

   // Verify command-line values before continuing...
   try{
      // Must have 3-4 arguments
      if(argc < 3 || argc > 4)
      {
         cout << "ERROR: Invalid options" << endl << endl;
	 cout << "Usage: program1.exe <program_file> <timer_value> [--debug]" << endl << endl;
         throw;
      }
      // Third argument must be a natural number
      if(stoi(argv[2]) < 0)
      {
         cout << "ERROR: Invalid options." << endl; 
	 cout << "Timer value must be integer greater than zero." << endl << endl;
	 cout << "Usage: program1.exe <program_file> <timer_value> [--debug]" << endl <<endl;
         throw;
      }
      // Second argument must be a file path that exists and readable
      if(!existingFile(argv[1]))
      {
         cout << "ERROR: Program file does not exist!" << endl << endl;
	 cout << "Usage: program1.exe <program_file> <timer_value> [--debug]" << endl << endl;
         throw;
      }

      // If Fourth argument exists, it must be the --debug flag
      string debug = "--debug";
      if(argc == 4 && debug.compare(argv[3]) != 0)
      {
         cout << "ERROR: Invalid options" << endl; 
	 cout << "Usage: program1.exe <program_file> <timer_value> [--debug]" << endl << endl;
         throw;
      }

      // Get the interrupt timer
      timer = stoi(argv[2], NULL, 0);

      // Check if debug mode
      if(argc == 4)
         debugMode = true;
      else
         debugMode = false;

   }catch(...){
      return CLI_FAILURE;
   }

   // Create array of process IDs
   int processID[(int)pid_values::PIDCOUNT];
   // Get processor process id
   processID[PROCESSOR] = getpid();

   // Create pipes for IPC
   int procToMem[2];
   int memToProc[2];
   
   if( pipe(procToMem) == -1 || 
       pipe(memToProc) == -1 ) 
   {
      cout << "Failed pipe creation";
      return PIPE_FAILURE;
   }

   // Fork for main memory process
   int pid = -1;
   pid = fork();

   if(pid == -1)
   {
      cerr << "Failed to fork" << endl;
      return FORK_FAILURE;
   }
   else if(pid == 0)
   {
      // Child: Run main memory process
      run_main_memory(argv[1], procToMem, memToProc, debugMode);
   }
   else
   {
      // Parent: Processor process
      // Store the child process pid for main memory
      processID[MAIN_MEMORY] = pid;

      // Check if main memory intialized successfully
      int loadSuccess;
      read(memToProc[0], &loadSuccess, sizeof(loadSuccess));
      if(!loadSuccess)
         return FILE_PARSE_FAILURE;

      // Now that main memory has initialized, have parent run as processor
      run_processor(timer, processID, procToMem, memToProc, debugMode);
      
   }

   // Program should never reach this point
   return PROGRAM_PATH_FAILURE;
}

/* Existing File Check
 * Check if the file exists
 *
 * <path> relative path to file
 * <return> bool if file exists
 */
bool existingFile(const char *path)
{
   // If file can be opened for read,
   // return true
   if(FILE *file = fopen(path, "r"))
   {
      fclose(file);
      return true;
   }
   else
   {
     return false;
   }
}
