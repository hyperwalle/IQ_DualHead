
ROOT_DIR=..\..\..
PROJECT=IQlite_Timer

# include base dependency checks
!include $(ROOT_DIR)\common.mak

#copy the DUT runtime DLLs to the Bin folder
install: update-bin install-build
