
ROOT_DIR=..\..\..\..
PROJECT=IQmeasure_SCPI

# include base dependency checks
!include $(ROOT_DIR)\common.mak

setup:  update-xel

# copy the DUT runtime DLLs to the Bin folder
install: update-bin install-dll
