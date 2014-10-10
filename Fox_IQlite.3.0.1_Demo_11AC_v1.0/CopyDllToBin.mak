
ROOT_DIR=..\..\..
BIN_DIR_NAME=Bin_win
PROJECT=IQlite_Demo_11AC

BIN_DIR=$(ROOT_DIR)\$(BIN_DIR_NAME)

EXE_EXT=.exe

	
		
#make sure the bin folder exists
$(BIN_DIR):
	mkdir $(BIN_DIR)

	
install-files:	
	copy .\$(CONFIGURATION)\$(PROJECT)$(EXE_EXT) 	$(BIN_DIR) /y

install: install-files

