all:     ..\..\BinDebug \
	 ..\..\BinDebug\LP_Dut.DLL \
         ..\..\BinDebug\LP_Dut_setup.ini \
	 CopyCurrentImportToBin
	 

#make sure the debug foler exists under the bin folder
..\..\BinDebug:
	mkdir ..\..\BinDebug
	
#copy the debug version of DLL to the bin\debug folder

..\..\BinDebug\LP_Dut.DLL: .\debug\LP_Dut.DLL
	copy .\debug\LP_Dut.DLL ..\..\BinDebug\LP_Dut.DLL /y

#copy the LP_Dut Setup file to the Bin\Debug folder
..\..\BinDebug\LP_Dut_setup.ini: .\LP_Dut_setup.ini
	copy .\LP_Dut_setup.ini ..\..\BinDebug\LP_Dut_setup.ini /y


CopyCurrentImportToBin:
	if exist ..\..\ImportDebug Copy ..\..\ImportDebug\*.* ..\..\BinDebug /y
