#include <iostream>
#include <iomanip>
#include <FpgaIO/fpgaIO.h>
#include <FpgaIO/fpgaDefs.h>
#include <hardwareInterface/hardwareInterface.h>

#include "cxxopts.hpp"

int main(int argc, const char* argv[])
{
    using namespace empower;
    using reg_t = helpers::types::reg_t;

    const auto wordMask = [](const auto val, const auto bytes)
    {
        using val_type = decltype(val);

        if constexpr(std::is_integral_v<val_type>)
        {
            return val & ~(static_cast<val_type>(bytes) - 1);
        }
        exit(1);
    };

    try
    {
        cxxopts::Options options(argv[0], " - example command line options");
        options
          .positional_help("[optional args]")
          .show_positional_help();

        options
            .set_width(80)
            .set_tab_expansion()
            .allow_unrecognised_options()
            .add_options()
            ("c,cmd", "Command to Execute", cxxopts::value<std::string>())
            ("d,dump", "Dump a Range.  Expects Start Address and End Address or Address and Range")
            ("a,addr", "Address", cxxopts::value<reg_t>())
            ("s,start", "Start Address", cxxopts::value<reg_t>())
            ("e,end", "End Address", cxxopts::value<reg_t>())
            ("r,range", "Range", cxxopts::value<reg_t>())
            ("positional",
                "Positional arguments: these are the arguments that are entered "
                "without an option", cxxopts::value<std::vector<std::string>>())
            ("h,help", "Print help")
        ;

        const auto helpPrint = [&options](const int exitVal = 0)
        {
            std::cout << options.help({"", "Group"}) << std::endl;
            exit(exitVal);
        };

        auto result = options.parse(argc, argv);

        if(result.count("dump"))
        {
            reg_t addr, len, startOff, stopOff, pageAddr, pageCnt, pageLen;
            const reg_t pageSize{hardwareInterface::getPageSize()};

            if(result.count("start") and result.count("end"))
            {
                addr = result["start"].as<reg_t>();
                len = result["end"].as<reg_t>() - addr;
            }
            else if(result.count("addr") and result.count("range"))
            {
                addr = result["addr"].as<reg_t>();
                len = result["range"].as<reg_t>();
            }
            else
            {
                helpPrint(1);
            }

            addr = wordMask(addr, 4);

            pageAddr = wordMask(addr, pageSize);
            pageCnt = (len / pageSize) + 1;
            pageLen = pageCnt * pageSize;

            startOff = addr - pageAddr;
            stopOff = startOff + len;

            hardwareInterface hwIf{pageAddr, pageLen};

            if(const auto data{hwIf.readRangeAddr(startOff, stopOff)}; data.has_value())
            {
                for(const auto& [key, val]: data.value())
                {
                    std::cout << std::hex << "0x" << std::setw(16) << std::setfill('0') << std::uppercase << key <<
                        std::nouppercase << ",0x" << std::setw(8) << std::setfill('0') << std::uppercase << val <<
                        std::nouppercase << std::dec << "\n";
                }

                exit(0);
            }
        }

        helpPrint();
    }
    catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

    return 0;
}
