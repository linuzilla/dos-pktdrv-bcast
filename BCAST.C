/*  hdserv.c  -- */

#include <stdio.h>
#include <dos.h>
#include <bios.h>
#include <conio.h>
#include <alloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pktdrv.h"

const unsigned char broadcast[] = { -1, -1, -1, -1, -1, -1 };

int main(int argc, char *argv[])
{
    int                  vector;
    InPkt                pkt;
    unsigned int         pktlen = 100;
    unsigned long        counter = 0L;
    int                  x, y, i;
    unsigned int         delaytime = 0; /* in milliseconds */
    int                  terminate = 0;
    unsigned char        addrmsk[6] = { -1, -1, -1, -1, -1, -1 };
    unsigned char        *ptr;
    int                  masklen = 6;
    time_t               first, second;
    float                sec;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
            case 'd':
                delaytime = atoi(&argv[i][2]);
                break;
            case 'l':
                pktlen = atoi(&argv[i][2]);
                break;
            }
        } else {
            int  len = strlen(argv[i]);
            int  val, j;

            if ((len % 2) || (len > 12)) {
                printf("error: %s\n", argv[i]);
                exit(1);
            }
            strlwr(argv[i]);

            for (j = masklen = val = 0; j < len; j++) {
                switch (argv[i][j]) {
                case '0' : case '1' : case '2' : case '3' : case '4' :
                case '5' : case '6' : case '7' : case '8' : case '9' :
                    val = val * 16 + (int) (argv[i][j] - '0');
                    break;
                case 'a' : case 'b' : case 'c' : case 'd' : case 'e' : case 'f' :
                    val = val * 16 + (int) (argv[i][j] - 'a') + 10;
                    break;
                default:
                    printf("error: %s\n", argv[i]);
                    exit(1);
                }
                if (j % 2) {
                    addrmsk[masklen++] = (unsigned char) val;
                    val = 0;
                }
            }
            printf("%x:%x:%x:%x:%x:%x\n", addrmsk[0], addrmsk[1], addrmsk[2],
                                          addrmsk[3], addrmsk[4], addrmsk[5]);
        }
    }

    if ((vector = initial_pktdrv()) == 0) {
        printf("Packet Driver not found\n");
        return 2;
    }


    printf("Broadcast generator .... using packet driver install at 0x%02x\n\n\n\n\n\n", vector);
    printf("UP/DOWN/HOME/END -> packet size,  LEFT/RIGHT/PgUP/PgDN -> delay time");

    first = time(NULL);

    memcpy(pkt.da, broadcast, 6);
    memcpy(pkt.sa, addrmsk,   6);
    ptr = pkt.sa;

    gotoxy((x = 1), (y = wherey() - 4));
    printf("Packet Size = %4d, Delay time %6d milliseconds", pktlen, delaytime);

    while (! terminate) {
        delay(delaytime);
        for (i = 5; i >= masklen; i--) {
            if (ptr[i] == '\0') {
                ptr[i] = -1;
            } else {
                ptr[i]--;
                break;
            }
        }
        gotoxy(x, y + 2);
        fprintf(stderr, "send packet .... %lu", counter++);
        send_pkt(&pkt, pktlen);

        while (kbhit()) {
            switch (getch()) {
            case 27:
                terminate = 1;
                break;
            case 0:
                switch (getch()) {
                case 72: /* up    */
                    if (pktlen < 1500)
                        pktlen++;
                    break;
                case 80: /* down  */
                    if (pktlen > 60)
                        pktlen--;
                    break;
                case 77: /* right */
                    if (delaytime < 65535U)
                        delaytime++;
                    break;
                case 75: /* left  */
                    if (delaytime > 0)
                        delaytime--;
                    break;
                case 71: /* home  */
                    if (pktlen < 1500 - 10)
                        pktlen+=10;
                    break;
                case 79: /* end   */
                    if (pktlen > 60 + 10)
                        pktlen-=10;
                    break;
                case 73: /* pgup  */
                    if (delaytime < 65535U - 10)
                        delaytime+=10;
                    break;
                case 81: /* pgdn  */
                    if (delaytime > 0 + 10)
                        delaytime-=10;
                    break;
                }
                gotoxy(x, y);
                printf("Packet Size = %4d, Delay time %6d milliseconds", pktlen, delaytime);
                break;
            }
        }
    }
    second = time(NULL);

    fprintf(stderr, "\n\n\n\n\n");
    sec = difftime(second, first);
    printf("Elapse time: %3.1f seconds, %5.4f packets/sec.\n", sec, counter/sec);
    return 0;
}
