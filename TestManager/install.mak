
ROOT_DIR=..\..\..
PROJECT=TestManager

# include base dependency checks
!include $(ROOT_DIR)\common.mak

#copy the DUT runtime DLLs to the Bin folder
install: update-bin install-libs-hdr
