
ROOT_DIR=..\..\..
BIN_DIR=$(ROOT_DIR)\Bin_win\$(CONFIGURATION)
LIB_DIR=$(ROOT_DIR)\Lib\$(CONFIGURATION)
PROJECT=LP_Dut_11ac_A6200

#copy the DUT runtime DLLs to the Bin folder
install:
	if exist .\$(CONFIGURATION) Copy  .\$(CONFIGURATION)\$(PROJECT).dll  $(BIN_DIR) /y

