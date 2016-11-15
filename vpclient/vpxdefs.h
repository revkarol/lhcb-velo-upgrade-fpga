/* ----------------------------------------------------------------------------
File   : vpxdefs.h

Descr  : Definitions for Velopix devices.

History:
29MAR2016; HenkB; Created.
---------------------------------------------------------------------------- */
#ifndef VPXDEFS_H_
#define VPXDEFS_H_

//#define VPX_BROADCAST_ADDR        0x7FFF
#define VPX_BROADCAST_ADDR          0x0000

// ----------------------------------------------------------------------------
// Velopix register definitions

#define VPXREG_DACOUT               0x0304
#define VPXREG_DAC                  0x0400
#define VPXREG_CHIPID               0x0530

// ----------------------------------------------------------------------------
// Velopix DACs

// Number of DACs
#define VPX_DAC_COUNT               14

// Velopix DAC select codes
#define VPX_DAC_IPREAMP             1
#define VPX_DAC_IKRUM               2
#define VPX_DAC_IDISC               3
#define VPX_DAC_IPIXELDAC           4
#define VPX_DAC_VTPCOARSE           5
#define VPX_DAC_VTPFINE             6
#define VPX_DAC_VPREAMP_CAS         7
#define VPX_DAC_VFBK                8
#define VPX_DAC_VTHR                9
#define VPX_DAC_VCAS_DISC           10
#define VPX_DAC_VIN_CAS             11
#define VPX_DAC_VREFSLVS            12
#define VPX_DAC_IBIASSLVS           13
#define VPX_DAC_RES_BIAS            14 // NB: not used for this DAC, but a VOUT

// ----------------------------------------------------------------------------
// Velopix monitoring voltages

// Number of monitoring voltages
#define VPX_VOUT_COUNT              12

// Monitoring voltage output select code
#define VPX_VOUT_BAND_GAP_TEMP      14
#define VPX_VOUT_BAND_GAP           15
#define VPX_VOUT_BIAS_DAC_CAS       16
#define VPX_VOUT_BIAS_DAC           17
#define VPX_VOUT_GNDA_1_3           18
#define VPX_VOUT_VDDA_2_3           19
#define VPX_VOUT_GND_1_3            20
#define VPX_VOUT_VDD_2_3            21
#define VPX_VOUT_GWT_DLL0           22
#define VPX_VOUT_GWT_DLL1           23
#define VPX_VOUT_GWT_DLL2           24
#define VPX_VOUT_GWT_DLL3           25

// ----------------------------------------------------------------------------
#endif // VPXDEFS_H_
