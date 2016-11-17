/* ----------------------------------------------------------------------------
File   : MiniDAQ.h

Descr  : Functions for controlling MiniDAQ board-specific parts.

History:
16NOV2016; AFP   ; Created by Antonio Fernández Prieto.
---------------------------------------------------------------------------- */

//19 DOWNTO 16
#define	MINIDAQ_GBT_HEADER	0x5
//15 DOWNTO 12
#define	MINIDAQ_GBT_FIBER_3	0X3
#define	MINIDAQ_GBT_FIBER_4	0X4
#define	MINIDAQ_GBT_FIBER_5	0x5
// 11 DOWNTO 0
	//VELO
#define MINIDAQ_VPX_ADDR_0		0x600
#define MINIDAQ_VPX_ADDR_1		0x608
#define MINIDAQ_VPX_ADDR_2		0x610
#define MINIDAQ_VPX_ADDR_3		0x618
#define MINIDAQ_VPX_ADDR_4		0x620
#define MINIDAQ_VPX_ADDR_5		0x628
#define MINIDAQ_GO_VELO			0x630
#define MINIDAQ_CONTROL_FIFO	0X634
	//FRAMEWORK
		//Tx & Rx
#define MINIDAQ_P_SOL40_OFFSET 						0x004
#define MINIDAQ_P_SOL40_TFC_WORD_INPUT_DELAY		0x008
#define MINIDAQ_P_SOL40_TO_FE_TFC_CMD_OFFSET		0x00C
#define MINIDAQ_P_SUBDETECTOR_TYPE					0x010
#define MINIDAQ_CNT_UPDATE							0x064
#define MINIDAQ_EC_RUNNING							0x068
#define MINIDAQ_CNT_RESET							0x070
#define MINIDAQ_nSRES_QSOL40						0x084
#define MINIDAQ_nSRES_SC_IC							0x088
#define MINIDAQ_nSRES_SC_SCA						0x08C
#define MINIDAQ_nSRES_SC_VELO						0x090
#define MINIDAQ_CNT_RESET_INDIVIDUAL_01 			0x098
#define MINIDAQ_CNT_RESET_INDIVIDUAL_02				0x09C
#define MINIDAQ_GBTX_I2C_ADDRESS					0x100
#define MINIDAQ_EC_GBT_ALIVE 						0x108
#define MINIDAQ_FE_ENB 								0x110
#define MINIDAQ_P_LIMIT_FE_ENB						0x114
#define MINIDAQ_P_LIMIT_FE_FRAMES					0x118
#define MINIDAQ_P_OVERRIDE_SYNCH_LENGTH 			0x11C
#define MINIDAQ_TEST_REGISTER						0xFF0
		//Tx
#define MINIDAQ_F_ECS_IC_COMMAND					0x300
#define MINIDAQ_i_goIC								0x308
		//Rx
#define MINIDAQ_S_GBT_ALIVE							0x104
#define MINIDAQ_S_SC_IC_FLAG_ERROR					0x120
#define MINIDAQ_C_BXID_RESET						0x500
#define MINIDAQ_C_EID_RESET							0x504
#define MINIDAQ_C_FE_RESET							0x508
#define MINIDAQ_C_BE_RESET							0x50C
#define MINIDAQ_C_HEADER_ONLY						0x510
#define MINIDAQ_C_NZS_MODE							0x514
#define MINIDAQ_C_BX_VETO							0x518
#define MINIDAQ_C_TRIGGER							0x51C
#define MINIDAQ_C_SNAPSHOT							0x520
#define MINIDAQ_C_SYNCH								0x524
#define MINIDAQ_C_MEP_ACCEPT						0x528
#define MINIDAQ_C_RX_READY_FROM_LLI					0x52C
			//Test
#define MINIDAQ_TEST_FIBER_MAPPING_LOW				0xFE0
#define MINIDAQ_TEST_FIBER_MAPPING_HIGH				0xFE4
#define MINIDAQ_TEST_ACTIVE_FIBER_LOW				0xFE8
#define MINIDAQ_TEST_ACTIVE_FIBER_HIGH				0xFEC
#define MINIDAQ_DATE								0xFF4
#define MINIDAQ_VERSION								0XFF8




