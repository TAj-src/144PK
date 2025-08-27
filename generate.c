#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define data_pos 4

void printBinary(unsigned int num, int bits) {
    for (int i = bits - 1; i >= 0; i--) {
        // Print each bit from the most significant to the least significant
        printf("%d", (num >> i) & 1);
    }
}
int main(int argc, char *argv[]) {
unsigned char byte, readbyte;


    FILE *file = fopen(argv[1], "rb+");
    if (!file) {
        perror("Error opening file");
        return 1;
    }


    // JP  A2 & A3 default grounded
    int A2 = 0;
    int A3 = 0;

  // channel switch
    int A4 = 1;
    int A5 = 1;
    int A6 = 1;
    int A7 = atoi(argv[3]);

    switch (atoi(argv[2])) {
     case 1 : printf("CHannel 1\n");
	       break;
     case 2 : printf("CHannel 2\n");
	      A4=0;
	      break;
     case 3 : printf("Channel 3\n");
	      A5=0;
	      break;
     case 4 : printf("Channel 4\n");
	      A4=0;
	      A6=0;
	      break;


    }

float freq_argv = atof(argv[4]);
int max_count = 65;
int nj_count = 0;
int nj_word = 0;
int r = 200; // 25khz pacing and 10Mhz fosc
double n, a_double, m_double;
int m, a;
int bandwidth = 25000;

   if ( A7 ) {             // RX IF offset 24.1Mhz
     freq_argv -= 21.4;
    }
    n = freq_argv * 1000000.0  / bandwidth;
    m_double = n / 64;
    m = m_double;
    a_double = roundf((m_double - m) * 64);
    a = (int)a_double;
    printf("r = %d | m_d = %f | m = %d | a = %d\n", r , m_double, m , a);

    nj_word  = a << 21;
    nj_word |= m << 11;
    nj_word |= r;

    printf("Creating ");
    if ( A7 )
	printf("RX");
    else
        printf("TX");

    printf(" string starting addr 0x");

    nj_count = 27;
    // start of word is 4 bytes in. Each byte present for 2 addresses. Clock transitions 1-0
    for (int count = 4; count < (28*2); count++) {
        unsigned int addr = 0;

        addr = 1; // A0 = 1

        addr |= (1 << 1); // A1 = 1
        addr |= (A2 << 2);
        addr |= (A3 << 3);
        addr |= (A4 << 4);
        addr |= (A5 << 5);
        addr |= (A6 << 6);
        addr |= (A7 << 7); // A7 = PTT
        addr |= (count  & 1) << 10; // Q0 = A10
        addr |= ((count >> 1) & 1) << 8; // Q1 = A8
        addr |= ((count >> 2) & 1) << 9; // Q2 = A9
        addr |= ((count >> 3) & 1) << 11; // Q3 = A11
        addr |= ((count >> 4) & 1) << 13; // Q4 = A13
        addr |= ((count >> 5) & 1) << 12; // Q5 = A12


	if (count==4) {
	   printf("%04X\nWORD =", addr);
	   printBinary(nj_word, 28);
	   printf("\n");
        }
	byte = (nj_word >> nj_count) & 1;
        printf(" Addr: %04X | Count: %02d | %04X | ", addr, nj_count, byte);
        printBinary(byte, 8);
        printf("\n");


        fseek(file, addr, SEEK_SET);
        if (fread(&readbyte, sizeof(unsigned char), 1, file) != 1) {
            printf("Error reading byte at address %03X\n", addr);
            break;
        }

	printf("Read byte = ");
	printBinary(readbyte,8);
	if ( byte ) {
	   readbyte |= (1 << data_pos);
	} else {
	   readbyte &= ~(1 << data_pos);
	}

	printf(" New byte = ");
	printBinary(readbyte,8);
	printf("\n");

        fseek(file, addr, SEEK_SET);
        if (fwrite(&readbyte, sizeof(unsigned char), 1, file) != 1) {
            printf("Error writing byte at address %03X\n", addr);
            break;
        }


      if ( (count-4) % 2 == 1 ) {
        nj_count--;
      }


   }  //for loop

    printf("NJ count : %d | NJ word %028X ", nj_count, nj_word);
    printBinary(nj_word,28);
    printf("\n");

/*
    // calc freq  0101000 0001001100 00011001000

    int a = (nj_word >> 21 ) & 0x7F ;
    int m = (nj_word >> 11) & 0x3FF ;

    printBinary(a,8);
    printf(" (%X)\n", a);
    printBinary(m,11);
    printf(" (%X)\n",m);

    double a2 = a / 64.0;
    a2 = a2 + m;
    a2 = a2 * 64.0;
    a2 = a2 * 25000.0;

    if ( A7 )
      printf("RX Freq = %f (plus IF = %f)\n", a2/1000000,(a2 /1000000) + 21.4);
    else
      printf("TX Freq = %f\n", a2/1000000);
*/
    return 0;
}
