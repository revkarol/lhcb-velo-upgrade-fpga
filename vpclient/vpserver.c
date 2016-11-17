#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "spidrvpxcmds.h"
#include "vpxdefs.h"
#include "MiniDAQ.h"	//AFP


#define u32 uint32_t
#define u8 uint8_t

// CPU packet-to/from-Velopix indices
#define PKT_HEADER_I                0
#define PKT_CHIPADDR_HI_I           1
#define PKT_CHIPADDR_LO_I           2
#define PKT_REGADDR_HI_I            3
#define PKT_REGADDR_LO_I            4

// Error identifiers
#define VPX_ERR_PARAMETER           0x01
#define VPX_ERR_RX_TIMEOUT          0x02
#define VPX_ERR_TX_TIMEOUT          0x04
#define VPX_ERR_EMPTY               0x08
#define VPX_ERR_FULL                0x10
#define VPX_ERR_NOTEMPTY            0x20
#define VPX_ERR_UNEXP_REPLY         0x40
#define VPX_ERR_REPLY               0x80
#define vpx_err()   (ERR_VPX_HARDW | (vpx_errorid()<<8))
#define store_err() (ERR_FLASH_STORAGE | (storage_errorid()<<8))

static u32 u8array_to_u32( u8 *bytes );
static void u32_to_u8array( u32 val, u8 *bytes );
int open_session();

void err_die(char *msg, int sock);
void do_cmd(u8 *msg, int n, int sock);
void do_response(u8 *msg, int sock);

int vpx_reg( int addr, int nbytes, u8 *data, u32 *status );
int vpx_set_reg( int addr, int nbytes, u8 *data, u32 *status );
int vpx_errorid();
static int vpx_get_reply( int nbytes, u32 address );
static void vpx_tx_transmit();
static int vpx_tx_full();
static int vpx_rx_notempty();
static u32 vpx_addr_map(u32 dev);

int listen_fd;
u8 sendbuff[2048/8];
u8 recvbuff[2048/8];

static u32 UseBroadcast = 0;
static u32  VpxId;
static int  VpxErr;
// Reserve enough space to assemble a packet for the largest register,
// and make the packet size a multiple of 4 bytes (32-bit words)
static u8 VpxPkt[1+2+2+(1536/8)+2+1];

void err_die(char *msg, int sock)
{
        perror(msg);
        if(sock != 0)
                close(sock);
        exit(EXIT_FAILURE);
}


int open_session()
{
	struct sockaddr_in sa;
	lbPcie_user_init();       //inicialize the PCIe of CCPC AFP
        listen_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listen_fd == -1) {
                err_die("cannot create socket", 0);
        }
        int yes=1;
        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
                err_die("setsockopt", listen_fd);
        }

        memset(&sa, 0, sizeof sa);
        memset(sendbuff, 0, sizeof sendbuff);
        memset(recvbuff, 0, sizeof recvbuff);

        sa.sin_family = AF_INET;
        sa.sin_port = htons(50000);
        sa.sin_addr.s_addr = htonl(INADDR_ANY);

        //close(listen_fd);
        if (bind(listen_fd,(struct sockaddr *)&sa, sizeof sa) == -1) {
                err_die("bind failed", listen_fd);
        }

        if (listen(listen_fd, 10) == -1) {
                err_die("listen failed", listen_fd);
        }

        while(1) {
                int connect_fd = accept(listen_fd, NULL, NULL);

                if (0 > connect_fd) {
                	err_die("accept failed", listen_fd);
                }

                bzero(recvbuff, sizeof (recvbuff));
                u32 n = read(connect_fd, recvbuff, sizeof (recvbuff)-1);

		do_cmd(recvbuff, n, connect_fd);
                //u8 to32 = recvbuff[0]<<24 | recvbuff[1]<<16 | recvbuff[2]<<8 | recvbuff[3];
                //printf("got cmd size %u %x %x %x %x \n", sizeof recvbuff[0], recvbuff[0] , recvbuff[1],   recvbuff[2],   recvbuff[3]);
                //u32 r = ntohl(recvbuff[0]);
                //printf("got cmd %x %x\n", r, to32);
                if (shutdown(connect_fd, SHUT_RDWR) == -1) {
                        close(connect_fd);
                	err_die("shutdown failed", listen_fd);
                }
                close(connect_fd);
        }

 	lbPcie_user_close();  //close PCIe port of CCPC
        close(listen_fd);
        return EXIT_SUCCESS;  
}

void do_cmd(u8 *msg, int n, int sock)
{
	u32 cmd  = (u8array_to_u32(&(msg[0*4])));
	u32 len  = (u8array_to_u32(&(msg[1*4])));
	u32 dum  = (u8array_to_u32(&(msg[2*4])));
	u32 dev  = (u8array_to_u32(&(msg[3*4])));
	u32 data = (u8array_to_u32(&(msg[4*4])));
        u8 reply[5*4 + 1536]; // Large enough to fit largest register (1536 bits) 8x
        u32 err   = ERR_NONE;
        u32 rlen = 5 * 4; // The default reply length including 1 dataword
        printf("got packet %x %d %d %d %x \n", cmd, len, dum, dev, data);

	VpxId = dev;

	switch(cmd){
		case CMD_GET_SOFTWVERSION : 
			do_response("SWVERSION: 0.0.1", sock);
                        break;
		case CMD_GET_FIRMWVERSION : 
			do_response("FWVERSION: 3.0.1", sock);
                        break;
		case CMD_GET_VPXREG : 
                        {
                        int addr   = (int) (data & 0xFFFF);
                        int nbytes = (int) ((data >> 16) & 0xFF);
                        int regcnt = (int) ((data >> 24) & 0xFF);
                        int reg;
                        u32 status;
                        // Optionally read multiple registers with one command
                        if( regcnt == 0 ) regcnt = 1;
                        if( (5*4 + nbytes*regcnt) <= sizeof(reply ) )
                        {
                                for( reg=0; reg<regcnt; ++reg )
                                {
                                        if( !vpx_reg( addr + reg, nbytes,
                                                                &reply[5*4 + reg*nbytes], &status ) )
                                                err = vpx_err();
                                        // Also return the Velopix status word
                                        data &= 0xFFFF;
                                        data |= (status << 16);
                                }
                                // Add expected number of bytes after 'data'
                                rlen += nbytes * regcnt;
                        }
                        else
                        {
                                // Reply wouldn't fit !
                                err = ERR_ILLEGAL_PAR;
                        }
                        }

                        do_response("CMD_GET_VPXREG: OK", sock);

                        break;
		case CMD_SET_VPXREG: 
                        {
                        int addr   = (int) (data & 0xFFFF);
                        int nbytes = (int) ((data >> 16) & 0xFF);
                        int regcnt = (int) ((data >> 24) & 0xFF);
                        int reg;
                        u32 status;
                        // Optionally write multiple registers with one command
                        if( regcnt == 0 ) regcnt = 1;
                        for( reg=0; reg<regcnt; ++reg )
                        {
                                if( !vpx_set_reg( addr + reg, nbytes,
                                                        &msg[5*4 + reg*nbytes], &status ) )
                                        err = vpx_err();
                                // Also return the Velopix status word
                                data &= 0xFFFF;
                                data |= (status << 16);
                        }
                        }
                        do_response("CMD_SET_VPXREG: OK", sock);

                        break;
		default : 
			do_response("Unknown command", sock);
	}	
}



void do_response(u8 *msg, int sock)
{
	
	int n = write(sock, msg, strlen(msg));
	if (n<0) err_die("write failed", sock);

}

static u32 u8array_to_u32( u8 *bytes )
{
  // Extract a u32 item from the 'bytes' array
  u32 val;
  // This platform is big-endian, like the network
  val  = (u32) (bytes[0] << 24);
  val |= (u32) (bytes[1] << 16);
  val |= (u32) (bytes[2] << 8);
  val |= (u32) bytes[3];
  return val;
}

// ----------------------------------------------------------------------------

static void u32_to_u8array( u32 val, u8 *bytes )
{
  // Put a u32 item in the 'bytes' array:
  // the network is big-endian
  bytes[0] = (u8) ((val & 0xFF000000) >> 24);
  bytes[1] = (u8) ((val & 0x00FF0000) >> 16);
  bytes[2] = (u8) ((val & 0x0000FF00) >> 8);
  bytes[3] = (u8) ((val & 0x000000FF) >> 0);
}

// ----------------------------------------------------------------------------

int vpx_reg( int addr, int nbytes, u8 *data, u32 *status )
{
  u32 vpx_addr;
  if( UseBroadcast )
    vpx_addr = VPX_BROADCAST_ADDR;
  else
    vpx_addr = VpxId;

  u8 vpx_addr_hi = (u8) ((vpx_addr & 0x7F00) >> 8);
  u8 vpx_addr_lo = (u8) (vpx_addr & 0xFF);
  u8 reg_addr_hi = (u8) ((addr & 0xFF00) >> 8);
  u8 reg_addr_lo = (u8) (addr & 0xFF);

  VpxPkt[0] = (u8) 0xE8;
  VpxPkt[1] = vpx_addr_hi | (u8) 0x80; // Incl. read bit = 1
  VpxPkt[2] = vpx_addr_lo;
  VpxPkt[3] = reg_addr_hi;
  VpxPkt[4] = reg_addr_lo;
  // Fill padding data bytes, not part of the actual message,
  // but written to the FIFO
  VpxPkt[5] = (u8) 0x00;
  VpxPkt[6] = (u8) 0x00;
  VpxPkt[7] = (u8) 0x00;

//#define DISPLAY_SEND_PACKET_GET
#ifdef DISPLAY_SEND_PACKET_GET
  {
    printf( "==> Sent %d*4 bytes:\n", 2 );
    int j;
    for( j=0; j<2*4; ++j )
      {
	printf( "%02X ", (u32) VpxPkt[j] );
	if( (j & 0x1F) == 0x1F ) printf( "\n" );
      }
    printf( "\n" );
  }
#endif

  // Do read sequence
  u32 word, address;
  u8 go;
  go = 0x7; // go 3 links of velo
  word = (((u32) VpxPkt[3] <<  0) | ((u32) VpxPkt[2] << 8) |
	  ((u32) VpxPkt[1] << 16) | ((u32) VpxPkt[0] << 24)) ;
   
  //KH SpidrRegs[SPIDRVPX_TX_DATA_I] = word;
  address = vpx_addr_map(vpx_addr);
  //address = (((u32) MINIDAQ_GBT_HEADER << 16) | ((u32) MINIDAQ_GBT_FIBER_4 <<12) | ((u32) MINIDAQ_TEST_REGISTER));		//MiniDAQ address

   printf("write one packet\n");
   if (lbPcie_user_write(address , &word, 1)!=0)                   //write address to read
    {
        printf("LLI error Writing rx1 \n");
    } 
    

  word = (((u32) VpxPkt[7] <<  0) | ((u32) VpxPkt[6] << 8) |
	  ((u32) VpxPkt[5] << 16) | ((u32) VpxPkt[4] << 24)) ;
  //KH SpidrRegs[SPIDRVPX_TX_DATA_I] = word;
  //vpx_tx_transmit();
   printf("write one packet\n");
   if (lbPcie_user_write(address , &word, 1)!=0)                   //write address to read
    {
        printf("LLI error Writing rx1 \n");
    } 

   printf("write go velo\n");
    if (lbPcie_user_write(MINIDAQ_GO_VELO , &go, 1)!=0)                   //Go velo
    {
        printf("LLI error Writing go rx2\n");
    } 	
  // Check TX statemachine?
  // ...


   printf("get reply\n");
  int nwords = vpx_get_reply( nbytes, address);

  // Check header
  if( VpxPkt[0] != 0xE8 )
    VpxErr |= VPX_ERR_REPLY;

  // Check return addresses (chip, register)
  if( UseBroadcast )
    {
      // Don't check chip address, only read/write bit
      if( (VpxPkt[1] & (u8) 0x80) != (u8) 0x80 )
	VpxErr |= VPX_ERR_REPLY;
    }
  else
    {
      if( VpxPkt[1] != (vpx_addr_hi | (u8) 0x80) ||
	  VpxPkt[2] != vpx_addr_lo )
	VpxErr |= VPX_ERR_REPLY;
    }
  if( VpxPkt[3] != reg_addr_hi ||
      VpxPkt[4] != reg_addr_lo )
    VpxErr |= VPX_ERR_REPLY | (reg_addr_lo << 8) | (reg_addr_hi << 16);

  // Copy status to caller (assume status cannot be trusted in case of error)
  if( status && VpxErr == 0 )
    *status = ((u32) VpxPkt[5] << 8) | (u32) VpxPkt[6];
  else if( status )
    *status = 0;

  // Copy payload to caller
  if( nbytes > 1536/8 ) nbytes = 1536/8;
  int i;
  for( i=0; i<nbytes; ++i )
    data[i] = VpxPkt[7 + i];

  if( VpxErr & VPX_ERR_REPLY )
    {
      // At least one of bytes 0-4 is not as expected, status in bytes 5-6
      /*error( "### VPX-reply: %02X %02X %02X %02X %02X Stat: %02X %02X",
	     (u32) VpxPkt[0], (u32) VpxPkt[1], (u32) VpxPkt[2],
	     (u32) VpxPkt[3], (u32) VpxPkt[4], (u32) VpxPkt[5],
	     (u32) VpxPkt[6] );*/

#define DISPLAY_RECV_PACKET_GET
#ifdef DISPLAY_RECV_PACKET_GET
      {
	printf( "==> Recvd %d*4 bytes:\n", nwords );
	int i;
	//for( i=0; i<nwords*4; ++i )
	for( i=0; i<nbytes; ++i )
	  {
	    printf( "%02X ", (u32) VpxPkt[i] );
	    if( (i & 0x1F) == 0x1F ) printf( "\n" );
	  }
	printf( "\n" );
      }
#endif
    }

  return( VpxErr == 0 );
}

// ----------------------------------------------------------------------------

int vpx_set_reg( int addr, int nbytes, u8 *data, u32 *status )
{
  u32 vpx_addr;
  if( UseBroadcast )
    vpx_addr = VPX_BROADCAST_ADDR;
  else
    vpx_addr = VpxId; //chip addr

  u8 vpx_addr_hi = (u8) ((vpx_addr & 0x7F00) >> 8);
  u8 vpx_addr_lo = (u8) (vpx_addr & 0xFF);
  u8 reg_addr_hi = (u8) ((addr & 0xFF00) >> 8);
  u8 reg_addr_lo = (u8) (addr & 0xFF);

  VpxPkt[0] = (u8) 0xE8;
  VpxPkt[1] = vpx_addr_hi; // Incl. write bit = 0
  VpxPkt[2] = vpx_addr_lo;
  VpxPkt[3] = reg_addr_hi;
  VpxPkt[4] = reg_addr_lo;

  u8 go;
  go = 0x7; // go 3 links of velo

  // Append the data bytes
  if( nbytes > 1536/8 ) nbytes = 1536/8;
  int i;
  for( i=0; i<nbytes; ++i )
    VpxPkt[5 + i] = data[i];

  // Append padding bytes as required
  for( i=0; i<((1+2+2+nbytes+3)/4)*4 - (5+nbytes); ++i )
    VpxPkt[5 + nbytes + i] = (u8) 0x00;

//#define DISPLAY_SEND_PACKET_SET
#ifdef DISPLAY_SEND_PACKET_SET
  {
    printf( "==> Sent %d*4 bytes:\n", (5+nbytes+3)/4 );
    int j;
    for( j=0; j<((5+nbytes+3)/4)*4; ++j )
      {
	printf( "%02X ", (u32) VpxPkt[j] );
	if( (j & 0x1F) == 0x1F ) printf( "\n" );
      }
    printf( "\n" );
  }
#endif

  // Do write sequence
  u32 word, address, j = 0;
  for( i=0; i<(1+2+2+nbytes+3)/4; ++i, j+=4 )
    {
      if( vpx_tx_full() )
	{
	  VpxErr |= VPX_ERR_FULL;
	  break;
	}
      word = (((u32) VpxPkt[j+3] <<  0) | ((u32) VpxPkt[j+2] << 8) |
	      ((u32) VpxPkt[j+1] << 16) | ((u32) VpxPkt[j+0] << 24)) ;
      //SpidrRegs[SPIDRVPX_TX_DATA_I] = word;
//	address = (((u32) MINIDAQ_GBT_HEADER << 16) | ((u32) MINIDAQ_GBT_FIBER_4 <<12) | ((u32) MINIDAQ_VPX_ADDR_1));
  address = vpx_addr_map(vpx_addr);
       if (lbPcie_user_write(&address, &word, 1)!=0)
        {
                                        printf("LLI bar2 error writing tx.\n");
        }    
    }
   printf("write go velo\n");
    if (lbPcie_user_write(MINIDAQ_GO_VELO , &go, 1)!=0)                   //Go velo
    {
        printf("LLI error Writing go rx2\n");
    } 	
  if( VpxErr == 0 )
    vpx_tx_transmit();

  int nwords = vpx_get_reply( 20+nbytes, address+0x40);  // minidaq address for velo read is always [write addr + 0x40]

  // Check header
  if( VpxPkt[0] != 0xE8 )
    VpxErr |= VPX_ERR_REPLY;

  // Check return addresses (chip, register)
  if( UseBroadcast )
    {
      // Don't check chip address, only read/write bit
      if( (VpxPkt[1] & (u8) 0x80) != (u8) 0x00 )
	VpxErr |= VPX_ERR_REPLY;
    }
  else
    {
      if( VpxPkt[1] != vpx_addr_hi ||
	  VpxPkt[2] != vpx_addr_lo )
	VpxErr |= VPX_ERR_REPLY;
    }
  if( VpxPkt[3] != reg_addr_hi ||
      VpxPkt[4] != reg_addr_lo )
    VpxErr |= VPX_ERR_REPLY | (reg_addr_lo << 8) | (reg_addr_hi << 16);

  // Copy status to caller (assume status cannot be trusted in case of error)
  if( status && VpxErr == 0 )
    *status = ((u32) VpxPkt[5] << 8) | (u32) VpxPkt[6];
  else if( status )
    *status = 0;

  if( VpxErr & VPX_ERR_REPLY )
    // At least one of bytes 0-4 is not as expected, status in bytes 5-6
    error( "### VPX-reply: %02X %02X %02X %02X %02X Stat: %02X %02X",
	   (u32) VpxPkt[0], (u32) VpxPkt[1], (u32) VpxPkt[2],
	   (u32) VpxPkt[3], (u32) VpxPkt[4], (u32) VpxPkt[5],
	   (u32) VpxPkt[6] ); 

//#define DISPLAY_RECV_PACKET_SET
#ifdef DISPLAY_RECV_PACKET_SET
  printf( "==> Recvd %d*4 bytes:\n", nwords );
  for( i=0; i<nwords*4; ++i )
    {
      printf( "%02X ", (u32) VpxPkt[i] );
      if( (i & 0x1F) == 0x1F ) printf( "\n" );
    }
  printf( "\n" );
#endif

  return( VpxErr == 0 );
}

int vpx_errorid()
{
  // Return error identifier, then reset it
  int err = VpxErr;
  VpxErr = 0;
  return err;
}

static int vpx_get_reply( int nbytes, u32 address )
{
  // Read reply data from Velopix into VpxPkt array
  // while not-empty and expected #bytes not received, with time-out
  u8 *pkt         = VpxPkt;
  u32 word;
  int index       = 0;
  int expected    = (1+2+2+(1536/8)+2+1)/4; // NB: SPIDR always reads max #bytes
  int expected_s  = (1+2+2+nbytes+2 + 3)/4; // Expected significant words/bytes 
  int timeout_cnt = 100 + expected;
  //while( index < expected && timeout_cnt > 0 )
  while( index < expected && timeout_cnt > 0 )
    {
      if( vpx_rx_notempty() )
	{
	  // KH word = SpidrRegs[SPIDRVPX_RX_DATA_I];
	  lbPcie_user_readW(address, &word);
              printf("reply address=%x word = %x, \n", address, word);
	  // Don't bother to unpack and store non-significant bytes
	  if( index < expected_s )
	    {
	      *pkt = (unsigned char) ((word >> 24) & 0xFF); ++pkt;
	      *pkt = (unsigned char) ((word >> 16) & 0xFF); ++pkt;
	      *pkt = (unsigned char) ((word >>  8) & 0xFF); ++pkt;
	      *pkt = (unsigned char) ((word >>  0) & 0xFF); ++pkt;
	    }
	  ++index;
	}
      --timeout_cnt;
    }

  // Indicate various error cases
 /*
  if( vpx_rx_notempty() )
    VpxErr |= VPX_ERR_NOTEMPTY;
  if( index > 0 && index < expected )
    VpxErr |= VPX_ERR_UNEXP_REPLY;
  else if( timeout_cnt == 0 )
    VpxErr |= VPX_ERR_EMPTY;
*/
  // Return number of words read
  return index;
}

static void vpx_tx_transmit()
{
  // KH SpidrRegs[SPIDRVPX_TX_STATEMACH_I] |= SPIDRVPX_TX_TRANSMIT;
  //printf( "tx C04 = %08X\n", spidr_reg(0xC04>>2) );
  //printf( "tx C04 = %08X\n", spidr_reg(0xC04>>2) );
  // KH SpidrRegs[SPIDRVPX_TX_STATEMACH_I] &= ~SPIDRVPX_TX_TRANSMIT;
}

static int vpx_tx_full()
{
  return 0;	//( (SpidrRegs[SPIDRVPX_TX_STATEMACH_I] & SPIDRVPX_TX_FULL) != 0 );
}

static int vpx_rx_notempty()
{
  return 1;	//( (SpidrRegs[SPIDRVPX_RX_STATEMACH_I] & SPIDRVPX_RX_EMPTY) == 0 );
}

static u32 vpx_addr_map(u32 dev)
{
	u32 minidaq_vpx_addr;
	u32 minidaq_gbt_fiber;
	minidaq_gbt_fiber = MINIDAQ_GBT_FIBER_4;
	u32 address;
	switch (dev){
		case 0 : 
			minidaq_vpx_addr = MINIDAQ_VPX_ADDR_0;
			break;
		case 1 : 
			minidaq_vpx_addr = MINIDAQ_VPX_ADDR_1;
			break;
		case 2 : 
			minidaq_vpx_addr = MINIDAQ_VPX_ADDR_2;
			break;
		case 3 : 
			minidaq_vpx_addr = MINIDAQ_VPX_ADDR_3;
			break;
		case 4 : 
			minidaq_vpx_addr = MINIDAQ_VPX_ADDR_4;
			break;
		case 5 : 
			minidaq_vpx_addr = MINIDAQ_VPX_ADDR_5;
			break;
	}
	address = (((u32) MINIDAQ_GBT_HEADER << 16) | ((u32) minidaq_gbt_fiber <<12) | ((u32) minidaq_vpx_addr));
	return address;
		
}

int main()
{
	int ret = open_session();
	return ret;
}
