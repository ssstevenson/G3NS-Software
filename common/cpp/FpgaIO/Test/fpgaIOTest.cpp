#include <iostream>
#include <thread>  // For std::this_thread::sleep_for
#include <chrono>  // For std::chrono::milliseconds
#include <iomanip>

#include <FpgaIO/fpgaIO.h>
#include <FpgaIO/fpgaDefs.h>

#define REG_OFFSET     0x00000008    //Test Register


void TestReadWrite( int iter, FpgaIO &fpga)
{
    unsigned int wval = 1;
    bool err = false;


   while (iter--) {
       for ( int j=0; j< 31; j++ )
       {
            fpga.writeReg(REG_OFFSET, wval);
            unsigned int rval = fpga.readReg(REG_OFFSET);
            if ( rval != wval )
            {
               std::cout << " Read-Write Failed  " << " wrote : " <<  std::hex << wval << " Read  : " <<  std::hex << rval << std::endl;
               err= true;
            }
           wval <<= j;

       }
        wval |=  static_cast<unsigned int> (iter);
   }

}
int main(int argc, char* argv[])
{
    unsigned int rval=0, wval = 0;
    unsigned int mask =0;
    unsigned long iter = 0;
    std::string name;;


    if (argc >=  3) {
        iter = static_cast <unsigned long > (std::stoi(argv[2])) ;
        name  = argv[1];

        if (argc == 4) {
            mask =  static_cast<unsigned int> (std::stoul(std::string(argv[3]), nullptr, 16));
        }
    }
    else {
        wval = static_cast <unsigned int > (std::stoi(argv[1])) ;

    }

   std::cout << "  wval = " << std::hex << wval << std::endl;


   std::cout << "  PFpga IO test started  " << std::hex << rval << std::endl;
   FpgaIO fpga(FpgaBase,FpgaMemSize);



   if ( wval == 0 )
   {
       rval = fpga.readReg (FpgaRfManager::offset + FpgaRfManager::rfEnable);
       std::cout << "  RF ENABLE: " << std::hex << rval << std::endl;
       rval = fpga.readReg (FpgaPSU::offset + FpgaPSU::ctrl);
       std::cout << "  PSU ENABLE:  " << std::hex << rval << std::endl;
       // We are testing multiple processes at same time..
       // run this program many tines
       TestReadWrite(iter,fpga);
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
   }
   else
   {
       // Testing Read --- Write
         rval = fpga.readReg(REG_OFFSET);
         std::cout << " read reg: " <<  REG_OFFSET << " --->  Value = " << std::hex << rval << std::endl;
        fpga.writeReg(REG_OFFSET, wval);
        std::cout << " write to reg: " << REG_OFFSET << " --->  Value = " << std::hex << wval << std::endl;
         rval = fpga.readReg (FpgaRfManager::offset + FpgaRfManager::rfEnable);
         wval = 0;
         if (rval == 0  ) wval =1;
          std::cout << " Process " << name << " Write to  reg: " <<  std::hex <<  (FpgaRfManager::offset + FpgaRfManager::rfEnable)  << " --->  Value = " << std::hex << wval << std::endl;
         fpga.writeReg (FpgaRfManager::offset + FpgaRfManager::rfEnable, wval);

   }


    return 0;
}

