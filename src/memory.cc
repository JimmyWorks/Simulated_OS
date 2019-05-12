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
//   Main Memory
//   Implementation below is executed only by the main memory
//   process which is responsible for setting up the memory
//   space, initializing the user program from the input file
//   as the simulated, loaded program to execute, and simply
//   executing read and write operations from the processor
//   process.


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <signal.h>
#include <sys/types.h>
#include "program.h"
using namespace std;

// Main Memory -- addressable memory space
int memory[MEMORY_SIZE];

// I/O pipes to processor
int *readpipe;
int *writepipe;

// Methods
void signalhandler(int signum);

/* Run Main Memory
 * Initial routine for running the main memory process.
 * Declares, initializes and assignes values needed.
 * Opens and parses the input user program file.
 * Populates main memory.
 * Returns status to processor.
 * Waits until an I/O operation is invoked when receiving
 * a SIGINT signal.
 * 
 * <file> input file path
 * <rpipe> read pipe
 * <wpipe> write pipe
 * <debugMode> debug flag
 */
void run_main_memory(char* file, int rpipe[], int wpipe[], bool debugMode)
{
   // Process SIGINT signals
   signal(SIGINT, signalhandler);
   // Assign pipes
   readpipe = rpipe;
   writepipe = wpipe;

   // Process the input file
   fstream file_stream;
   int success = 0;
   try{
      file_stream.open(file);

      // If file can be opened...
      if(file_stream.is_open())
      {
         std::string line;
	 int address = 0; // starting at address 0

	 // While not EOF and a line exists
	 while(getline(file_stream, line))
	 {
	    // If the line is not empty...
	    if(!line.empty())
	    {
	       const char *c_line = line.c_str();
               int operation, startIndex;

	       // Go through each line until a char is found
	       for(unsigned int i = 0; i<line.length(); i++)
	       {
	          // Skip spaces
	          if(c_line[i] == ' ')
	          {
	             // ignore spaces
	          }
		  // If '.' encountered, this is a JUMP_AHEAD
	          else if(c_line[i] == '.')
	          {
	             operation = JUMP_AHEAD;
		     startIndex = i + 1;
		     break;
  	          }
		  // If # is encountered, this is a LOAD
	          else if(isdigit(c_line[i]))
	          {
	             operation = LOAD;
		     startIndex = i;
		     break;
	          }
		  // Else, this is a comment
	          else
	          {
		     operation = SKIP;
	             break;
	          }
	       }

               // Process the line based on the operation
	       if(operation == JUMP_AHEAD)
	       {
	       // For jump, process the rest of the number
	       // and set the address to that value
	       string jumpAddress = "";
	       unsigned int i = startIndex;
	       while(i < line.length() && isdigit(c_line[i]))
	          {
	             jumpAddress += c_line[i];
		     i++;
	          }
	          address = stoi(jumpAddress);
	       }
	       else if(operation == LOAD)
	       {
	       // For load, process the rest of the number
	       // and set the value at the current address
	          string loadValue = "";
	          unsigned int i = startIndex;
	          while(i < line.length() && isdigit(c_line[i]))
	          {
	             loadValue += c_line[i];
		     i++;
	          }
                  memory[address] = stoi(loadValue);
                  address++;
	       }
	       else if(operation == SKIP)
	       {
	          // For skip, do nothing
	       }
	       else
	       {
	          // Else, throw error
	          throw;
	       }
	    }
	 }
	 // If no errors thrown, return success 
         success = 1;
      }
      else // Throw exception if file cannot be opened
         throw;
   }catch(...)
   {
      cout << "ERROR PARSING FILE!!!!" << endl;

   }
   // Close stream
   file_stream.close();

   // DEBUG SECTION
   // If debug flag set, print a few lines from each memory section
   if(debugMode)
   {
      // Print some address and values from user space
      for(int i = 0; i < 300; i++)
         cout << i << ": " << memory[i] << endl;
      cout << endl;
      // Print some address and values from lower system space
      for(int i = SYS_INDEX; i < (SYS_INDEX+15); i++)
         cout << i << ": " << memory[i] << endl;
      cout << endl;
      // Print some address and values from upper system space
      for(int i = INT_INDEX; i < (INT_INDEX+20); i++)
         cout << i << ": " << memory[i] << endl;
      cout << endl;
   }

   // Return if main memory was successful in initialization
   write(writepipe[1], &success, sizeof(int));

   // Wait for a signal to process
   while(1)
   {
      pause();
   }
}


/* Signal Handler
 * Prompts the process to check the read pipe for
 * and I/O operation. If a READ operation is called,
 * the process will check the read pipe again for an
 * address, fetch the value at that address and 
 * return a success write and the value.  If a WRITE
 * operation is called, the process will check the
 * read pipe two more times for the address and value.
 * After writing the value to the address, a success
 * write will be returned.
 *
 * <signum> signal received (SIGINT)
 */
void signalhandler(int signum)
{
   int action;
   int address;
   int value;
   int returnCode;

   // REad the action and the address
   read(readpipe[0], &action, sizeof(int));
   read(readpipe[0], &address, sizeof(int));
   
   if(action == READ) // If read action
   {
      // For valid memory address,
      // write into write pipe the return code
      // write into write pipe the value at address
      if(address >= 0 && address < MEMORY_SIZE)
      {
         returnCode = SUCCESS;
         write(writepipe[1], &returnCode, sizeof(int));
         write(writepipe[1], &memory[address], sizeof(int)); 
      }
      else // else return failed operation
      {
         returnCode = READ_FAILURE;
         write(writepipe[1], &returnCode, sizeof(int));
      }

   }
   else if(action == WRITE) // If write action
   {
      // For valid memory address,
      // read again for value to write
      // write the value to the address space
      // write into write pipe the return code
      if(address >= 0 && address < MEMORY_SIZE)
      {
         read(readpipe[0], &value, sizeof(int));
         memory[address] = value;
	 returnCode = SUCCESS;
         write(writepipe[1], &returnCode, sizeof(int));
      }
      else // else return failed operation
      {
         returnCode = WRITE_FAILURE;
         write(writepipe[1], &returnCode, sizeof(int));
      }
   }
   else // Else, invalid memory action
   {
      returnCode = INVALID_MEM_ACTION;
      write(writepipe[1], &returnCode, sizeof(int));
   }
}
