/*
 * Based on Vortex Tracker II v1.0 PT3 player for ZX Spectrum
 * (c)2004,2007 S.V.Bulba <vorobey@mail.khstu.ru>
 * http://bulba.untergrund.net (http://bulba.at.kz)
 */
#include "pt3.h"
#include <assert.h>

#include <stdio.h> // REMOVEME
#include <stdlib.h> // REMOVEME
FILE* out; // REMOVEME

typedef struct Channel
{
    unsigned char PT3_CHP_PsInOr;
    unsigned char PT3_CHP_PsInSm;
    signed char PT3_CHP_CrAmSl;
    unsigned char PT3_CHP_CrNsSl;
    signed char PT3_CHP_CrEnSl;
    unsigned char PT3_CHP_TSlCnt;
    unsigned short PT3_CHP_CrTnSl;
    unsigned short PT3_CHP_TnAcc;
    unsigned char PT3_CHP_COnOff;
    unsigned char PT3_CHP_OnOffD;
    unsigned char PT3_CHP_OffOnD;       // IX for PTDECOD here (+12)
    const unsigned char* PT3_CHP_OrnPtr;
    const unsigned char* PT3_CHP_SamPtr;
    unsigned char PT3_CHP_NNtSkp;
    unsigned char PT3_CHP_Note;
    unsigned char PT3_CHP_SlToNt;
    unsigned char PT3_CHP_Env_En; // FIXME: bool
    unsigned char PT3_CHP_Flags;
    unsigned char PT3_CHP_TnSlDl;       // Enabled - 0, SimpleGliss - 2
    unsigned short PT3_CHP_TSlStp;
    unsigned short PT3_CHP_TnDelt;
    unsigned char PT3_CHP_NtSkCn;
    unsigned char PT3_CHP_Volume;
} Channel;

unsigned char PT3_EMPTYSAMORN[] = { 0, 1, 0, 0x90 };

const unsigned char* PT3_MODADDR;
const unsigned char* PT3_currentPos;
const unsigned char* PT3_AdInPtA;
const unsigned char* PT3_AdInPtB;
const unsigned char* PT3_AdInPtC;

const unsigned short* PT3_SamPtrs;
unsigned char PT3_PrNote;
unsigned short PT3_PrSlide;
unsigned char PT3_delayValue;
const unsigned short* PT3_OrnPtrs;
unsigned short PT3_CSP;
signed char PT3_AddToEn;
const unsigned char* PT3_LPosPtr;
const unsigned short* PT3_PatsPtr;
unsigned short PT3_CurESld;
unsigned char PT3_Env_Del;
unsigned short PT3_ESldAdd;
unsigned char PT3_Noise_Base;
unsigned char PT3_AddToNs;

// PT3_VARS:
Channel PT3_ChanA;
Channel PT3_ChanB;
Channel PT3_ChanC;
unsigned char PT3_DelyCnt;
unsigned char PT3_CurEDel;
AYRegs AY;
// PT3_VARSEND:

static const unsigned char VolumeTable[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
    0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3,
    0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4,
    0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5,
    0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 6, 6,
    0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7,
    0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
    0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 7, 7, 8, 8, 9,
    0, 1, 1, 2, 3, 3, 4, 5, 5, 6, 7, 7, 8, 9, 9,10,
    0, 1, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 9,10,10,11,
    0, 1, 2, 2, 3, 4, 5, 6, 6, 7, 8, 9,10,10,11,12,
    0, 1, 2, 3, 3, 4, 5, 6, 7, 8, 9,10,10,11,12,13,
    0, 1, 2, 3, 4, 5, 6, 7, 7, 8, 9,10,11,12,13,14,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
};

static const unsigned short NoteTable[] = {
    0xd10,0xc55,0xba4,0xafc,0xa5f,0x9ca,0x93d,0x8b8,0x83b,0x7c5,0x755,0x6ec,
    0x688,0x62a,0x5d2,0x57e,0x52f,0x4e5,0x49e,0x45c,0x41d,0x3e2,0x3ab,0x376,
    0x344,0x315,0x2e9,0x2bf,0x298,0x272,0x24f,0x22e,0x20f,0x1f1,0x1d5,0x1bb,
    0x1a2,0x18b,0x174,0x160,0x14c,0x139,0x128,0x117,0x107,0x0f9,0x0eb,0x0dd,
    0x0d1,0x0c5,0x0ba,0x0b0,0x0a6,0x09d,0x094,0x08c,0x084,0x07c,0x075,0x06f,
    0x069,0x063,0x05d,0x058,0x053,0x04e,0x04a,0x046,0x042,0x03e,0x03b,0x037,
    0x034,0x031,0x02f,0x02c,0x029,0x027,0x025,0x023,0x021,0x01f,0x01d,0x01c,
    0x01a,0x019,0x017,0x016,0x015,0x014,0x012,0x011,0x010,0x00f,0x00e,0x00d,
};

void PT3_init(const unsigned char* data)
{
    PT3_MODADDR = data;

    data += 100;
    PT3_delayValue = *data;

    const unsigned char* IX = data;

    data += 100;
    PT3_currentPos = data;

    PT3_LPosPtr = data + *(IX+102-100) + 1;  // song loop;

    unsigned short offset = *(IX+103-100) | (*(IX+104-100) << 8);
    PT3_PatsPtr = (unsigned short*)(PT3_MODADDR + offset);
    PT3_OrnPtrs = (unsigned short*)(PT3_MODADDR + 169);
    PT3_SamPtrs = (unsigned short*)(PT3_MODADDR + 105);

    memset(&PT3_ChanA, 0, sizeof(PT3_ChanA));
    memset(&PT3_ChanB, 0, sizeof(PT3_ChanB));
    memset(&PT3_ChanC, 0, sizeof(PT3_ChanC));
    PT3_DelyCnt = 0;
    PT3_CurEDel = 0;
    memset(&AY, 0, sizeof(AY));

    PT3_DelyCnt = 1;

    // H - CHP.Volume, L - CHP.NtSkCn
    PT3_ChanA.PT3_CHP_NtSkCn = 0x01;
    PT3_ChanA.PT3_CHP_Volume = 0xf0;
    PT3_ChanB.PT3_CHP_NtSkCn = 0x01;
    PT3_ChanB.PT3_CHP_Volume = 0xf0;
    PT3_ChanC.PT3_CHP_NtSkCn = 0x01;
    PT3_ChanC.PT3_CHP_Volume = 0xf0;

    PT3_AdInPtA = PT3_EMPTYSAMORN;     // ptr to zero
    PT3_ChanA.PT3_CHP_OrnPtr = PT3_EMPTYSAMORN; // ornament 0 is "0,1,0"
    PT3_ChanB.PT3_CHP_OrnPtr = PT3_EMPTYSAMORN; // in all versions from
    PT3_ChanC.PT3_CHP_OrnPtr = PT3_EMPTYSAMORN; // 3.xx to 3.6x and VTII
    PT3_ChanA.PT3_CHP_SamPtr = PT3_EMPTYSAMORN;
    PT3_ChanB.PT3_CHP_SamPtr = PT3_EMPTYSAMORN;
    PT3_ChanC.PT3_CHP_SamPtr = PT3_EMPTYSAMORN;
    //WriteAY();
}

void PT3_mute()
{
    AY.amplA = 0;
    AY.amplB = 0;
    AY.mixer = 0x3f;
    //WriteAY();
}

void PT3_PTDECOD(const unsigned char** pOffset, Channel* ix);
void PT3_CHREGS(unsigned char* ampl, unsigned short* PT3_TonA_HL, Channel* ix);

void PT3_play()
{
    PT3_AddToEn = 0;
    AY.mixer = 0;
    AY.envTp = 0xff;

    --PT3_DelyCnt;
    if (PT3_DelyCnt == 0) {
        --PT3_ChanA.PT3_CHP_NtSkCn;
        if (PT3_ChanA.PT3_CHP_NtSkCn == 0) {
            unsigned char a = *PT3_AdInPtA;
            if (a == 0) {
                const unsigned char* HL = PT3_currentPos + 1;
                if (*HL == 0xff) {
                    HL = PT3_LPosPtr;
fclose(out); // REMOVEME
exit(0); // REMOVEME
                }
                PT3_currentPos = HL;
                a = *HL;

                const unsigned short* hl = &PT3_PatsPtr[a];
                PT3_AdInPtA = PT3_MODADDR + *hl++;
                PT3_AdInPtB = PT3_MODADDR + *hl++;
                PT3_AdInPtC = PT3_MODADDR + *hl++;

                PT3_Noise_Base = 0;
            }

            PT3_PTDECOD(&PT3_AdInPtA, &PT3_ChanA);
        }

        --PT3_ChanB.PT3_CHP_NtSkCn;
        if (PT3_ChanB.PT3_CHP_NtSkCn == 0)
            PT3_PTDECOD(&PT3_AdInPtB, &PT3_ChanB);

        --PT3_ChanC.PT3_CHP_NtSkCn;
        if (PT3_ChanC.PT3_CHP_NtSkCn == 0)
            PT3_PTDECOD(&PT3_AdInPtC, &PT3_ChanC);

        PT3_DelyCnt = PT3_delayValue;
    }

    PT3_CHREGS(&AY.amplA, &AY.tonA, &PT3_ChanA);
    PT3_CHREGS(&AY.amplB, &AY.tonB, &PT3_ChanB);
    PT3_CHREGS(&AY.amplC, &AY.tonC, &PT3_ChanC);

    AY.noise = (PT3_AddToNs + PT3_Noise_Base) & 0x1f;
    AY.env = (PT3_CurESld + AY.envBase + PT3_AddToEn) & 0x0fff;

    if (PT3_CurEDel == 0)
        return;
    --PT3_CurEDel;
    if (PT3_CurEDel != 0)
        return;

    PT3_CurEDel = PT3_Env_Del;
    PT3_CurESld += PT3_ESldAdd;
}

void PT3_C_NOP()
{
}

void PT3_C_GLISS(const unsigned char** pOffset, Channel* ix)
{
    ix->PT3_CHP_Flags |= 4;

    unsigned char A = **pOffset;
    ++*pOffset;
    ix->PT3_CHP_TnSlDl = A;

    if (A == 0)
        ++A;
    ix->PT3_CHP_TSlCnt = A;

    unsigned char LO = **pOffset;
    ++*pOffset;
    unsigned char HI = **pOffset;
    ++*pOffset;

    ix->PT3_CHP_TSlStp = HI * 256 + LO;
    ix->PT3_CHP_COnOff = 0;
}

void PT3_C_PORTM(const unsigned char** pOffset, Channel* ix)
{
    ix->PT3_CHP_Flags &= ~4;

    unsigned char A = **pOffset;
    ++*pOffset;

    // SKIP PRECALCULATED TONE DELTA
    // (BECAUSE CANNOT BE RIGHT AFTER PT3 COMPILATION)
    ++*pOffset;
    ++*pOffset;

    ix->PT3_CHP_TnSlDl = A;
    ix->PT3_CHP_TSlCnt = A;

    A = ix->PT3_CHP_Note;
    ix->PT3_CHP_SlToNt = A;
    short HL = (short)NoteTable[A];

    A = PT3_PrNote;
    ix->PT3_CHP_Note = A;
    unsigned short DE = NoteTable[A];

    HL -= DE;

    ix->PT3_CHP_TnDelt = HL;
    DE = PT3_PrSlide;
    ix->PT3_CHP_CrTnSl = DE;

    unsigned char LO = **pOffset;      // SIGNED TONE STEP
    ++*pOffset;
    unsigned char HI = **pOffset;
    ++*pOffset;

    short value = LO + HI * 256;

    if (HI != 0) {
        unsigned short tmp = DE;
        DE = HL;
        HL = tmp;
    }

    HL -= DE;
    if (HL < 0)
        value = -value;

    ix->PT3_CHP_TSlStp = value;
    ix->PT3_CHP_COnOff = 0;
}

void PT3_C_SMPOS(const unsigned char** pOffset, Channel* ix)
{
    unsigned char A = **pOffset;
    ++*pOffset;
    ix->PT3_CHP_PsInSm = A;
}

void PT3_C_ORPOS(const unsigned char** pOffset, Channel* ix)
{
    unsigned char A = **pOffset;
    ++*pOffset;
    ix->PT3_CHP_PsInOr = A;
}

void PT3_C_VIBRT(const unsigned char** pOffset, Channel* ix)
{
    unsigned char A = **pOffset;
    ++*pOffset;
    ix->PT3_CHP_OnOffD = A;
    ix->PT3_CHP_COnOff = A;
    A = **pOffset;
    ++*pOffset;
    ix->PT3_CHP_OffOnD = A;

    ix->PT3_CHP_TSlCnt = 0;
    ix->PT3_CHP_CrTnSl = 0;
}

void PT3_C_ENGLS(const unsigned char** pOffset, Channel* ix)
{
    unsigned char A = **pOffset;
    ++*pOffset;

    PT3_Env_Del = A;
    PT3_CurEDel = A;

    unsigned char L = **pOffset;
    ++*pOffset;
    unsigned char H = **pOffset;
    ++*pOffset;
    PT3_ESldAdd = L + H * 256;
}

void PT3_C_DELAY(const unsigned char** pOffset, Channel* ix)
{
    PT3_delayValue = **pOffset;
    ++*pOffset;
}

static void doStack(unsigned char* stack, int stackSize,
    const unsigned char** pOffset, Channel* ix)
{
    while (stackSize > 0) {
        switch (stack[--stackSize]) {
            case 0: PT3_C_NOP(); break;
            case 1: PT3_C_GLISS(pOffset, ix); break;
            case 2: PT3_C_PORTM(pOffset, ix); break;
            case 3: PT3_C_SMPOS(pOffset, ix); break;
            case 4: PT3_C_ORPOS(pOffset, ix); break;
            case 5: PT3_C_VIBRT(pOffset, ix); break;
            case 6: PT3_C_NOP(); break;
            case 7: PT3_C_NOP(); break;
            case 8: PT3_C_ENGLS(pOffset, ix); break;
            case 9: PT3_C_DELAY(pOffset, ix); break;
            default: assert(0);
        }
    }
}

void PT3_SETENV(Channel* ix, const unsigned char** pOffset, unsigned char a)
{
    ix->PT3_CHP_Env_En = 0x10;
    AY.envTp = a;

    unsigned char H = **pOffset;
    ++*pOffset;
    unsigned char L = **pOffset;
    ++*pOffset;
    AY.envBase = L + H * 256;

    ix->PT3_CHP_PsInOr = 0;
    PT3_CurEDel = 0;
    PT3_CurESld = 0;
}

void PT3_SETORN(Channel* ix, unsigned char a)
{
    ix->PT3_CHP_PsInOr = 0;
    ix->PT3_CHP_OrnPtr = PT3_MODADDR + PT3_OrnPtrs[a];
}

void PT3_PTDECOD(const unsigned char** pOffset, Channel* ix)
{
    enum { STACK_SIZE = 10 };
    unsigned char stack[STACK_SIZE];
    int stackSize = 0;

    PT3_PrNote = ix->PT3_CHP_Note;
    PT3_PrSlide = ix->PT3_CHP_CrTnSl;

    for (;;) {
        unsigned char a = **pOffset;
        ++*pOffset;
        if (a >= 0xf0) {    /* ornament + sample */
            ix->PT3_CHP_Env_En = 0;
            PT3_SETORN(ix, a - 0xf0);
            a = **pOffset;
            ++*pOffset;
            a >>= 1;
            goto PT3_PD_SAM;
        }
        if (a == 0xd0) {    /* end line */
          PD_FIN:
            ix->PT3_CHP_NtSkCn = ix->PT3_CHP_NNtSkp;
            doStack(stack, stackSize, pOffset, ix);
            return;
        }
        if (a > 0xd0) {      /* sample number */
            a -= 0xd0;
            goto PT3_PD_SAM;
        }
        if (a == 0xc0) {    /* pause + end line */
            ix->PT3_CHP_Flags &= 0xfe;
          PT3_PD_RES:
            ix->PT3_CHP_OnOffD = 0;
            ix->PT3_CHP_COnOff = 0;
            ix->PT3_CHP_TnAcc = 0;
            ix->PT3_CHP_CrTnSl = 0;
            ix->PT3_CHP_TSlCnt = 0;
            ix->PT3_CHP_CrEnSl = 0;
            ix->PT3_CHP_CrNsSl = 0;
            ix->PT3_CHP_CrAmSl = 0;
            ix->PT3_CHP_PsInSm = 0;
            ix->PT3_CHP_PsInOr = 0;
            goto PD_FIN;
        }
        if (a > 0xc0) {     /* volume */
            a -= 0xc0;
            a <<= 4;
            ix->PT3_CHP_Volume = a;
            continue;
        }
        if (a == 0xb0) {    /* envelope off */
            ix->PT3_CHP_Env_En = 0;
            ix->PT3_CHP_PsInOr = 0;
            continue;
        }
        if (a == 0xb1) {    /* don't analyze channel for N lines */
            ix->PT3_CHP_NNtSkp = **pOffset;
            ++*pOffset;
            continue;
        }
        if (a > 0xb1) {
            PT3_SETENV(ix, pOffset, a - 0xb1);
            continue;
        }
        if (a >= 0x50) {    /* note */
            ix->PT3_CHP_Note = a - 0x50;
            ix->PT3_CHP_Flags |= 1;
            goto PT3_PD_RES;
        }
        if (a >= 0x40)  {   /* ornament */
            PT3_SETORN(ix, a - 0x40);
            continue;
        }
        if (a >= 0x20) {    /* noise */
            PT3_Noise_Base = a - 0x20;
            continue;
        }
        if (a >= 0x10) {
            a -= 0x10;
            ix->PT3_CHP_Env_En = a;
            ix->PT3_CHP_PsInOr = a;
            if (a != 0)
                PT3_SETENV(ix, pOffset, a);
            a = **pOffset;
            ++*pOffset;
            a >>= 1;
          PT3_PD_SAM:
            ix->PT3_CHP_SamPtr = PT3_MODADDR + PT3_SamPtrs[a];
            continue;
        }

        assert(stackSize < STACK_SIZE);
        stack[stackSize] = a;
        ++stackSize;

        continue;
    }
}

void PT3_CHREGS(unsigned char* ampl, unsigned short* PT3_TonA_HL, Channel* ix)
{
    *ampl = 0;
    unsigned char A = 0;

    if ((ix->PT3_CHP_Flags & 1) != 0) {
        const unsigned char* SP = ix->PT3_CHP_OrnPtr;
        unsigned char E = *SP++;
        unsigned char D = *SP++;

        A = ix->PT3_CHP_PsInOr;
        const unsigned char* HL = SP + A;
        ++A;
        if (A >= D)
            A = E;
        ix->PT3_CHP_PsInOr = A;

        signed char a = ix->PT3_CHP_Note;
        a += *HL;
        if (a < 0)
            a = 0;
        if (a > 95)
            a = 95;

        SP = ix->PT3_CHP_SamPtr;
        E = *SP++;
        D = *SP++;

        unsigned char B = A = ix->PT3_CHP_PsInSm;
        A *= 4;
        SP += A;

        A = B;
        ++A;
        if (A >= D)
            A = E;
        ix->PT3_CHP_PsInSm = A;

        unsigned char C = *SP++;
        B = *SP++;

        unsigned char L = *SP++;
        unsigned char H = *SP++;

        unsigned short HL_ = (L + H * 256) + ix->PT3_CHP_TnAcc;
        if (B & 0x40)
            ix->PT3_CHP_TnAcc = HL_;

        unsigned short hl_ = NoteTable[a];
        hl_ += HL_;
        hl_ += ix->PT3_CHP_CrTnSl;
        *PT3_TonA_HL = hl_ & 0x0fff;

        if (ix->PT3_CHP_TSlCnt == 0)
            goto PT3_CH_AMP;

        --ix->PT3_CHP_TSlCnt;
        if (ix->PT3_CHP_TSlCnt != 0)
            goto PT3_CH_AMP;

        ix->PT3_CHP_TSlCnt = ix->PT3_CHP_TnSlDl;
        short hl = ix->PT3_CHP_TSlStp;
        hl += ix->PT3_CHP_CrTnSl;
        ix->PT3_CHP_CrTnSl = hl;

        if (ix->PT3_CHP_Flags & 0x04)
            goto PT3_CH_AMP;

        short de = ix->PT3_CHP_TnDelt;
        if (ix->PT3_CHP_TSlStp > 255) {
            short tmp = de;
            de = hl;
            hl = tmp;
        }

        hl -= de;
        if (hl < 0)
            goto PT3_CH_AMP;

        ix->PT3_CHP_Note = ix->PT3_CHP_SlToNt;
        ix->PT3_CHP_TSlCnt = 0;
        ix->PT3_CHP_CrTnSl = 0;

      PT3_CH_AMP:
        a = ix->PT3_CHP_CrAmSl;
        if ((C & 0x80) == 0)
            goto PT3_CH_NOAM;
        if ((C & 0x40) == 0) {
            if (a == -15)
                goto PT3_CH_NOAM;
            --a;
            goto PT3_CH_SVAM;
        }
        if (a == 15)
            goto PT3_CH_NOAM;
        ++a;
      PT3_CH_SVAM:
        ix->PT3_CHP_CrAmSl = a;
      PT3_CH_NOAM:
        L = a;
        a = B & 15;
        a += L;
        if (a < 0)
            a = 0;
        if (a > 15)
            a = 15;
        A = VolumeTable[a | ix->PT3_CHP_Volume];
        if ((C & 1) == 0)
            A |= ix->PT3_CHP_Env_En;
        *ampl = A & 0x1f;
        A = C;
        if ((B & 0x80) != 0) {
            a = (signed char)A;
            // 6-bit signed value
            a <<= 2;    // move 6th bit to 8th position
            a >>= 3;    // arithmetic shift right to preserve sign
            a += ix->PT3_CHP_CrEnSl;  // SEE COMMENT BELOW
            if ((B & 0x20) != 0)
                ix->PT3_CHP_CrEnSl = a;
            PT3_AddToEn += a;   // BUG IN PT3 - NEED WORD HERE. FIX IT IN NEXT VERSION?
        } else {
            A >>= 1;    // rra
            A += ix->PT3_CHP_CrNsSl;
            PT3_AddToNs = A;
            if ((B & 0x20) != 0)
                ix->PT3_CHP_CrNsSl = A;
        }
        A = B;
        A >>= 1; // rra
        A &= 0x48;
    }

    A |= AY.mixer;
    A >>= 1; // rrca
    AY.mixer = A;

    if (ix->PT3_CHP_COnOff == 0)
        return;
    --ix->PT3_CHP_COnOff;
    if (ix->PT3_CHP_COnOff != 0)
        return;

    A = 0;
    A ^= ix->PT3_CHP_Flags;
    ix->PT3_CHP_Flags = A;

    int CF = (A & 1) != 0;
    A = ix->PT3_CHP_OnOffD;
    if (!CF)
        A = ix->PT3_CHP_OffOnD;
    ix->PT3_CHP_COnOff = A;
}
