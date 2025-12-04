#ifndef SIGGEN_H_
#define SIGGEN_H_

#include <string>

#include "scpiinstrument.h"

namespace empower
{
    /**
     * @brief      This class describes a Signal Generator interface.
     */
    class siggen:
        public scpiInstrument
    {
    public:
        siggen(const std::string& target);

        bool outputEnable(const bool on);
        bool setFreq(const std::uint32_t freq, const std::string& units);
        bool setAmp(const double ampl);
    };
}

#endif
