
ROOT_DIR=..\..\..
PROJECT=WiFi_Test

# include base dependency checks
!include $(ROOT_DIR)\common.mak

install-files:
	copy .\WiFi_Test_Internal.h         $(INCLUDE_DIR) /y
	copy .\path_loss.csv                $(BIN_DIR) /y
	copy .\Multi_Segment_Waveform.ini   $(BIN_DIR) /y

#copy the DUT runtime DLLs to the Bin folder
install: update-bin install-libs-hdr install-files
