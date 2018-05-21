#include <mbed.h>

I2CSlave slave(p9, p10);

int main() {
   char buf[10];
   char r1[6];
    r1[0] = 0x00;
    r1[1] = 0x00;
    r1[2] = 0x00;
    r1[3] = 0x00;
    r1[4] = 0x00;
    r1[5] = 0x00;
   
   slave.address(0xA0);
   
   while (1) {
       int i = slave.receive();
       //printf("I: %i\r\n");
       switch (i) {
           case I2CSlave::ReadAddressed:
               slave.write(r1, 6); // Includes null char
               break;
           case I2CSlave::WriteGeneral:
               slave.read(buf, 10);
               printf("Read G: %s\n", buf);
               for (int i=0; i<6; ++i)
                    printf("Reg %i: Byte: 0x%02x\r\n", i, buf[i]);
               break;
           case I2CSlave::WriteAddressed:
               slave.read(buf, 10);
               printf("Read A: %s\n", buf);
               for (int i=0; i<6; ++i) {
                    printf("Reg %i: Byte: 0x%02x\r\n", i, buf[i]);
                    r1[i] = buf[i];
                }
                slave.stop();
               break;
       }
       for(int i = 0; i < 10; i++) buf[i] = 0;    // Clear buffer
   }
}