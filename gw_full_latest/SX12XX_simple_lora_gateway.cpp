/* 
 *  Very simple LoRa gateway to test the new SX127XLT lib
 *
 *  Based on the simple receiver example from SX127XLT lib
 */

/* Output example 

-------------------------------------
Packet length: 10
Destination: 1
Packet Type: 16
Source: 8
SeqNo: 13

\!TC/22.50
CRC,4560,RSSI,-42dBm,SNR,8dB,Length,10,Packets,1,Errors,0,IRQreg,50
--> Packet is for gateway
-------------------------------------

*/

#ifdef SX126X
#include <SX126XLT.h>
SX126XLT LT;                                          
#endif

#ifdef SX127X
#include <SX127XLT.h>
SX127XLT LT;                                          
#endif

#ifdef SX128X
#include <SX128XLT.h>
SX128XLT LT;                                         
#endif

#include "SX12XX_simple_lora_gateway.h"                 //include the setiings file, frequencies, LoRa settings etc   

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h> 
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#define PRINTLN                   printf("\n")
#define PRINT_CSTSTR(param)       printf(param)
#define PRINTLN_CSTSTR(param)			do {printf(param);printf("\n");} while(0)	
#define PRINT_STR(fmt,param)      printf(fmt,param)
#define PRINTLN_STR(fmt,param)		{printf(fmt,param);printf("\n");}
#define PRINT_VALUE(fmt,param)    printf(fmt,param)
#define PRINTLN_VALUE(fmt,param)	do {printf(fmt,param);printf("\n");} while(0)
#define PRINT_HEX(fmt,param)      printf(fmt,param)
#define PRINTLN_HEX(fmt,param)		do {printf(fmt,param);printf("\n");} while(0)
#define FLUSHOUTPUT               fflush(stdout);

uint32_t RXpacketCount;
uint32_t errors;

uint8_t RXBUFFER[RXBUFFER_SIZE];                 //create the buffer that received packets are copied into

uint8_t RXPacketL;                               //stores length of packet received
int8_t  PacketRSSI;                              //stores RSSI of received packet
int8_t  PacketSNR;                               //stores signal to noise ratio (SNR) of received packet


void packet_is_OK()
{
  uint16_t IRQStatus, localCRC;

  IRQStatus = LT.readIrqStatus();                 //read the LoRa device IRQ status register

  RXpacketCount++;

  PRINTLN;
  LT.printASCIIPacket(RXBUFFER, RXPacketL);       //print the packet as ASCII characters
	PRINTLN;

  localCRC = LT.CRCCCITT(RXBUFFER, RXPacketL, 0xFFFF);  //calculate the CRC, this is the external CRC calculation of the RXBUFFER
  PRINT_CSTSTR("CRC,");                       //contents, not the LoRa device internal CRC
  PRINT_HEX("%X",localCRC);
  PRINT_CSTSTR(",RSSI,");
  PRINT_VALUE("%d",PacketRSSI);
  PRINT_CSTSTR("dBm,SNR,");
  PRINT_VALUE("%d",PacketSNR);
  PRINT_CSTSTR("dB,Length,");
  PRINT_VALUE("%d",RXPacketL);
  PRINT_CSTSTR(",Packets,");
  PRINT_VALUE("%d",RXpacketCount);
  PRINT_CSTSTR(",Errors,");
  PRINT_VALUE("%d",errors);
  PRINT_CSTSTR(",IRQreg,");
  PRINT_HEX("%X",IRQStatus);
  PRINTLN;
  FLUSHOUTPUT;
}


void packet_is_Error()
{
  uint16_t IRQStatus;
  IRQStatus = LT.readIrqStatus();                   //read the LoRa device IRQ status register

  if (IRQStatus & IRQ_RX_TIMEOUT)                   //check for an RX timeout
  {
  	//here if we don't receive anything, we normally print nothing
    //PRINT_CSTSTR("RXTimeout");
  }
  else
  {
    errors++;
    PRINT_CSTSTR("PacketError");
    PRINT_CSTSTR(",RSSI,");
    PRINT_VALUE("%d",PacketRSSI);
    PRINT_CSTSTR("dBm,SNR,");
    PRINT_VALUE("%d",PacketSNR);
    PRINT_CSTSTR("dB,Length,");
    PRINT_VALUE("%d",LT.readRXPacketL());               //get the device packet length
    PRINT_CSTSTR(",Packets,");
    PRINT_VALUE("%d",RXpacketCount);
    PRINT_CSTSTR(",Errors,");
    PRINT_VALUE("%d",errors);
    PRINT_CSTSTR(",IRQreg,");
    PRINT_HEX("%X",IRQStatus);
    LT.printIrqStatus();                            //print the names of the IRQ registers set
    PRINTLN;
  }

  FLUSHOUTPUT;
  delay(250);                                       //gives a longer buzzer and LED flash for error 
}

void setup() {

  SPI.begin();
  //Set Most significant bit first
  SPI.setBitOrder(MSBFIRST);
  //Divide the clock frequency
  SPI.setClockDivider(SPI_CLOCK_DIV64);
  //Set data mode
  SPI.setDataMode(SPI_MODE0); 

  //SPI beginTranscation is normally part of library routines, but if it is disabled in the library
  //a single instance is needed here, so uncomment the program line below
  //SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  //setup hardware pins used by device, then check if device is found
#ifdef SX126X
  if (LT.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, SW, LORA_DEVICE))
#endif

#ifdef SX127X
  if (LT.begin(NSS, NRESET, DIO0, DIO1, DIO2, LORA_DEVICE))
#endif

#ifdef SX128X
  if (LT.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, LORA_DEVICE))
#endif
  {
    PRINTLN_CSTSTR("Lora Device found");
    delay(1000);
  }
  else
  {
    PRINTLN_CSTSTR("No device responding");
    while (1)
    {
    }
  }

  //The function call list below shows the complete setup for the LoRa device using the information defined in the
  //Settings.h file.
  //The 'Setup Lora device' list below can be replaced with a single function call;
  //LT.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  //***************************************************************************************************
  //Setup Lora device
  //***************************************************************************************************
  
  LT.setMode(MODE_STDBY_RC);
#ifdef SX126X
  LT.setRegulatorMode(USE_DCDC);
  LT.setPaConfig(0x04, PAAUTO, LORA_DEVICE);
  LT.setDIO3AsTCXOCtrl(TCXO_CTRL_3_3V);
  LT.calibrateDevice(ALLDevices);                //is required after setting TCXO
  LT.calibrateImage(Frequency);
  LT.setDIO2AsRfSwitchCtrl();
#endif
#ifdef SX128X
  LT.setRegulatorMode(USE_LDO);
#endif
  //set for LoRa transmissions                              
  LT.setPacketType(PACKET_TYPE_LORA);
  //set the operating frequency                 
  LT.setRfFrequency(Frequency, Offset);
//run calibration after setting frequency
#ifdef SX126X
  if (LT.begin(NSS, NRESET, RFBUSY, DIO1, DIO2, DIO3, RX_EN, TX_EN, SW, LORA_DEVICE))
#endif
#ifdef SX127X
  LT.calibrateImage(0);
#endif
  //set LoRa modem parameters
#if defined SX126X || defined SX127X
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate, Optimisation);
#endif
#ifdef SX128X
  LT.setModulationParams(SpreadingFactor, Bandwidth, CodeRate);
#endif                                     
  //where in the SX buffer packets start, TX and RX
  LT.setBufferBaseAddress(0x00, 0x00);
  //set packet parameters
#if defined SX126X || defined SX127X                     
  LT.setPacketParams(8, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL);
#endif
#ifdef SX128X
  LT.setPacketParams(12, LORA_PACKET_VARIABLE_LENGTH, 255, LORA_CRC_ON, LORA_IQ_NORMAL, 0, 0);
#endif
  //syncword, LORA_MAC_PRIVATE_SYNCWORD = 0x12, or LORA_MAC_PUBLIC_SYNCWORD = 0x34
  //TODO check for sync word when SX128X 
#if defined SX126X || defined SX127X            
  LT.setSyncWord(LORA_MAC_PRIVATE_SYNCWORD);
  //set for highest sensitivity at expense of slightly higher LNA current
  LT.setHighSensitivity();  //set for maximum gain
#endif
#ifdef SX126X
  //set for IRQ on TX done and timeout on DIO1
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif
#ifdef SX127X
  //set for IRQ on RX done
  LT.setDioIrqParams(IRQ_RADIO_ALL, IRQ_TX_DONE, 0, 0);
#ifdef PABOOST
  LT.setPA_BOOST(true);
  PRINTLN_CSTSTR("Use PA_BOOST amplifier line");
#endif
#endif
#ifdef SX128X
  LT.setDioIrqParams(IRQ_RADIO_ALL, (IRQ_TX_DONE + IRQ_RX_TX_TIMEOUT), 0, 0);
#endif    
  
  //***************************************************************************************************

  PRINTLN;
  LT.printModemSettings();                                     //reads and prints the configured LoRa settings, useful check
  PRINTLN;
  LT.printOperatingSettings();                                 //reads and prints the configured operting settings, useful check
  PRINTLN;
  PRINTLN;
#if defined SX126X || defined SX127X  
  //print contents of device registers, normally 0x00 to 0x4F
  LT.printRegisters(0x00, 0x4F);
#endif                       
#ifdef SX128X
  //print contents of device registers, normally 0x900 to 0x9FF 
  LT.printRegisters(0x900, 0x9FF);
#endif  
  PRINTLN;
  PRINTLN;

#ifdef SX126X
	PRINT_CSTSTR("SX126X - ");
#endif
#ifdef SX127X
	PRINT_CSTSTR("SX127X - ");
#endif
#ifdef SX128X
	PRINT_CSTSTR("SX128X - ");
#endif

  PRINT_CSTSTR("Receiver ready - RXBUFFER_SIZE ");
  PRINTLN_VALUE("%d",RXBUFFER_SIZE);
  PRINTLN;
}

void loop()
{
  RXPacketL = LT.receiveAddressed(RXBUFFER, RXBUFFER_SIZE, 10000, WAIT_RX); //wait for a packet to arrive with 60seconds (60000mS) timeout

  PacketRSSI = LT.readPacketRSSI();              //read the recived RSSI value
  PacketSNR = LT.readPacketSNR();                //read the received SNR value

  if (RXPacketL == 0)                            //if the LT.receive() function detects an error, RXpacketL is 0
  {
    packet_is_Error();
  }
  else
  {
  	PRINTLN;
  	PRINTLN_CSTSTR("-------------------------------------");
  	PRINT_CSTSTR("Packet length: ");
  	PRINTLN_VALUE("%d", RXPacketL);
    PRINT_CSTSTR("Destination: ");
    PRINTLN_VALUE("%d", LT.readRXDestination());
    PRINT_CSTSTR("Packet Type: ");
    PRINTLN_VALUE("%d", LT.readRXPacketType());
    PRINT_CSTSTR("Source: ");
    PRINTLN_VALUE("%d", LT.readRXSource());
    PRINT_CSTSTR("SeqNo: ");
    PRINTLN_VALUE("%d", LT.readRXSeqNo());
  	PRINT_CSTSTR("RXTimestamp: ");
  	PRINTLN_VALUE("%d", LT.readRXTimestamp());
  	PRINT_CSTSTR("RXDoneTimestamp: ");
  	PRINTLN_VALUE("%d", LT.readRXDoneTimestamp());  	      
    packet_is_OK();
    
    if (LT.readRXDestination()==1)
    	PRINTLN_CSTSTR("--> Packet is for gateway");
    
    PRINTLN_CSTSTR("-------------------------------------");
    FLUSHOUTPUT;	
  }
}

int main (int argc, char *argv[]){

  setup();
  
  while(1){
    loop();
  }
  
  return (0);
}

