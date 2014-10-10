
ROOT_DIR=..\..\..
PROJECT=IQmeasure

# include base dependency checks
!include $(ROOT_DIR)\common.mak

setup: update-xel

#copy the DUT runtime DLLs to the Bin folder
install: update-bin install-libs-hdr
