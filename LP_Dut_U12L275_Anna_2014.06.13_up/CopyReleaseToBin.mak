all:     ..\..\Bin \
	 ..\..\Bin\LP_Dut.DLL \
         ..\..\Bin\LP_Dut_setup.ini \
	 CopyCurrentImportToBin
	 
# make sure the Bin folder exists
..\..\Bin:
	mkdir ..\..\Bin
	
#copy the release version of DLL to the Bin folder

..\..\Bin\LP_Dut.DLL: .\Release\LP_Dut.DLL
	copy .\Release\LP_Dut.DLL ..\..\Bin\LP_Dut.DLL /y

#copy the LP_Dut Setup file to the Bin folder
..\..\Bin\LP_Dut_setup.ini: .\LP_Dut_setup.ini
	copy .\LP_Dut_setup.ini ..\..\Bin\LP_Dut_setup.ini /y
	
CopyCurrentImportToBin:
	if exist ..\..\Import xcopy ..\..\Import ..\..\Bin /e /y