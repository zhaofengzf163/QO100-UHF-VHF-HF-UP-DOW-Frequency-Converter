/* Very simple software for PLL ADF4351 and ATTiny13
  can be compiled with arduino and microcore
  https://github.com/MCUdude/MicroCore


  Please check the ADF4351 datasheet or use ADF4351 software
  to get PLL register values.


  ATTINY44
  PA1 ADF4531 clock (CLK)
  PA2  ADF4531 data (DATA)
  PA3  ADF4531 latch enable (LE)
  PB0  TX_EN


  /********* 10M 基准 ********************************
  TX 2400.05     RX_LNB:739.55
   **************IF=28.55*****************************
    // TX  LOFout= 2371.5 MHz -4dBm LO= 2371.5 MHz +28.55 MHz=2400.05MHZ

  //RX LOFout= 711 MHz -4dBm LO= 739.55MHz -711 MHz=28.55MHZ



  ************IF=145.55*******************
    // TX  LOFout= 2254.5 MHz -4dBm LO= 2254.5 MHz +145.55 MHz=2400.05MHZ
   //RX LOFout= 594 MHz -4dBm LO= 739.55MHz -594 MHz=145.55MHZ

******************IF=430.55 **********************************
  // TX  LOFout= 1969.5 MHz -4dBm LO= 1969.5 MHz +430.55 MHz=2400.05MHZ

  //RX LOFout= 309 MHz -4dBm LO= 739.55MHz -309 MHz=430.55MHZ
************************************************************************
  // PLL registers
  //long int r0, r1, r2, r3, r4, r5;
  //long int TX_R[6] = {0X4E00A8, 0X80080C9, 0X4E42, 0X4B3, 0X9C803C, 0X580005};
  //long int RX_R[6] = {0X300000, 0X8008011, 0X4E42, 0X4B3, 0XBC803C, 0X580005};

  20200114  使用attiny44 mcu  拨码开关控制中频为 28 145 430  。
  20200113 调试了rx—lo幅度
          // write2PLL(0XB50024);   //-4DB
          //write2PLL(0xB5002C);   //-1DB
          //write2PLL(0xB50034);   //+2DB
          //write2PLL(0xB5003C);   //+5DB
          //write2PLL(0xB500FC);   //+5DB
 20200214 调试了TXlo，lnb基准使用gpsdo。cw模式调节oxco，侧音800hz。然后微调txlo，发射偏差月100hz。
*//////////////////////////////////////////////////////////////////////////////////////////////


int LE = 3;                                 //EL引脚
int CLK = 1;                                 //CLK引脚
int DAT = 2;                                //DAT引脚
int RX_TX = 8;                              //PTT
int LED = 0;                                //RX/TX_LED
int TX_EN = 10;
int RX_P;
int TX_P;
int BAND = 0;
int BAK = 5;
int VHF = 6;
int UHF = 4;
int HF  = 7;
void write2PLL(uint32_t PLLword)
{ // clocks 32 bits word  directly to the ADF4351
  // msb (b31) first, lsb (b0) last
  for (byte i = 32; i > 0; i--)
  { // PLL word 32 bits
    (PLLword & 0x80000000 ? PORTA |= 0b00000100 : PORTA &= 0b11111011);  // data on PA2
    PORTA |= 0b00000010;                   // clock in bit on rising edge of CLK (PA1 = 1)
    PORTA &= 0b11111101;                   // CLK (PA1 = 0)
    PLLword <<= 1;                         // rotate left for next bit
  }
  PORTA |= 0b00001000;                   // latch in PLL word on rising edge of LE (PA3 = 1)
  PORTA &= 0b11110111;  // LE (PA3 = 0)

  delayMicroseconds(1);
}

void setup ()
{
  //DDRB  = 0xff; // PB are all outputs
  //PORTB = 0x00; // make PB low
  pinMode(LE, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DAT, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(RX_TX, INPUT_PULLUP);
  pinMode(HF, INPUT_PULLUP);
  pinMode(VHF, INPUT_PULLUP);
  pinMode(UHF, INPUT_PULLUP);
  pinMode(BAK, INPUT_PULLUP);
  pinMode(TX_EN, OUTPUT);
  digitalWrite(LED, 1);
  delay(1000);

  if (digitalRead(HF) == 0 && digitalRead(VHF) == 1 && digitalRead(UHF) == 1 && digitalRead(BAK) == 1)BAND = 1;
  if (digitalRead(HF) == 1 && digitalRead(VHF) == 0 && digitalRead(UHF) == 1 && digitalRead(BAK) == 1)BAND = 2;
  if (digitalRead(HF) == 1 && digitalRead(VHF) == 1 && digitalRead(UHF) == 0 && digitalRead(BAK) == 1)BAND = 3;
  if (digitalRead(HF) == 1 && digitalRead(VHF) == 1 && digitalRead(UHF) == 1 && digitalRead(BAK) == 0)BAND = 4;

  switch (BAND)
  {
    case 1:
      //RX LOFout= 711 MHz -4dBm LO= 739.55MHz -711 MHz=28.55MHZ
      write2PLL(0X8008029 & 0xffffffdf);  //关闭输出
      write2PLL(0X580005);
      //write2PLL(0XA50024);   //-4DB
      write2PLL(0xA5003C);   //+5DB
      write2PLL( 0X4B3);
      write2PLL(0X4E42);
      write2PLL(0X8008029);
      write2PLL(0X8E0010);
      break;
    case 2:
      //RX LOFout= 594 MHz -4dBm LO= 739.55MHz -594 MHz=145.55MHZ
      write2PLL(0x8008029 & 0xffffffdf);  //关闭输出
      write2PLL(0X580005);
      write2PLL(0xA50024);   //-4DB
      //write2PLL(0xA500FC);   //+5DB  KK0311
      write2PLL( 0X4B3);
      write2PLL(0X4E42);
      write2PLL(0x8008029);
      write2PLL(0X768018);
      break;
    case 3:
      //RX LOFout= 309 MHz -4dBm LO= 739.55MHz -309 MHz=430.55MHZ
      write2PLL(0XB50024 & 0xffffffdf);  //关闭输出
      write2PLL(0X580005);
      //write2PLL(0XB50024);   //-4DB
      write2PLL(0xB500FC);   //+5DB
      write2PLL( 0X4B3);
      write2PLL(0X4E42);
      write2PLL(0X8008029);
      write2PLL(0X7B8008);
      break;
    case 4:
      while (1)
      {
        error_dis();
      }
      break;
    default :
      while (1)
      {
        error_dis();
      }

  }

  digitalWrite(TX_EN, 1);
  digitalWrite(LED, 0);
}

// main loop
void loop()
{
  //digitalWrite(LED, 0);
  //delay(100);
  if (digitalRead(RX_TX) == 1)
  { if (RX_P == 0)
    { delay(500);
      switch (BAND)
      {
        case 1:
          //RX LOFout= 711 MHz -4dBm LO= 739.55MHz -711 MHz=28.55MHZ
          write2PLL(0X8008029 & 0xffffffdf);  //关闭输出
          write2PLL(0X580005);
           //write2PLL(0XA50024);   //-4DB
          write2PLL(0xA5003C);   //+5DB
          write2PLL( 0X4B3);
          write2PLL(0X4E42);
          write2PLL(0X8008029);
          write2PLL(0X8E0010);
          break;
        case 2:
          //RX LOFout= 594 MHz -4dBm LO= 739.55MHz -594 MHz=145.55MHZ
          write2PLL(0x8008029 & 0xffffffdf);  //关闭输出
          write2PLL(0X580005);
          write2PLL(0xA50024);   //-4DB
          //write2PLL(0xA500FC);   //+5DB  KKK0311
          write2PLL( 0X4B3);
          write2PLL(0X4E42);
          write2PLL(0x8008029);
          write2PLL(0X768018);
          break;
        case 3:
          //RX LOFout= 309 MHz -4dBm LO= 739.55MHz -309 MHz=430.55MHZ
          write2PLL(0XB50024 & 0xffffffdf);  //关闭输出
          write2PLL(0X580005);
          // write2PLL(0XB50024);   //-4DB
          write2PLL(0xB500FC);   //+5DB
          write2PLL( 0X4B3);
          write2PLL(0X4E42);
          write2PLL(0X8008029);
          write2PLL(0X7B8008);
          break;
        case 4:
          while (1)
          {
            error_dis();
          }
          break;
        default :
          while (1)
          {
            error_dis();
          }
      }
      RX_P = 1;
      TX_P = 0;
      digitalWrite(TX_EN, 1);
      digitalWrite(LED, 0);
    }
  }
  else
  { if (TX_P == 0)
    {
      switch (BAND)
      {
        case 1:
          // TX  LOFout= 2371.5 MHz -4dBm LO= 2371.5 MHz +28.55 MHz=2400.05MHZ
          //          write2PLL(0X80080A1 & 0xffffffdf);  //关闭输出
          //          write2PLL(0X580005);
          //          write2PLL(0X850024);   //-4DB
          //          write2PLL( 0X4B3);
          //          write2PLL(0X4E42);
          //          write2PLL(0X80080A1);
          //          write2PLL(0X768018);
          // TX  LOFout= 2371.5 MHz -4dBm LO= 2371.5008 MHz +28.55 MHz=2400.05MHZ
          write2PLL(0X80080A1 & 0xffffffdf);  //关闭输出
          write2PLL(0X580005);
          //write2PLL(0X828024);   //-4DB
          write2PLL(0X82802c);   //+2DB
          write2PLL(0X4B3);
          write2PLL(0X8E42);
          write2PLL(0X800E1A9);
          write2PLL(0XED1D50);
          break;
        case 2:
          // TX  LOFout= 2254.5 MHz -4dBm LO= 2254.5 MHz +145.55 MHz=2400.05MHZ
          //          write2PLL(0X80080A1 & 0xffffffdf);  //关闭输出
          //          write2PLL(0X580005);
          //          write2PLL(0X850024);   //-4DB
          //          write2PLL( 0X4B3);
          //          write2PLL(0X4E42);
          //          write2PLL(0X80080A1);
          //          write2PLL(0X708048);
          // TX  LOFout= 2254.5 MHz -4dBm LO= 2254.5008 MHz +145.55 MHz=2400.05MHZ
          write2PLL(0X80080A1 & 0xffffffdf);  //关闭输出
          write2PLL(0X580005);
          write2PLL(0X828024);   //-4DB
          write2PLL( 0X4B3);
          write2PLL(0X8E42);
          write2PLL(0X800E1A9);
          write2PLL(0XE157E8);
          break;
        case 3:
          // TX  LOFout= 1969.5 MHz -4dBm LO= 1969.5 MHz +430.55 MHz=2400.05MHZ
          write2PLL(0X8008051 & 0xffffffdf);  //关闭输出
          //          write2PLL(0X580005);
          //          write2PLL(0X950024);   //-4DB
          //          write2PLL( 0X4B3);
          //          write2PLL(0X4E42);
          //          write2PLL(0X8008051);
          //          write2PLL(0XC48048);
          // TX  LOFout= 1969.5 MHz -4dBm LO= 1969.500625 MHz +430.55 MHz=2400.05MHZ
          write2PLL(0X580005);
          write2PLL(0X928024);   //-4DB
          write2PLL( 0X4B3);
          write2PLL(0X8E42);
          write2PLL(0x800FD01);
          write2PLL(0x189E408);
          break;
        case 4:
          while (1)
          {
            error_dis();
          }
          break;
        default :
          while (1)
          {
            error_dis();
          }
      }
      TX_P = 1;
      RX_P = 0;
      digitalWrite(TX_EN, 0);
      digitalWrite(LED, 1);
    }
  }
  // digitalWrite(LED, 1);
  // delay(100);
}
//*****************************THE END*********************************************************************8
void error_dis(void)
{ digitalWrite(LED, 1);
  delay(200);
  digitalWrite(LED, 0);
  delay(200);
}
// delay(10);
//TX ON/OFF can be used for CW keying
//write2PLL(r4 & 0xffffffdf);

////void write_dat(long int *r)
//// write from r5 to r0
//{ write2PLL(r5);
//  write2PLL(r4);
//  write2PLL(r3);
//  write2PLL(r2);
//  write2PLL(r1);
//  write2PLL(r0);
//}
