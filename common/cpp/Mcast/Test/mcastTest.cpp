#include <iostream>
#include <chrono>
#include <ratio>
#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib>

#include "../McastCommon.h"

using namespace std;
const char *boosterModePath = "/empowerStatic/operationalMode.txt";

int  isSOSController(const std::string& filePath, bool &isController) 
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return -1; 
    }

    std::string content;
    std::getline(file, content); 
    file.close();

   isController = (content.find("SOS_CONTROLLER") != std::string::npos);
   return 0;
}

void action() 
{
	cout << "\n Controller Action Function Prior to Sending MCAST Called. This could be like blanking" << endl;
}

void mcastDone( bool pass, int nbBooster)
{
	cout << "Callback Function Called: " << (pass? " Passed": " failed" ) <<  "! The Number of  Boosters Executed the Command and  Responded  = " << nbBooster << endl;
	cout << " If passed It can undo the Action such as unblanking " << endl;
}
int main(int argc, char* argv[]) 
{
   int numberOfboosters = 0;
   bool isController = true; // by default
   CommandID cmdId = ALC_MODE;
   
  if (isSOSController(boosterModePath, isController ) < 0 ) {
      // The file does not exist
      // Read mode from Parameters
      if (argc < 3) {
          std::cerr << "Usage: " << argv[0] << " <system Mode>  <numberOfBooster\n";
          return 1;
      }
      std::string mode = argv[1];
      numberOfboosters = atoi(argv[2]);
     isController = (mode.find("SOS_CONTROLLER") != std::string::npos);
	 if ( argc == 4 ) {
		 cmdId =  static_cast <CommandID> (atoi(argv[3]) );
	 }
  }
  else {
      // file exist, we need one parameters
      if (argc < 2) {
          std::cerr << "Usage: " << argv[0] << " <numberOfBooster\n";
          return 1;
      }
  
      numberOfboosters = atoi(argv[1]);
  }
                     
  Mcast mcast= Mcast(MULTICAST_GROUP,COMMAND_PORT, RESPONSE_PORT,  isController, numberOfboosters);
  
  if (!isController) {
      mcast.boosterStart();
  }
  
  if (isController) {
      //  from and send command 
	   mcast.setAction(action);
	   mcast.setCallBack(mcastDone);
      CommandPacket cmd = mcast.formMcastCommand(cmdId, 0 );
      mcast.ControllerBcastCommand (&cmd, sizeof(cmd) );
  }

    return 0;
}

