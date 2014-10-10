
ROOT_DIR=..\..\..
PROJECT=lp_util

# include base dependency checks
!include $(ROOT_DIR)\common.mak

install-files:
	copy .\$(CONFIGURATION)\$(PROJECT)$(STAT_LIB_EXT) $(LIB_DIR) /y
	copy .\StringUtil.h   $(INCLUDE_DIR) /y
	copy .\StringUtil.cpp $(SOURCE_DIR)  /y

#copy the DUT runtime DLLs to the Bin folder
install: update-bin install-files
