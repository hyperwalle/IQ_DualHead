
ROOT_DIR=..\..\..
PROJECT=WiFi_MiMo_Test

# include base dependency checks
!include $(ROOT_DIR)\common.mak

install-files:
	copy .\WiFi_MiMo_Test_Internal.h         $(INCLUDE_DIR) /y

#copy the DUT runtime DLLs to the Bin folder
install: update-bin install-libs-hdr install-files
