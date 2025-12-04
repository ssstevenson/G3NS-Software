#include <iostream>
#include <thread>  // For std::this_thread::sleep_for
#include <chrono>  // For std::chrono::milliseconds
#include <iomanip>

#include "fpgaIO.h"
#include "fpgaDefs.h"

#define REG_OFFSET     0x00000008    //Test Register

int main(int argc, char* argv[]) 
{
    unsigned int rval, wval;
    unsigned long mask =0;
    unsigned long iter = 0;
    
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << "name  iter \n";
        return 1;
    }

   std::string name  = argv[1];
   iter = static_cast <unsigned long > (std::stoi(argv[2])) ;
   if (argc == 4) mask = std::stoul(std::string(argv[3]), nullptr, 16);
   
  
   
   FpgaIO fpga(FpgaBase,FpgaMemSize);
   
   
   rval = fpga.readReg (FpgaRfManager::offset + FpgaRfManager::rfEnable);
   std::cout << "  RF ENABLE: " << std::hex << rval << std::endl;
   rval = fpga.readReg (FpgaPSU::offset + FpgaPSU::ctrl);
   std::cout << "  PSU ENABLE:  " << std::hex << rval << std::endl;
   fpga.writeReg(REG_OFFSET, mask);
   for ( unsigned long  i =0; i < iter; i++ )
   {
       rval = fpga.readReg(REG_OFFSET);
       std::cout << " Process " << name << " read reg: " <<  REG_OFFSET << " --->  Value = " << std::hex << rval << std::endl;
       if ( (rval & mask) != mask )
       {
           std::cout << " Process " << name << "[ mask = " << std::hex << std::setw(8) << std::setfill('0') << mask << " Read reg: " <<  REG_OFFSET << " ---> Value = " << std::hex << std::setw(8) << std::setfill('0')<< rval << std::endl;
       }
       wval  = mask + (i + 1);
       fpga.writeReg(REG_OFFSET, wval);
       std::cout << " Process " << name << " Write reg: " <<  REG_OFFSET << " ---> Value = " << std::hex << wval << std::endl;
       std::this_thread::sleep_for(std::chrono::milliseconds(5));  
       
   }
  

    return 0;
}

