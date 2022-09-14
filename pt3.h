
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

typedef struct PT3Channel
{
    unsigned char PsInOr;
    unsigned char PsInSm;
    signed char CrAmSl;
    unsigned char CrNsSl;
    signed char CrEnSl;
    unsigned char TSlCnt;
    unsigned short CrTnSl;
    unsigned short TnAcc;
    unsigned char COnOff;
    unsigned char OnOffD;
    unsigned char OffOnD;       // IX for PTDECOD here (+12)
    const unsigned char* OrnPtr;
    const unsigned char* SamPtr;
    unsigned char NNtSkp;
    unsigned char Note;
    unsigned char SlToNt;
    unsigned char Env_En;       // FIXME: bool
    unsigned char Flags;
    unsigned char TnSlDl;       // Enabled - 0, SimpleGliss - 2
    unsigned short TSlStp;
    unsigned short TnDelt;
    unsigned char NtSkCn;
    unsigned char Volume;
} PT3Channel;

typedef struct PT3
{
    const unsigned char* MODADDR;
    const unsigned char* currentPos;
    const unsigned char* AdInPtA;
    const unsigned char* AdInPtB;
    const unsigned char* AdInPtC;

    const unsigned short* SamPtrs;
    unsigned char PrNote;
    unsigned short PrSlide;
    unsigned char delayValue;
    const unsigned short* OrnPtrs;
    unsigned short CSP;
    signed char AddToEn;
    const unsigned char* LPosPtr;
    const unsigned short* PatsPtr;
    unsigned short CurESld;
    unsigned char Env_Del;
    unsigned short ESldAdd;
    unsigned char Noise_Base;
    unsigned char AddToNs;

    // PT3_VARS:
    PT3Channel ChanA;
    PT3Channel ChanB;
    PT3Channel ChanC;
    unsigned char DelyCnt;
    unsigned char CurEDel;
    AYRegs AY;
    // PT3_VARSEND:
} PT3;

void PT3_init(PT3* pt3, const unsigned char* data);
void PT3_mute(PT3* pt3);
void PT3_play(PT3* pt3);
