#   Simulating an Operating System using Multiple Processes and IPC
#
#   Author: Jimmy Nguyen
#   Email:  Jimmy@JimmyWorks.net
#
#   Commands:
#   make		Make all executables.
#   make clean		Clean all intermediate files
#   make test -i	Test the program in command terminal ignoring errors
#   make backup 	Make a backup of the current project

# Project name for make backup
PROJECTNAME = OS_Concepts_Project1

# Filenames
 # Source
SRCS = main.cc \
       processor.cc \
       memory.cc \

 # Executables
EXE = program.exe
 # Input Files
INPUT1 = sample1.txt
INPUT2 = sample2.txt
INPUT3 = sample3.txt
INPUT4 = sample4.txt
INPUT5 = sample5.txt

 # Directories 
INPUTDIR = ../input/
OUTPUTDIR = ../output/
BIN_DIR = ../bin/
SRC_DIR = ../src/

# Compilers and Flags

CXX = g++
CXXFLAGS =  -Wall -I../include/ -std=c++11 
CPPFLAGS = -Wall -I../include/

# Make Targets
OBJS=$(SRCS:cc=o)

 # make
all: $(EXE) 

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN_DIR)$@ 

 # make clean
clean: backup
	rm -f *.o $(OUTPUTDIR)* *.d*

 # make test
test: $(all)
	@clear
	@clear
	# TESTING WITH "sample1.txt" #################
	@echo
	$(BIN_DIR)$(EXE) $(INPUTDIR)$(INPUT1) 5
	@echo
	@echo
	# TESTING WITH "sample2.txt" #################
	@echo
	$(BIN_DIR)$(EXE) $(INPUTDIR)$(INPUT2) 5
	@echo
	@echo
	# TESTING WITH "sample3.txt" #################
	@echo
	$(BIN_DIR)$(EXE) $(INPUTDIR)$(INPUT3) 5
	@echo
	@echo
	# TESTING WITH "sample4.txt" #################
	@echo Note: Make sure to include -i flag to 
	@echo \ \ \ \ \ \ \"make test\" if the test stops here
	@echo
	$(BIN_DIR)$(EXE) $(INPUTDIR)$(INPUT4) 5
	@echo
	@echo
	# TESTING WITH "sample5.txt" #################
	@echo
	$(BIN_DIR)$(EXE) $(INPUTDIR)$(INPUT5) 5
	@echo
	@echo

Makefile: $(SRCS:.c=.d)

 # Pattern for .d files.
%.d:%.cc
	@echo Updating .d Dependency File
	@set -e; rm -f $@; \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

 # make backup (Backup Target)
backup: 
	@mkdir -p ~/backups; chmod 700 ~/backups
	@$(eval CURDIRNAME := $(shell basename `pwd`))
	@$(eval MKBKUPNAME := ~/backups/$(PROJECTNAME)-$(shell date +'%Y.%m.%d-%H:%M:%S').tar.gz)
	@echo
	@echo Writing Backup file to: $(MKBKUPNAME)
	@echo
	@-tar zcfv $(MKBKUPNAME) ../$(CURDIRNAME)
	@chmod 600 $(MKBKUPNAME)
	@echo
	@echo Done!


 # Include the dependency files
-include $(SRCS:.cc=.d)

