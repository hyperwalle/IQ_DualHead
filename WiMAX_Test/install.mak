
ROOT_DIR=..\..\..
PROJECT=WiMAX_Test

# include base dependency checks
!include $(ROOT_DIR)\common.mak

install-files:
	copy .\WiMAX_Test_Internal.h         $(INCLUDE_DIR) /y
	copy .\path_loss_WiMAX.csv           $(BIN_DIR) /y
	copy .\RX_Path_Loss_WiMAX.csv        $(BIN_DIR) /y
	copy .\MaskRequirement.csv           $(BIN_DIR) /y

#copy the DUT runtime DLLs to the Bin folder
install: update-bin install-libs-hdr install-files
