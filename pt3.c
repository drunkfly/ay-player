/*
 * Based on Vortex Tracker II v1.0 PT3 player for ZX Spectrum
 * (c)2004,2007 S.V.Bulba <vorobey@mail.khstu.ru>
 * http://bulba.untergrund.net (http://bulba.at.kz)
 */
#include "pt3.h"
#include <assert.h>
#include <string.h>

static const unsigned char PT3_EMPTYSAMORN[] = { 0, 1, 0, 0x90 };

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

void PT3_init(PT3* pt3, const unsigned char* data)
{
    pt3->MODADDR = data;

    data += 100;
    pt3->delayValue = *data;

    const unsigned char* IX = data;

    data += 100;
    pt3->currentPos = data;

    pt3->LPosPtr = data + *(IX+102-100) + 1;  // song loop;

    unsigned short offset = *(IX+103-100) | (*(IX+104-100) << 8);
    pt3->PatsPtr = (unsigned short*)(pt3->MODADDR + offset);
    pt3->OrnPtrs = (unsigned short*)(pt3->MODADDR + 169);
    pt3->SamPtrs = (unsigned short*)(pt3->MODADDR + 105);

    memset(&pt3->ChanA, 0, sizeof(pt3->ChanA));
    memset(&pt3->ChanB, 0, sizeof(pt3->ChanB));
    memset(&pt3->ChanC, 0, sizeof(pt3->ChanC));
    pt3->DelyCnt = 0;
    pt3->CurEDel = 0;
    memset(&pt3->AY, 0, sizeof(pt3->AY));

    pt3->DelyCnt = 1;

    // H - CHP.Volume, L - CHP.NtSkCn
    pt3->ChanA.NtSkCn = 0x01;
    pt3->ChanA.Volume = 0xf0;
    pt3->ChanB.NtSkCn = 0x01;
    pt3->ChanB.Volume = 0xf0;
    pt3->ChanC.NtSkCn = 0x01;
    pt3->ChanC.Volume = 0xf0;

    pt3->AdInPtA = PT3_EMPTYSAMORN;     // ptr to zero
    pt3->ChanA.OrnPtr = PT3_EMPTYSAMORN; // ornament 0 is "0,1,0"
    pt3->ChanB.OrnPtr = PT3_EMPTYSAMORN; // in all versions from
    pt3->ChanC.OrnPtr = PT3_EMPTYSAMORN; // 3.xx to 3.6x and VTII
    pt3->ChanA.SamPtr = PT3_EMPTYSAMORN;
    pt3->ChanB.SamPtr = PT3_EMPTYSAMORN;
    pt3->ChanC.SamPtr = PT3_EMPTYSAMORN;
}

void PT3_mute(PT3* pt3)
{
    pt3->AY.amplA = 0;
    pt3->AY.amplB = 0;
    pt3->AY.mixer = 0x3f;
}

static void PT3_PTDECOD(PT3* pt3, const unsigned char** pOffset, PT3Channel* ix);
static void PT3_CHREGS(PT3* pt3, unsigned char* ampl, unsigned short* PT3_TonA_HL, PT3Channel* ix);

void PT3_play(PT3* pt3)
{
    pt3->AddToEn = 0;
    pt3->AY.mixer = 0;
    pt3->AY.envTp = 0xff;

    --pt3->DelyCnt;
    if (pt3->DelyCnt == 0) {
        --pt3->ChanA.NtSkCn;
        if (pt3->ChanA.NtSkCn == 0) {
            unsigned char a = *pt3->AdInPtA;
            if (a == 0) {
                const unsigned char* HL = pt3->currentPos + 1;
                if (*HL == 0xff) {
                    HL = pt3->LPosPtr;
                    // Stop here if not looping
                }
                pt3->currentPos = HL;
                a = *HL;

                const unsigned short* hl = &pt3->PatsPtr[a];
                pt3->AdInPtA = pt3->MODADDR + *hl++;
                pt3->AdInPtB = pt3->MODADDR + *hl++;
                pt3->AdInPtC = pt3->MODADDR + *hl++;

                pt3->Noise_Base = 0;
            }

            PT3_PTDECOD(pt3, &pt3->AdInPtA, &pt3->ChanA);
        }

        --pt3->ChanB.NtSkCn;
        if (pt3->ChanB.NtSkCn == 0)
            PT3_PTDECOD(pt3, &pt3->AdInPtB, &pt3->ChanB);

        --pt3->ChanC.NtSkCn;
        if (pt3->ChanC.NtSkCn == 0)
            PT3_PTDECOD(pt3, &pt3->AdInPtC, &pt3->ChanC);

        pt3->DelyCnt = pt3->delayValue;
    }

    PT3_CHREGS(pt3, &pt3->AY.amplA, &pt3->AY.tonA, &pt3->ChanA);
    PT3_CHREGS(pt3, &pt3->AY.amplB, &pt3->AY.tonB, &pt3->ChanB);
    PT3_CHREGS(pt3, &pt3->AY.amplC, &pt3->AY.tonC, &pt3->ChanC);

    pt3->AY.noise = (pt3->AddToNs + pt3->Noise_Base) & 0x1f;
    pt3->AY.env = (pt3->CurESld + pt3->AY.envBase + pt3->AddToEn) & 0x0fff;

    if (pt3->CurEDel == 0)
        return;
    --pt3->CurEDel;
    if (pt3->CurEDel != 0)
        return;

    pt3->CurEDel = pt3->Env_Del;
    pt3->CurESld += pt3->ESldAdd;
}

static void PT3_C_NOP()
{
}

static void PT3_C_GLISS(const unsigned char** pOffset, PT3Channel* ix)
{
    ix->Flags |= 4;

    unsigned char A = **pOffset;
    ++*pOffset;
    ix->TnSlDl = A;

    if (A == 0)
        ++A;
    ix->TSlCnt = A;

    unsigned char LO = **pOffset;
    ++*pOffset;
    unsigned char HI = **pOffset;
    ++*pOffset;

    ix->TSlStp = HI * 256 + LO;
    ix->COnOff = 0;
}

static void PT3_C_PORTM(PT3* pt3, const unsigned char** pOffset, PT3Channel* ix)
{
    ix->Flags &= ~4;

    unsigned char A = **pOffset;
    ++*pOffset;

    // SKIP PRECALCULATED TONE DELTA
    // (BECAUSE CANNOT BE RIGHT AFTER PT3 COMPILATION)
    ++*pOffset;
    ++*pOffset;

    ix->TnSlDl = A;
    ix->TSlCnt = A;

    A = ix->Note;
    ix->SlToNt = A;
    short HL = (short)NoteTable[A];

    A = pt3->PrNote;
    ix->Note = A;
    unsigned short DE = NoteTable[A];

    HL -= DE;

    ix->TnDelt = HL;
    DE = pt3->PrSlide;
    ix->CrTnSl = DE;

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

    ix->TSlStp = value;
    ix->COnOff = 0;
}

static void PT3_C_SMPOS(const unsigned char** pOffset, PT3Channel* ix)
{
    unsigned char A = **pOffset;
    ++*pOffset;
    ix->PsInSm = A;
}

static void PT3_C_ORPOS(const unsigned char** pOffset, PT3Channel* ix)
{
    unsigned char A = **pOffset;
    ++*pOffset;
    ix->PsInOr = A;
}

static void PT3_C_VIBRT(const unsigned char** pOffset, PT3Channel* ix)
{
    unsigned char A = **pOffset;
    ++*pOffset;
    ix->OnOffD = A;
    ix->COnOff = A;
    A = **pOffset;
    ++*pOffset;
    ix->OffOnD = A;

    ix->TSlCnt = 0;
    ix->CrTnSl = 0;
}

static void PT3_C_ENGLS(PT3* pt3, const unsigned char** pOffset, PT3Channel* ix)
{
    unsigned char A = **pOffset;
    ++*pOffset;

    pt3->Env_Del = A;
    pt3->CurEDel = A;

    unsigned char L = **pOffset;
    ++*pOffset;
    unsigned char H = **pOffset;
    ++*pOffset;
    pt3->ESldAdd = L + H * 256;
}

static void PT3_C_DELAY(PT3* pt3, const unsigned char** pOffset, PT3Channel* ix)
{
    pt3->delayValue = **pOffset;
    ++*pOffset;
}

static void doStack(PT3* pt3, unsigned char* stack, int stackSize,
    const unsigned char** pOffset, PT3Channel* ix)
{
    while (stackSize > 0) {
        switch (stack[--stackSize]) {
            case 0: PT3_C_NOP(); break;
            case 1: PT3_C_GLISS(pOffset, ix); break;
            case 2: PT3_C_PORTM(pt3, pOffset, ix); break;
            case 3: PT3_C_SMPOS(pOffset, ix); break;
            case 4: PT3_C_ORPOS(pOffset, ix); break;
            case 5: PT3_C_VIBRT(pOffset, ix); break;
            case 6: PT3_C_NOP(); break;
            case 7: PT3_C_NOP(); break;
            case 8: PT3_C_ENGLS(pt3, pOffset, ix); break;
            case 9: PT3_C_DELAY(pt3, pOffset, ix); break;
            default: assert(0);
        }
    }
}

static void PT3_SETENV(PT3* pt3, PT3Channel* ix, const unsigned char** pOffset, unsigned char a)
{
    ix->Env_En = 0x10;
    pt3->AY.envTp = a;

    unsigned char H = **pOffset;
    ++*pOffset;
    unsigned char L = **pOffset;
    ++*pOffset;
    pt3->AY.envBase = L + H * 256;

    ix->PsInOr = 0;
    pt3->CurEDel = 0;
    pt3->CurESld = 0;
}

static void PT3_SETORN(PT3* pt3, PT3Channel* ix, unsigned char a)
{
    ix->PsInOr = 0;
    ix->OrnPtr = pt3->MODADDR + pt3->OrnPtrs[a];
}

static void PT3_PTDECOD(PT3* pt3, const unsigned char** pOffset, PT3Channel* ix)
{
    enum { STACK_SIZE = 10 };
    unsigned char stack[STACK_SIZE];
    int stackSize = 0;

    pt3->PrNote = ix->Note;
    pt3->PrSlide = ix->CrTnSl;

    for (;;) {
        unsigned char a = **pOffset;
        ++*pOffset;
        if (a >= 0xf0) {    /* ornament + sample */
            ix->Env_En = 0;
            PT3_SETORN(pt3, ix, a - 0xf0);
            a = **pOffset;
            ++*pOffset;
            a >>= 1;
            goto PT3_PD_SAM;
        }
        if (a == 0xd0) {    /* end line */
          PD_FIN:
            ix->NtSkCn = ix->NNtSkp;
            doStack(pt3, stack, stackSize, pOffset, ix);
            return;
        }
        if (a > 0xd0) {      /* sample number */
            a -= 0xd0;
            goto PT3_PD_SAM;
        }
        if (a == 0xc0) {    /* pause + end line */
            ix->Flags &= 0xfe;
          PT3_PD_RES:
            ix->OnOffD = 0;
            ix->COnOff = 0;
            ix->TnAcc = 0;
            ix->CrTnSl = 0;
            ix->TSlCnt = 0;
            ix->CrEnSl = 0;
            ix->CrNsSl = 0;
            ix->CrAmSl = 0;
            ix->PsInSm = 0;
            ix->PsInOr = 0;
            goto PD_FIN;
        }
        if (a > 0xc0) {     /* volume */
            a -= 0xc0;
            a <<= 4;
            ix->Volume = a;
            continue;
        }
        if (a == 0xb0) {    /* envelope off */
            ix->Env_En = 0;
            ix->PsInOr = 0;
            continue;
        }
        if (a == 0xb1) {    /* don't analyze channel for N lines */
            ix->NNtSkp = **pOffset;
            ++*pOffset;
            continue;
        }
        if (a > 0xb1) {
            PT3_SETENV(pt3, ix, pOffset, a - 0xb1);
            continue;
        }
        if (a >= 0x50) {    /* note */
            ix->Note = a - 0x50;
            ix->Flags |= 1;
            goto PT3_PD_RES;
        }
        if (a >= 0x40)  {   /* ornament */
            PT3_SETORN(pt3, ix, a - 0x40);
            continue;
        }
        if (a >= 0x20) {    /* noise */
            pt3->Noise_Base = a - 0x20;
            continue;
        }
        if (a >= 0x10) {
            a -= 0x10;
            ix->Env_En = a;
            ix->PsInOr = a;
            if (a != 0)
                PT3_SETENV(pt3, ix, pOffset, a);
            a = **pOffset;
            ++*pOffset;
            a >>= 1;
          PT3_PD_SAM:
            ix->SamPtr = pt3->MODADDR + pt3->SamPtrs[a];
            continue;
        }

        assert(a != 0);

        assert(stackSize < STACK_SIZE);
        stack[stackSize] = a;
        ++stackSize;

        continue;
    }
}

static void PT3_CHREGS(PT3* pt3, unsigned char* ampl, unsigned short* PT3_TonA_HL, PT3Channel* ix)
{
    *ampl = 0;
    unsigned char A = 0;

    if ((ix->Flags & 1) != 0) {
        const unsigned char* SP = ix->OrnPtr;
        unsigned char E = *SP++;
        unsigned char D = *SP++;

        A = ix->PsInOr;
        const unsigned char* HL = SP + A;
        ++A;
        if (A >= D)
            A = E;
        ix->PsInOr = A;

        signed char a = ix->Note;
        a += *HL;
        if (a < 0)
            a = 0;
        if (a > 95)
            a = 95;

        SP = ix->SamPtr;
        E = *SP++;
        D = *SP++;

        unsigned char B = A = ix->PsInSm;
        A *= 4;
        SP += A;

        A = B;
        ++A;
        if (A >= D)
            A = E;
        ix->PsInSm = A;

        unsigned char C = *SP++;
        B = *SP++;

        unsigned char L = *SP++;
        unsigned char H = *SP++;

        unsigned short HL_ = (L + H * 256) + ix->TnAcc;
        if (B & 0x40)
            ix->TnAcc = HL_;

        unsigned short hl_ = NoteTable[a];
        hl_ += HL_;
        hl_ += ix->CrTnSl;
        *PT3_TonA_HL = hl_ & 0x0fff;

        if (ix->TSlCnt == 0)
            goto PT3_CH_AMP;

        --ix->TSlCnt;
        if (ix->TSlCnt != 0)
            goto PT3_CH_AMP;

        ix->TSlCnt = ix->TnSlDl;
        short hl = ix->TSlStp;
        hl += ix->CrTnSl;
        ix->CrTnSl = hl;

        if (ix->Flags & 0x04)
            goto PT3_CH_AMP;

        short de = ix->TnDelt;
        if (ix->TSlStp > 255) {
            short tmp = de;
            de = hl;
            hl = tmp;
        }

        hl -= de;
        if (hl < 0)
            goto PT3_CH_AMP;

        ix->Note = ix->SlToNt;
        ix->TSlCnt = 0;
        ix->CrTnSl = 0;

      PT3_CH_AMP:
        a = ix->CrAmSl;
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
        ix->CrAmSl = a;
      PT3_CH_NOAM:
        L = a;
        a = B & 15;
        a += L;
        if (a < 0)
            a = 0;
        if (a > 15)
            a = 15;
        A = VolumeTable[a | ix->Volume];
        if ((C & 1) == 0)
            A |= ix->Env_En;
        *ampl = A & 0x1f;
        A = C;
        if ((B & 0x80) != 0) {
            a = (signed char)A;
            // 6-bit signed value
            a <<= 2;    // move 6th bit to 8th position
            a >>= 3;    // arithmetic shift right to preserve sign
            a += ix->CrEnSl;  // SEE COMMENT BELOW
            if ((B & 0x20) != 0)
                ix->CrEnSl = a;
            pt3->AddToEn += a;   // BUG IN PT3 - NEED WORD HERE. FIX IT IN NEXT VERSION?
        } else {
            A >>= 1;    // rra
            A += ix->CrNsSl;
            pt3->AddToNs = A;
            if ((B & 0x20) != 0)
                ix->CrNsSl = A;
        }
        A = B;
        A >>= 1; // rra
        A &= 0x48;
    }

    A |= pt3->AY.mixer;
    A >>= 1; // rrca
    pt3->AY.mixer = A;

    A = 0;
    if (ix->COnOff == 0)
        return;
    --ix->COnOff;
    if (ix->COnOff != 0)
        return;

    A ^= ix->Flags;
    ix->Flags = A;

    int CF = (A & 1) != 0;
    A = ix->OnOffD;
    if (!CF)
        A = ix->OffOnD;
    ix->COnOff = A;
}
