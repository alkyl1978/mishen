
#include "stm32f0xx.h" 

int16_t crc16_modbus(unsigned char *buf,unsigned char len)
{

unsigned int crc=0xffff,temp;
unsigned char pos,i;
for (pos=0; pos<=len; pos++)
{
  temp=buf[pos];
  temp&=0x00ff;
  crc ^=temp;
  for (i=0; i<8; i++)
  {
      temp=crc;
      crc>>=1;
      if(temp & 0x0001) crc^=0xa001;
  }
}

return crc;
}  
