/*! \mainpage Overview
 *
 * \section sect1  GEN 3 BOOSTER DEVELOPMENT
 * 
 *
 *  Link to Gen 3 SDD Overview Section:
 * 
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.qqdayy9ohom7
 *
 *
 * \subsection subsect1-1  -
 *
 *
 * ___
 *
 * ___
 *
 * 
 * \section sect2  SYSTEM MODULES
 * Listing of overviews of each of the document modules
 * We're adding placeholders for 'module' descritptions following the order of the SDD document
 * 
 * Hopefully to be filled out to completely cover the entire Gen 3 Project
 *
 *
 *
 *
 * ___
 * 
 * \subsection subsect2-1  Bundle Manager
 * Responsible Engineers:  Darren
 *
 * Module Description:
 * * The Bundle Manager (BM) is responsible for updating both software and firmware within the booster.  
 * * The BM is always the first software process to start up, and is a blocking process from the startup perspective.  All other processes must wait for BM to close.
 * * The BM categorizes application processes into "run levels".  Run Level 1 includes the Bundle Manager.  
 * See Process Manager for detailed explanation of run levels.
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.d5cl1gvtn1np
 *
 *
 * 
 * ___
 * 
 * \subsection subsect2-2  Software Heartbeat Monitor
 * Responsible Engineers:  Darren
 *
 * Module Description:
 * * The Software Heartbeat Monitor (SHM) is responsible for maintaining knowledge of the operational state of the individual software processes within the Gen3 amplifier.  
 * * The SHM is self-configuring, requiring no configuration parameters.  
 * * The SHM periodically reports the status of the current processes for display in the GUI, and for use by the Startup Manager..  Process start up, failure, and restart will result in a log message being generated.

 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.gijbh8683gbi
 *
 *
 * ___
 * 
 * \subsection subsect2-3  Process Manager
 * Responsible Engineers:  Darren
 *
 * Module Description:
 * * The Process Manager (PM) is responsible for gracefully starting up the various HPA processes. 
 * * The Process Manager is paired with the SHM for process monitoring.  
 * * In order to resolve process dependency issues and decrease system startup time, the Process Manager extracts active process status from the Software Heartbeat Monitor and uses a system of gated dependencies to decide when processes should be started, restarted, or killed.
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.mmizalh5vwts
 *
 * ___
 * 
 * 
 * \subsection subsect2-4  Message Factories
 * Responsible Engineers:  Darren & Michael
 *
 * Module Description:
 * * The message factories provide a single source for generation of process-to-process JSON messages.  
 * * This behavior design pattern is used to help keep messages uniform across a diversified development team by containing JSON generation operations to a singular standalone process.
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.c6qa4vlv4r2y
 * 
 *
 * ___
 * 
 * 
 * \subsection subsect2-5  Configuration Manager
 * Responsible Engineers:  Darren & Steve
 *
 * Module Description:
 * * The configuration manager stores key/value information for the system.  
 * * The configuration manager is divided into two functional behaviors:
 * * > Standard Data Configuration
 * * > Sticky Data Configuration
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.8nka0dz9q5os
 *
 * ___
 * 
 * 
 * \subsection subsect2-6  System Logger
 * Responsible Engineers:  Darren
 *
 * Module Description:
 * * The system logger provides a sink mechanism for all software processes to output messages.  
 * * Log messages shall be time-stamped at the origin by the process before the message is sent to logger.  
 * * In addition to a user message, log messages also include the origin process name, and a criticality level.  Valid criticality levels are "verbose", "debug", "info", "warn", and "critical".
 * 
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.m5q26biuev1k
 *
 * ___
 * 
 * 
 * \subsection subsect2-7  Command Processor
 * Responsible Engineers:  Darren
 *
 * Module Description:
 * * The Command Processor (CP) is responsible for collecting incoming requests from a variety of ZMQ sockets, and arbitrating them based on the current command source.
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.i6stz8i627s8
 *
 * ___
 * 
 * 
 * \subsection subsect2-8  Status Processor
 * Responsible Engineers:  Darren & Michael
 *
 * Module Description:
 * * The Status Processor (SP) has two main roles:
 * * > Providing a sink & rebroadcast mechanism for command responses.
 * * > Providing an interface to caching amplifier telemetry points into Redis.
 * * The SP provides a ingressive interface (aka, a message sink) for status updates that are generated by the core worker processes.  This behavior can be seen in the Machine Message Flow diagram.

 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.urt5qib1eeil
 *
 * ___
 * 
 * 
 * \subsection subsect2-9  System Manager
 * Responsible Engineers:  Darren, Michael & Steve
 *
 * Module Description:
 * * The System Manager provides a variety of functionality with respect to the overall system.
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.wun70ykcj7lq
 *
 * ___
 * 
 * 
 * \subsection subsect2-10  Serial Hardware Interface
 * Responsible Engineers:   Darren & Michael
 *
 * Module Description:
 * * The Serial Hardware Interface (SHI) consists of an array of separate processes, each of which offer access to a given UART.
 * * The calling process utilizes a Request/Reply ZMQ socket to interface the SHI process.  
 * * The request includes the UART configuration details, such as baud rate, start/stop bits, and parity. 
 * * The SHI process is responsible for configuring the UART per the request, then transferring the request data to the virtual UART hardware.  (See Virtual UARTs).

 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.qzujc550x17c
 *
 * ___
 * 
 * 
 * \subsection subsect2-11  FPGA Hardware Interface
 * Responsible Engineers:   Steve & Michael
 *
 * Module Description:
 * * The FPGA Hardware Interface (FHI) provides low-level FPGA register access, and supports low-level "get" and "set" operations.
 * *  The FHI acts as a transparent bridge, offering a request/reply ZMQ socket to any worker processes which requires FPGA interaction.

 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.136kx7a6yngp
 *
 * ___
 * 
 * 
 * \subsection subsect2-12  Factory Proxy Interface
 * Responsible Engineers:   Michael & Steve
 *
 * Module Description:
 * * The Factory ProxyInterface (FPI) serves as a relay process between the external LAN to a variety of internal Req-Reply sockets.

 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.nj9vr5i8bp00
 *
 * ___
 * 
 * 
 * \subsection subsect2-13  RF Manager
 * Responsible Engineers:   Steve & Michael
 *
 * Module Description:
 * * The RF Manager serves as the primary means to control the RF logic within the FPGA.
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.89yrtzbf9mkc
 *
 * ___
 * 
 * 
 * \subsection subsect2-14  Module Manager
 * Responsible Engineers:   Darren & Michael
 *
 * Module Description:
 * * The Module Manager (MM) is responsible for managing communication with the PMOD bus, aka the palettes and drivers internal to the booster chassis.
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.8rjck845yfzz
 *
 * ___
 * 
 * 
 * \subsection subsect2-15  PSU Manager
 * Responsible Engineers:   Darren & Michael
 *
 * Module Description:
 * * The PSU Manager is responsible for maintenance of all power supply modules within a standalone system. 
 * * Power supply modules are most often constructed internally to the amplifier chassis.
 * * There are three variations of power supplies, GE Galaxy, Martek 400Hz, and DC-to-DC modules. 
 * * The GE Galaxy modules are expected to be the primary use case for Gen3 boosters.
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.4gx683xu3ows
 *
 * ___
 * 
 * \subsection subsect2-16  Environmental Manager
 * Responsible Engineers:  Steve & Darren
 *
 * The Environmental Manager main duties are:
 * * Collect temperature data of various devices in system via the Status Processor.
 * * Set the fan speed (duty cycle & fan enable) based on temperature thresholds.
 * * Generate temperature fault message if some monitored value is out of range.
 *
 *  Link to Gen 3 SDD Environmental Manager Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.1rwov2hbv38q
 *
 * ___
 * 
 * 
 * \subsection subsect2-17  Fault Manager
 * Responsible Engineers:   Michael & Steve / Marc
 *
 * Module Description:
 * * Faults can be described as System faults or Module faults. In general, system faults are considered faults that are at system level and module faults occur at module level.  A fault in a module will cause a system fault.  A system fault will not necessarily generate a module fault.

 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.e71ygly4tw89
 *
 * ___
 * 
 * 
 * \subsection subsect2-18  M2M Manager
 * Responsible Engineers:   Marc & Michael
 *
 * Module Description:
 * *  There are three flavors in M2M that could potentially run in the system depending on the configuration:
 * * 1-  M2M Over TCP
 * * 2-  M2M Over UDP
 * * 3- M2M Over Serial
 * * Each of these M2M are exclusive, meaning only shall be running at any  time. 

 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.g20vkuu8nlc1
 *
 * ___
 * 
 * \subsection subsect2-19  Switch Manager
 * Responsible Engineers:   Darren & Michael
 *
 * Module Description:
 * * There are several aspects to this module. 
 * * This module will need to first off, control RF switches for band selection, as well as T/R capabilities. 
 * * This module will also be responsible for determining which PA Enable line is active and select the fine control Coupling Factors.

 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.t4xlnjezge72
 *
 * ___
 * 
 * 
 * \subsection subsect2-20  Upgrade Manager
 * Responsible Engineers:   Darren & Michael
 *
 * Module Description:
 * * TBD
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.od5fwz4pavjt
 * ___
 * 
 * 
 * \subsection subsect2-21  Built In Test
 * Responsible Engineers:   Marc & Michael
 *
 * Module Description:
 * * TBD
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.gfpu57z46so4
 *
 * ___
 * 
 * 
 * \subsection subsect2-22  Automated Test Equipment
 * Responsible Engineers:   Darren & Michael
 *
 * Module Description:
 * * TBD
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.mbsk3oez59uj
 *
 * ___
 * 
 * 
 * \subsection subsect2-23  Zynq Drivers
 * Responsible Engineers:   Marc & Michael
 *
 * Module Description:
 * * TBD
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.v53sdd8p03d0
 *
 * ___
 * 
 * 
 * 
 * \subsection subsect2-24  FPGA
 * Responsible Engineers:   Michael & Shahnam
 *
 * Module Description:
 * * TBD
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.yvjii6kxnh9i
 *
 * ___
 * 
 * 
 * 
 * \subsection subsect2-25  SOS Inter-Chassis Comms (Discovery)
 * Responsible Engineers:   Marc
 *
 * Module Description:
 * * TBD
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.aifai4wt139
 *
 * ___
 * 
 * 
 * 
 * \subsection subsect2-26  Fast Boot Manager
 * Responsible Engineers:   Darren & Michael
 *
 * Module Description:
 * * TBD
 *
 *  Link to Gen 3 SDD Module Section:
 *
 *   https://docs.google.com/document/d/1OOwac7lC0E2hpeZu_kmMkxLqWg3iyZODJPP_FFIlVrc/edit#heading=h.e9z5pc5ggktw
 *
 * ___
 * 
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
