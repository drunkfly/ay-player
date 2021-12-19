
typedef struct AYRegs
{
    unsigned short tonA;
    unsigned short tonB;
    unsigned short tonC;
    unsigned char noise;
    unsigned char mixer;
    unsigned char amplA;
    unsigned char amplB;
    unsigned char amplC;
    unsigned short env;
    unsigned char envTp;
    unsigned short envBase; // move out of this struct
} AYRegs;
