
ROOT_DIR=..\..\..
BIN_DIR=$(ROOT_DIR)\Bin_win\$(CONFIGURATION)
LIB_DIR=$(ROOT_DIR)\Lib\$(CONFIGURATION)
PROJECT=LP_Dut_11ac_R6200v2

#copy the DUT runtime DLLs to the Bin folder
install:
	if exist .\$(CONFIGURATION) Copy  .\$(CONFIGURATION)\$(PROJECT).dll  $(BIN_DIR) /y

