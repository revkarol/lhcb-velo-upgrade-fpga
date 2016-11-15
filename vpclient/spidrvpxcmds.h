#ifndef SPIDRVPXCMDS_H
#define SPIDRVPXCMDS_H

// Command identifiers in messages to (and from) the SPIDR-TPX3 module
// (trying to remain compatible with SPIDR-TPX3 and SPIDR-MPX3)
#define CMD_NOP                0x000

// General: module
#define CMD_GET_SOFTWVERSION   0x901
#define CMD_GET_FIRMWVERSION   0x902

#define CMD_RESET_MODULE       0x907
#define CMD_SET_BUSY           0x908
#define CMD_CLEAR_BUSY         0x909
#define CMD_SET_LOGLEVEL       0x90A
#define CMD_DISPLAY_INFO       0x90B
#define CMD_SET_TIMEOFDAY      0x90C
#define CMD_GET_DEVICECOUNT    0x90D

#define CMD_GET_CHIPBOARDID    0x90F

// Configuration: devices
#define CMD_GET_DEVICEID       0x110
#define CMD_GET_IPADDR_SRC     0x112
#define CMD_SET_IPADDR_SRC     0x113
#define CMD_GET_IPADDR_DEST    0x114
#define CMD_SET_IPADDR_DEST    0x115
#define CMD_GET_DEVICEPORT     0x116
#define CMD_GET_SERVERPORT     0x117

#define CMD_SET_SERVERPORT     0x119
#define CMD_GET_DAC            0x11A
#define CMD_SET_DAC            0x11B
#define CMD_SET_DACS_DFLT      0x11F

#define CMD_RESET_DEVICE       0x124
#define CMD_RESET_DEVICES      0x125

// Configuration: pixels
#define CMD_SET_PIXCONF        0x22A
#define CMD_SET_SUPERPIXCONF   0x22C
#define CMD_GET_PIXCONF        0x22D
#define CMD_RESET_PIXELS       0x22E
#define CMD_GET_SUPERPIXCONF   0x22F

// Configuration: devices (continued)
#define CMD_SELECT_DACOUT      0x338

// Trigger
#define CMD_GET_TRIGCONFIG     0x440
#define CMD_SET_TRIGCONFIG     0x441
#define CMD_AUTOTRIG_START     0x442
#define CMD_AUTOTRIG_STOP      0x443

// Monitoring
#define CMD_GET_ADC            0x548
#define CMD_GET_REMOTETEMP     0x549
#define CMD_GET_LOCALTEMP      0x54A
#define CMD_GET_AVDD           0x54B
#define CMD_GET_DVDD           0x54C
#define CMD_GET_AVDD_NOW       0x54D
#define CMD_GET_SPIDR_ADC      0x54E
#define CMD_GET_DVDD_NOW       0x54F

// Trigger (continued)
#define CMD_GET_SHUTTERSTART   0x554
#define CMD_GET_SHUTTEREND     0x555
#define CMD_GET_EXTSHUTTERCNTR 0x556
#define CMD_GET_SHUTTERCNTR    0x557
#define CMD_RESET_COUNTERS     0x558

// Monitoring (continued)
#define CMD_GET_FPGATEMP       0x568
#define CMD_GET_FANSPEED       0x569
#define CMD_SET_FANSPEED       0x56A
#define CMD_GET_VDD            0x56C
#define CMD_GET_VDD_NOW        0x56D

// Configuration: non-volatile onboard storage
#define CMD_STORE_ADDRPORTS    0x670
#define CMD_STORE_DACS         0x671
#define CMD_STORE_REGISTERS    0x672
#define CMD_STORE_PIXCONF      0x673
#define CMD_ERASE_ADDRPORTS    0x674
#define CMD_ERASE_DACS         0x675
#define CMD_ERASE_REGISTERS    0x676
#define CMD_ERASE_PIXCONF      0x677
#define CMD_VALID_ADDRPORTS    0x678
#define CMD_VALID_DACS         0x679
#define CMD_VALID_REGISTERS    0x67A
#define CMD_VALID_PIXCONF      0x67B
#define CMD_STORE_STARTOPTS    0x67C
#define CMD_GET_STARTOPTS      0x67D

// Other
#define CMD_GET_VPXREG         0x780
#define CMD_SET_VPXREG         0x781
#define CMD_GET_SPIDRREG       0x783
#define CMD_SET_SPIDRREG       0x784
#define CMD_SET_CHIPBOARDID    0x785

// Short strings describing the commands
// (indexed by the lower byte of the command identifier)
static const char *CMD_STR[] =
  {
    "<no operation>   ", // 0x900
    "GET_SOFTWVERSION ", // 0x901
    "GET_FIRMWVERSION ", // 0x902
    "-----",             // 0x903
    "-----",             // 0x904
    "-----",             // 0x905
    "-----",             // 0x906
    "RESET_MODULE     ", // 0x907
    "SET_BUSY         ", // 0x908
    "CLEAR_BUSY       ", // 0x909
    "SET_LOGLEVEL     ", // 0x90A
    "DISPLAY_INFO     ", // 0x90B
    "SET_TIMEOFDAY    ", // 0x90C
    "GET_DEVICECOUNT  ", // 0x90D
    "-----",             // 0x90E
    "GET_CHIPBOARDID  ", // 0x90F

    "GET_DEVICEID     ", // 0x110
    "-----",             // 0x111
    "GET_IPADDR_SRC   ", // 0x112
    "SET_IPADDR_SRC   ", // 0x113
    "GET_IPADDR_DEST  ", // 0x114
    "SET_IPADDR_DEST  ", // 0x115
    "GET_DEVICEPORT   ", // 0x116
    "GET_SERVERPORT   ", // 0x117
    "-----",             // 0x118
    "SET_SERVERPORT   ", // 0x119
    "GET_DAC          ", // 0x11A
    "SET_DAC          ", // 0x11B
    "-----",             // 0x11C
    "-----",             // 0x11D
    "-----",             // 0x11E
    "SET_DACS_DFLT    ", // 0x11F
    "-----",             // 0x120
    "-----",             // 0x121
    "-----",             // 0x122
    "-----",             // 0x123
    "RESET_DEVICE     ", // 0x124
    "RESET_DEVICES    ", // 0x125
    "-----",             // 0x126
    "-----",             // 0x127
    "-----",             // 0x128
    "-----",             // 0x129

    "SET_PIXCONF      ", // 0x22A
    "-----",             // 0x22B
    "SET_SUPERPIXCONF ", // 0x22C
    "GET_PIXCONF      ", // 0x22D
    "RESET_PIXELS     ", // 0x22E
    "GET_SUPERPIXCONF ", // 0x22F

    "-----",             // 0x330
    "-----",             // 0x331
    "-----",             // 0x332
    "-----",             // 0x333
    "-----",             // 0x334
    "-----",             // 0x335
    "-----",             // 0x336
    "-----",             // 0x337
    "SELECT_DACOUT    ", // 0x338
    "-----",             // 0x339
    "-----",             // 0x33A
    "-----",             // 0x33B
    "-----",             // 0x33C
    "-----",             // 0x33D
    "-----",             // 0x33E
    "-----",             // 0x33F

    "GET_TRIGCONFIG   ", // 0x440
    "SET_TRIGCONFIG   ", // 0x441
    "AUTOTRIG_START   ", // 0x442
    "AUTOTRIG_STOP    ", // 0x443
    "-----",             // 0x444
    "-----",             // 0x445
    "-----",             // 0x446
    "-----",             // 0x447

    "GET_ADC          ", // 0x548
    "GET_REMOTETEMP   ", // 0x549
    "GET_LOCALTEMP    ", // 0x54A
    "GET_AVDD         ", // 0x54B
    "GET_DVDD         ", // 0x54C
    "GET_AVDD_NOW     ", // 0x54D
    "GET_SPIDR_ADC    ", // 0x54E
    "GET_DVDD_NOW     ", // 0x54F

    "-----",             // 0x550
    "-----",             // 0x551
    "-----",             // 0x552
    "-----",             // 0x553

    "GET_SHUTTERSTART ", // 0x554
    "GET_SHUTTEREND   ", // 0x555
    "GET_EXTSHUTTRCNTR", // 0x556
    "GET_SHUTTERCNTR  ", // 0x557
    "RESET_COUNTERS   ", // 0x558
    "-----",             // 0x559
    "-----",             // 0x55A
    "-----",             // 0x55B
    "-----",             // 0x55C
    "-----",             // 0x55D
    "-----",             // 0x55E
    "-----",             // 0x55F
    "-----",             // 0x560
    "-----",             // 0x561
    "-----",             // 0x562
    "-----",             // 0x563
    "-----",             // 0x564
    "-----",             // 0x565
    "-----",             // 0x566
    "-----",             // 0x567

    "GET_FPGATEMP     ", // 0x568
    "GET_FANSPEED     ", // 0x569
    "SET_FANSPEED     ", // 0x56A
    "-----",             // 0x56B
    "GET_VDD          ", // 0x56C
    "GET_VDD_NOW      ", // 0x56D
    "-----",             // 0x56E
    "-----",             // 0x56F

    "STORE_ADDRPORTS  ", // 0x670
    "STORE_DACS       ", // 0x671
    "STORE_REGISTERS  ", // 0x672
    "STORE_PIXCONF    ", // 0x673
    "ERASE_ADDRPORTS  ", // 0x674
    "ERASE_DACS       ", // 0x675
    "ERASE_REGISTERS  ", // 0x676
    "ERASE_PIXCONF    ", // 0x677
    "VALID_ADDRPORTS  ", // 0x678
    "VALID_DACS       ", // 0x679
    "VALID_REGISTERS  ", // 0x67A
    "VALID_PIXCONF    ", // 0x67B
    "STORE_STARTOPTS  ", // 0x67C
    "GET_STARTOPTS    ", // 0x67D
    "-----",             // 0x67E
    "-----",             // 0x67F

    "GET_VPXREG       ", // 0x780
    "SET_VPXREG       ", // 0x781
    "-----",             // 0x782
    "GET_SPIDRREG     ", // 0x783
    "SET_SPIDRREG     ", // 0x784
    "SET_CHIPBOARDID  "  // 0x785
  };

// Reply bit: set in the reply message in the command identifier
#define CMD_REPLY            0x00010000

// No-reply bit: set in the command message in the command identifier
// indicating to the SPIDR server that no reply is expected
// (to speed up certain operations, such as pixel configuration uploads)
#define CMD_NOREPLY          0x00080000

#define CMD_MASK             0x0000FFFF

// Error identifiers in replies from the SPIDR module
// (in first byte; 2nd to 4th byte can be used for additional info)
#define ERR_NONE             0x00000000
#define ERR_UNKNOWN_CMD      0x00000001
#define ERR_MSG_LENGTH       0x00000002
#define ERR_SEQUENCE         0x00000003
#define ERR_ILLEGAL_PAR      0x00000004
#define ERR_NOT_IMPLEMENTED  0x00000005
#define ERR_VPX_HARDW        0x00000006
#define ERR_ADC_HARDW        0x00000007
#define ERR_DAC_HARDW        0x00000008
#define ERR_MON_HARDW        0x00000009
#define ERR_FLASH_STORAGE    0x0000000A
#define ERR_MONITOR          0x0000000B

// Short strings describing the errors
// (indexed by the lower byte of the error identifier)
static const char *ERR_STR[] =
  {
    "no error",
    "ERR_UNKNOWN_CMD",
    "ERR_MSG_LENGTH",
    "ERR_SEQUENCE",
    "ERR_ILLEGAL_PAR",
    "ERR_NOT_IMPLEMENTED",
    "ERR_VPX_HARDW",
    "ERR_ADC_HARDW",
    "ERR_DAC_HARDW",
    "ERR_MON_HARDW",
    "ERR_FLASH_STORAGE"
  };

#endif // SPIDRVPXCMDS_H
