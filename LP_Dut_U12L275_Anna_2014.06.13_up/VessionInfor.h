//This head file is about LP_Dut project¡]DUT control dll¡^version information
/*!
 *Your can change version information at LP_Dut.cpp,the parameter is:const char  *g_LP_Dut_Version
 *
 *Foxconn WiFi DUT Control version : LP_1.0.1 (2011-06-25)
 *1.add a socket used for telnet.please find more infor at PeerSocket.cpp							//By DX/FH
 *2.add funtion:bool SendSocketCmd(char *socket,char *cmd, char *ret, int timeout)					//By DX/FH
 *
 *Foxconn WiFi DUT Control version : LP_1.0.2 (2011-06-27)
 *1.check socket recv funtion(all send command is timeout(1000ms) break loop,test time is so long)
 *  change SendSocketCmd funtion to bool SendSocketCmd(char *cmd, char *ret, int timeout)			//By DX/FH
 *2.use SendSocketCmd funtion add TX test command into TX test funtion								//By DX/FH
 *
 *Foxconn WiFi DUT Control version : LP_1.0.3 (2011-06-28)
 *1.add read command prefix from LP_DUT_setup.ini to readue same code								//By FH,check by DX
 *2.add show debug information flag																	//By FH,check by DX
 *
 *Foxconn WiFi DUT Control version : LP_1.0.4 (2011-06-29)
 *1.add struct SameDut,used for mark same dut.														//By FH,check by DX
 *2.change TX command list
 *3.add RX test command list
 *
 *Foxconn WiFi DUT Control version : LP_1.0.5 (2011-07-07)
 *1.add struct SameDut,used for mark same dut.														//By FH,check by DX
 *2.Try LitePoint owen's command,try duck command
 *
 *Foxconn WiFi DUT Control version : LP_1.0.6 (2011-07-08)
 *1.Try LitePoint owen's command,change RX test key word											//By FH,check by DX
 *2.add RX by ant test
 *
 *Foxconn WiFi DUT Control version : LP_1.0.7 (2011-09-09)
 *1.change RX command(command for kim's DUCk's command)
 *2.add some command(command from peter's mail)
 *
 */