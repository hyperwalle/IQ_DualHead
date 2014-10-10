
ROOT_DIR=..\..\..
PROJECT=FM_Test

# include base dependency checks
!include $(ROOT_DIR)\common.mak

install-files:
	copy .\FM_Test_Internal.h           $(INCLUDE_DIR) /y
	copy .\path_loss_FM.csv             $(BIN_DIR) /y

#copy the DUT runtime DLLs to the Bin folder
install: update-bin install-libs-hdr install-files
