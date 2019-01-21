
#include <stdlib.h>
#include <string.h>
#include "common/definitions.h"
#include "richtext.h"

typedef struct
{
    char* name;
    unsigned long unicode;
}Utable;

static Utable utable[] =
{ "ALPHA",      0x0391
, "BETA",       0x0392
, "GAMMA",      0x0393
, "DELTA",      0x0394
, "EPSILON",    0x0395
, "ZETA",       0x0396
, "ETA",        0x0397
, "THETA",      0x0398
, "IOTA",       0x0399
, "KAPPA",      0x039A
, "LAMDA",      0x039B
, "MU",         0x039C
, "NU",         0x039D
, "XI",         0x039E
, "OMICRON",    0x039F
, "PI",         0x03A0
, "RHO",        0x03A1
, "SIGMA",      0x03A2
, "TAU",        0x03A3
, "UPSILON",    0x03A4
, "PHI",        0x03A5
, "CHI",        0x03A6
, "OMEGA",      0x03A7

, "alpha",      0x03B1
, "beta",       0x03B2
, "gamma",      0x03B3
, "delta",      0x03B4
, "epsilon",    0x03B5
, "zeta",       0x03B6
, "eta",        0x03B7
, "theta",      0x03B8
, "iota",       0x03B9
, "kappa",      0x03BA
, "lamda",      0x03BB
, "mu",         0x03BC
, "nu",         0x03BD
, "xi",         0x03BE
, "omicron",    0x03BF
, "pi",         0x03C0
, "rho",        0x03C1
, "stigma",     0x03C2
, "final_sigma",0x03C2
, "sigma",      0x03C3
, "tau",        0x03C4
, "upsilon",    0x03C5
, "phi",        0x03C6
, "chi",        0x03C7
, "psi",        0x03C8
, "omega",      0x03C9
, "", 0
};

static const char* get_unicode_number( const char* hex, unsigned long* p_ucode )
{
    unsigned long ucode = 0;
    const char* p = hex;
    for (;;){
        switch (*p){
            case '0':
                ucode *= 16;
                ucode += 0;
                break;
            case '1':
                ucode *= 16;
                ucode += 1;
                break;
            case '2':
                ucode *= 16;
                ucode += 2;
                break;
            case '3':
                ucode *= 16;
                ucode += 3;
                break;
            case '4':
                ucode *= 16;
                ucode += 4;
                break;
            case '5':
                ucode *= 16;
                ucode += 5;
                break;
            case '6':
                ucode *= 16;
                ucode += 6;
                break;
            case '7':
                ucode *= 16;
                ucode += 7;
                break;
            case '8':
                ucode *= 16;
                ucode += 8;
                break;
            case '9':
                ucode *= 16;
                ucode += 9;
                break;
            case 'a':
                ucode *= 16;
                ucode += 10;
                break;
            case 'A':
                ucode *= 16;
                ucode += 10;
                break;
            case 'b':
                ucode *= 16;
                ucode += 11;
                break;
            case 'B':
                ucode *= 16;
                ucode += 11;
                break;
            case 'c':
                ucode *= 16;
                ucode += 12;
                break;
            case 'C':
                ucode *= 16;
                ucode += 12;
                break;
            case 'd':
                ucode *= 16;
                ucode += 13;
                break;
            case 'D':
                ucode *= 16;
                ucode += 13;
                break;
            case 'e':
                ucode *= 16;
                ucode += 14;
                break;
            case 'E':
                ucode *= 16;
                ucode += 14;
                break;
            case 'f':
                ucode *= 16;
                ucode += 15;
                break;
            case 'F':
                ucode *= 16;
                ucode += 15;
                break;
            default:
                *p_ucode = ucode;
                return p;
        }
        p++;
    }
}

/* Converts a sequence %{U+1234} or %{epsilon} to unicode caracters */
const char* char2unicode( const char* text, unsigned long* unicode )
{
    const char* p = &(text[0]);
    const char* p1 = p;
    char symbol[32];
    int i;

    if (p == nullptr) return nullptr;

    if (*p == '%' && *(p + 1) == '{'){
        p += 2;
        p1 = p;
        i = 0;
        while (*p1 != '}' && i < 31){
            if (*p1 == '\0'){
                *unicode = *p;
                return p + 1;
            }
            symbol[i] = *p1;
            p1++;
            i++;
        }
        symbol[i] = '\0';

        i = 0;
        while (utable[i].unicode != 0){
            if (strcmp( symbol, utable[i].name ) == 0){
                *unicode = utable[i].unicode;
                return p1 + 1;
            }
            i++;
        }
        *unicode = *p;
        return p + 1;
    }
    else if (*p == '%' && (*(p + 1) == 'U' || *(p + 1) == 'u') && *(p + 2) == '+'){
        p += 3;
        p = get_unicode_number( p, unicode );
        return p;
    }
    else{
        *unicode = (unsigned char)(*p);
        return p + 1;
    }
}
