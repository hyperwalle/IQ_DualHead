
ROOT_DIR=..\..\..
PROJECT=WiFi_11ac_Test

# include base dependency checks
!include $(ROOT_DIR)\common.mak

install-files:
	copy .\WiFi_11ac_Test.h $(INCLUDE_DIR) /y
	copy .\WiFi_11ac_Test_Internal.h $(INCLUDE_DIR) /y
	copy .\path_loss.csv $(BIN_DIR) /y

#copy the DUT runtime DLLs to the Bin folder
install: update-bin install-libs-hdr install-files
