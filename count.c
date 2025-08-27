/*

(c) G7TAJ 2025
License GPL-3.0 license

*/

#include <stdio.h>
#include <stdlib.h>

void printBinary(unsigned int num, int bits) {
    for (int i = bits - 1; i >= 0; i--) {
        // Print each bit from the most significant to the least significant
        printf("%d", (num >> i) & 1);
    }
}
int main(int argc, char *argv[]) {
 unsigned char byte;


    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Error opening file");
        return 1;
    }


/* original prom channels =

  1   145.250
  2   144.625
  3   144.650
  4   144.675

*/

  // channel switch
    int A4 = 1;
    int A5 = 1;
    int A6 = 1;
    int A7 = atoi(argv[3]);

    // JP  A2 & A3 default grounded
    int A2 = 0;
    int A3 = 0;

    switch (atoi(argv[2])) {
     case 1 : printf("Channel 1\n");
              break;
     case 2 : printf("Channel 2\n");
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

 printf("Switch poistion : A4 = %d | A5 = %d | A6 = %d\n", A4,A5,A6);

//    int max_count = 4096; // 12 bits
int max_count = 65;
int nj_count = 0;
int nj_word = 0;

    for (int count = 0; count < max_count; count++) {
        unsigned int word = 0;

        word = 1; // A0 = 1

        word |= (1 << 1); // A1 = 1
        word |= (A2 << 2);
        word |= (A3 << 3);
        word |= (A4 << 4);
        word |= (A5 << 5);
        word |= (A6 << 6);
        word |= (A7 << 7); // A7 = PTT
        word |= (count  & 1) << 10; // Q0 = A10
        word |= ((count >> 1) & 1) << 8; // Q1 = A8
        word |= ((count >> 2) & 1) << 9; // Q2 = A9
        word |= ((count >> 3) & 1) << 11; // Q3 = A11
        word |= ((count >> 4) & 1) << 13; // Q4 = A13
        word |= ((count >> 5) & 1) << 12; // Q5 = A12

        printf("Count: %4d | %04X ", count, word);
	printBinary(word,13);

       fseek(file, word, SEEK_SET);
        if (fread(&byte, sizeof(unsigned char), 1, file) != 1) {
            printf("Error reading byte at address %03X\n", word);
            break;
        }
        printf(" Addr: %04X | %04X | ", word, byte);
        printBinary(byte, 8);
        printf(" | Data: %d | Clock: %d | Enable: %d\n",
               (byte >> 4) & 1, // Data
               (byte >> 5) & 1, // Clock
               (byte >> 6) & 1  // Enable
        );

      if ( (((byte >> 5) & 1) == 0) && ( (byte >> 6) & 1 ) ) {  //enabled & clock falling edge
	 nj_word = nj_word << 1;
         nj_word |= ((byte >> 4) & 1);
	 nj_count++;
      }
   }

    printf("NJ count : %d | NJ word %010X ", nj_count, nj_word);
    printBinary(nj_word,28);
    printf("\n");

    //               a        m          r
    // calc freq  0101000 0001001100 00011001000

    int a = (nj_word >> 21 ) & 0x7F ;  // 7 bits
    int m = (nj_word >> 11) & 0x3FF ;  // 10 bits
                                       // r is alwasy 200
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

    return 0;
}
