
ROOT_DIR=..\..\..\..
PROJECT=IQmeasure_IQapi_Legacy

# include base dependency checks
!include $(ROOT_DIR)\common.mak

#copy the DUT runtime DLLs to the Bin folder
install: update-bin install-dll
