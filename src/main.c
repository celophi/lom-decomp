#include "common.h"

INCLUDE_ASM("asm/nonmatchings/main", func_80010000);

INCLUDE_ASM("asm/nonmatchings/main", func_80010D58);

INCLUDE_ASM("asm/nonmatchings/main", func_80011358);

INCLUDE_ASM("asm/nonmatchings/main", func_80011638);

INCLUDE_ASM("asm/nonmatchings/main", func_80011710);

INCLUDE_ASM("asm/nonmatchings/main", func_800118DC);

INCLUDE_ASM("asm/nonmatchings/main", func_800119C0);

INCLUDE_ASM("asm/nonmatchings/main", func_800120A0);

INCLUDE_ASM("asm/nonmatchings/main", func_800122C0);

INCLUDE_ASM("asm/nonmatchings/main", func_80013744);

INCLUDE_ASM("asm/nonmatchings/main", func_80013A00);

INCLUDE_ASM("asm/nonmatchings/main", func_80013F2C);

INCLUDE_ASM("asm/nonmatchings/main", func_80013F64);

/*
 * Set audio volume for a specific stereo channel
 *
 * Params:
 *   volume - Volume level to set (0-255)
 *   stereoChannel - 0 for left channel, non-zero for right channel
 *
 * Returns: void
 * 
 * decomp.me link: https://decomp.me/scratch/XbegS
 * decomp.me (%): 99.12%
 */
void CD_SetAudioVolume(u_char volume, int stereoChannel)
{
    CdlATV audioConfig[2];
    
    if (stereoChannel != 0) {
        audioConfig[0].val0 = volume;
        audioConfig[0].val1 = 0;
        audioConfig[0].val2 = volume;
    } else {
        audioConfig[0].val0 = volume;
        audioConfig[0].val1 = volume;
        audioConfig[0].val2 = 0;
        audioConfig[0].val3 = 0;
    }
    
    CdMix(audioConfig);
}

INCLUDE_ASM("asm/nonmatchings/main", func_80014014);

INCLUDE_ASM("asm/nonmatchings/main", func_80014140);

INCLUDE_ASM("asm/nonmatchings/main", func_800141EC);

INCLUDE_ASM("asm/nonmatchings/main", func_80014448);

INCLUDE_ASM("asm/nonmatchings/main", func_80014AD0);

INCLUDE_ASM("asm/nonmatchings/main", func_80014ADC);

INCLUDE_ASM("asm/nonmatchings/main", func_80014C54);

INCLUDE_ASM("asm/nonmatchings/main", func_80015674);

INCLUDE_ASM("asm/nonmatchings/main", func_80015708);

INCLUDE_ASM("asm/nonmatchings/main", func_80015770);

INCLUDE_ASM("asm/nonmatchings/main", func_800157B0);

INCLUDE_ASM("asm/nonmatchings/main", func_800157DC);

INCLUDE_ASM("asm/nonmatchings/main", func_800158E0);

INCLUDE_ASM("asm/nonmatchings/main", func_800159F8);

INCLUDE_ASM("asm/nonmatchings/main", func_80015AA4);

INCLUDE_ASM("asm/nonmatchings/main", func_80015B58);

INCLUDE_ASM("asm/nonmatchings/main", func_80015C18);

INCLUDE_ASM("asm/nonmatchings/main", func_80015C28);

INCLUDE_ASM("asm/nonmatchings/main", func_80015C38);

INCLUDE_ASM("asm/nonmatchings/main", func_80015C48);

INCLUDE_ASM("asm/nonmatchings/main", func_80015C58);

INCLUDE_ASM("asm/nonmatchings/main", func_80015D6C);

INCLUDE_ASM("asm/nonmatchings/main", func_80015F88);

INCLUDE_ASM("asm/nonmatchings/main", func_800165CC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001676C);

INCLUDE_ASM("asm/nonmatchings/main", func_80016784);

INCLUDE_ASM("asm/nonmatchings/main", func_8001679C);

INCLUDE_ASM("asm/nonmatchings/main", func_800167AC);

INCLUDE_ASM("asm/nonmatchings/main", func_800167BC);

INCLUDE_ASM("asm/nonmatchings/main", func_800167DC);

INCLUDE_ASM("asm/nonmatchings/main", func_800167EC);

INCLUDE_ASM("asm/nonmatchings/main", func_800167FC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001682C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001688C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001689C);

INCLUDE_ASM("asm/nonmatchings/main", func_80016ABC);

INCLUDE_ASM("asm/nonmatchings/main", func_80016AD8);

INCLUDE_ASM("asm/nonmatchings/main", func_80016AF4);

INCLUDE_ASM("asm/nonmatchings/main", func_80016B10);

INCLUDE_ASM("asm/nonmatchings/main", func_80016B2C);

INCLUDE_ASM("asm/nonmatchings/main", func_80016B48);

INCLUDE_ASM("asm/nonmatchings/main", func_80016B7C);

INCLUDE_ASM("asm/nonmatchings/main", func_80016B98);

INCLUDE_ASM("asm/nonmatchings/main", func_80016E6C);

INCLUDE_ASM("asm/nonmatchings/main", func_80016E7C);

INCLUDE_ASM("asm/nonmatchings/main", func_80016EBC);

INCLUDE_ASM("asm/nonmatchings/main", func_80016EEC);

INCLUDE_ASM("asm/nonmatchings/main", func_80016F8C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001704C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001710C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001714C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001725C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001726C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001727C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001728C);

INCLUDE_ASM("asm/nonmatchings/main", func_800172EC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001737C);

INCLUDE_ASM("asm/nonmatchings/main", func_800173F8);

INCLUDE_ASM("asm/nonmatchings/main", func_80017590);

INCLUDE_ASM("asm/nonmatchings/main", func_80017608);

INCLUDE_ASM("asm/nonmatchings/main", func_800176EC);

INCLUDE_ASM("asm/nonmatchings/main", func_800176FC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001770C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001771C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001772C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001773C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001774C);

INCLUDE_ASM("asm/nonmatchings/main", func_80017760);

INCLUDE_ASM("asm/nonmatchings/main", func_80017774);

INCLUDE_ASM("asm/nonmatchings/main", func_800177EC);

INCLUDE_ASM("asm/nonmatchings/main", func_800177FC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001786C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001787C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001788C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001789C);

INCLUDE_ASM("asm/nonmatchings/main", func_80017978);

INCLUDE_ASM("asm/nonmatchings/main", func_80017A0C);

INCLUDE_ASM("asm/nonmatchings/main", func_80017A7C);

INCLUDE_ASM("asm/nonmatchings/main", func_80017ABC);

INCLUDE_ASM("asm/nonmatchings/main", func_80017EFC);

INCLUDE_ASM("asm/nonmatchings/main", func_800181A8);

INCLUDE_ASM("asm/nonmatchings/main", func_800181CC);

INCLUDE_ASM("asm/nonmatchings/main", func_800182BC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001834C);

INCLUDE_ASM("asm/nonmatchings/main", func_800183D8);

INCLUDE_ASM("asm/nonmatchings/main", func_8001846C);

INCLUDE_ASM("asm/nonmatchings/main", func_80018500);

INCLUDE_ASM("asm/nonmatchings/main", func_80018518);

INCLUDE_ASM("asm/nonmatchings/main", func_8001859C);

INCLUDE_ASM("asm/nonmatchings/main", func_800185DC);

INCLUDE_ASM("asm/nonmatchings/main", func_80018CEC);

INCLUDE_ASM("asm/nonmatchings/main", func_80018D3C);

INCLUDE_ASM("asm/nonmatchings/main", func_80018E38);

INCLUDE_ASM("asm/nonmatchings/main", func_8001940C);

INCLUDE_ASM("asm/nonmatchings/main", func_80019580);

INCLUDE_ASM("asm/nonmatchings/main", func_80019690);

INCLUDE_ASM("asm/nonmatchings/main", func_800196F0);

INCLUDE_ASM("asm/nonmatchings/main", func_80019788);

INCLUDE_ASM("asm/nonmatchings/main", func_800197F0);

INCLUDE_ASM("asm/nonmatchings/main", func_8001990C);

INCLUDE_ASM("asm/nonmatchings/main", func_80019AF4);

INCLUDE_ASM("asm/nonmatchings/main", func_80019C74);

INCLUDE_ASM("asm/nonmatchings/main", func_80019D7C);

INCLUDE_ASM("asm/nonmatchings/main", func_80019DEC);

INCLUDE_ASM("asm/nonmatchings/main", func_80019FB8);

INCLUDE_ASM("asm/nonmatchings/main", func_8001A7CC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001AA3C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001AA5C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001AAF4);

INCLUDE_ASM("asm/nonmatchings/main", func_8001AB8C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001ABA8);

INCLUDE_ASM("asm/nonmatchings/main", func_8001B484);

INCLUDE_ASM("asm/nonmatchings/main", func_8001B4CC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001B520);

INCLUDE_ASM("asm/nonmatchings/main", func_8001B7D0);

INCLUDE_ASM("asm/nonmatchings/main", func_8001BA30);

INCLUDE_ASM("asm/nonmatchings/main", func_8001BCBC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001BCF0);

INCLUDE_ASM("asm/nonmatchings/main", func_8001BE34);

INCLUDE_ASM("asm/nonmatchings/main", func_8001C2EC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001C314);

INCLUDE_ASM("asm/nonmatchings/main", func_8001C33C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001C56C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001C62C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001C6E8);

INCLUDE_ASM("asm/nonmatchings/main", func_8001C8EC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001CBF4);

INCLUDE_ASM("asm/nonmatchings/main", func_8001CE0C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001D58C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001D5AC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E0EC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E1CC);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E25C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E31C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E3D4);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E3F4);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E470);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E490);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E4B0);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E4C4);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E4D8);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E614);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E748);

INCLUDE_ASM("asm/nonmatchings/main", CdMix);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E8B4);

INCLUDE_ASM("asm/nonmatchings/main", func_8001E93C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001EEA0);

INCLUDE_ASM("asm/nonmatchings/main", func_8001F120);

INCLUDE_ASM("asm/nonmatchings/main", func_8001F3E8);

INCLUDE_ASM("asm/nonmatchings/main", func_8001F7F4);

INCLUDE_ASM("asm/nonmatchings/main", func_8001F87C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001F950);

INCLUDE_ASM("asm/nonmatchings/main", func_8001FA40);

INCLUDE_ASM("asm/nonmatchings/main", func_8001FA8C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001FC6C);

INCLUDE_ASM("asm/nonmatchings/main", func_8001FDD4);

INCLUDE_ASM("asm/nonmatchings/main", func_8001FED4);

INCLUDE_ASM("asm/nonmatchings/main", func_800200AC);

INCLUDE_ASM("asm/nonmatchings/main", func_8002010C);

INCLUDE_ASM("asm/nonmatchings/main", func_80020248);

INCLUDE_ASM("asm/nonmatchings/main", func_800203BC);

INCLUDE_ASM("asm/nonmatchings/main", func_800204C0);

INCLUDE_ASM("asm/nonmatchings/main", func_8002054C);

INCLUDE_ASM("asm/nonmatchings/main", func_800206C4);

INCLUDE_ASM("asm/nonmatchings/main", func_8002075C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002076C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002079C);

INCLUDE_ASM("asm/nonmatchings/main", func_800207CC);

INCLUDE_ASM("asm/nonmatchings/main", func_800207FC);

INCLUDE_ASM("asm/nonmatchings/main", func_800208C0);

INCLUDE_ASM("asm/nonmatchings/main", func_800208E8);

INCLUDE_ASM("asm/nonmatchings/main", func_800209D8);

INCLUDE_ASM("asm/nonmatchings/main", func_80020E08);

INCLUDE_ASM("asm/nonmatchings/main", func_80020E2C);

INCLUDE_ASM("asm/nonmatchings/main", func_80020E3C);

INCLUDE_ASM("asm/nonmatchings/main", func_80020E4C);

INCLUDE_ASM("asm/nonmatchings/main", func_80020E5C);

INCLUDE_ASM("asm/nonmatchings/main", func_80020EDC);

INCLUDE_ASM("asm/nonmatchings/main", func_80020FCC);

INCLUDE_ASM("asm/nonmatchings/main", func_80020FFC);

INCLUDE_ASM("asm/nonmatchings/main", func_80021274);

INCLUDE_ASM("asm/nonmatchings/main", func_8002129C);

INCLUDE_ASM("asm/nonmatchings/main", func_800212B0);

INCLUDE_ASM("asm/nonmatchings/main", func_800212CC);

INCLUDE_ASM("asm/nonmatchings/main", func_800212EC);

INCLUDE_ASM("asm/nonmatchings/main", func_800213D4);

INCLUDE_ASM("asm/nonmatchings/main", func_8002144C);

INCLUDE_ASM("asm/nonmatchings/main", func_800216CC);

INCLUDE_ASM("asm/nonmatchings/main", func_800219F0);

INCLUDE_ASM("asm/nonmatchings/main", func_80021C70);

INCLUDE_ASM("asm/nonmatchings/main", func_80021CF4);

INCLUDE_ASM("asm/nonmatchings/main", func_80021D58);

INCLUDE_ASM("asm/nonmatchings/main", func_80021D9C);

INCLUDE_ASM("asm/nonmatchings/main", func_80021ED4);

INCLUDE_ASM("asm/nonmatchings/main", func_80021EFC);

INCLUDE_ASM("asm/nonmatchings/main", func_80021F24);

INCLUDE_ASM("asm/nonmatchings/main", func_80021F8C);

INCLUDE_ASM("asm/nonmatchings/main", func_80021FBC);

INCLUDE_ASM("asm/nonmatchings/main", func_80022040);

INCLUDE_ASM("asm/nonmatchings/main", func_80022400);

INCLUDE_ASM("asm/nonmatchings/main", func_8002279C);

INCLUDE_ASM("asm/nonmatchings/main", func_800227D0);

INCLUDE_ASM("asm/nonmatchings/main", func_80022AA8);

INCLUDE_ASM("asm/nonmatchings/main", func_80022AC8);

INCLUDE_ASM("asm/nonmatchings/main", func_80022AE8);

INCLUDE_ASM("asm/nonmatchings/main", func_80022DAC);

INCLUDE_ASM("asm/nonmatchings/main", func_80023010);

INCLUDE_ASM("asm/nonmatchings/main", func_80023060);

INCLUDE_ASM("asm/nonmatchings/main", func_800233C8);

INCLUDE_ASM("asm/nonmatchings/main", func_80023508);

INCLUDE_ASM("asm/nonmatchings/main", func_80023548);

INCLUDE_ASM("asm/nonmatchings/main", func_800235A8);

INCLUDE_ASM("asm/nonmatchings/main", func_800235F8);

INCLUDE_ASM("asm/nonmatchings/main", func_80023630);

INCLUDE_ASM("asm/nonmatchings/main", func_80023660);

INCLUDE_ASM("asm/nonmatchings/main", func_800236EC);

INCLUDE_ASM("asm/nonmatchings/main", func_8002371C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002376C);

INCLUDE_ASM("asm/nonmatchings/main", func_80023830);

INCLUDE_ASM("asm/nonmatchings/main", func_80023AD0);

INCLUDE_ASM("asm/nonmatchings/main", func_80023BB8);

INCLUDE_ASM("asm/nonmatchings/main", func_80023BE0);

INCLUDE_ASM("asm/nonmatchings/main", func_80023C80);

INCLUDE_ASM("asm/nonmatchings/main", func_80023C90);

INCLUDE_ASM("asm/nonmatchings/main", func_80023CA0);

INCLUDE_ASM("asm/nonmatchings/main", func_80023D3C);

INCLUDE_ASM("asm/nonmatchings/main", func_80023D74);

INCLUDE_ASM("asm/nonmatchings/main", func_80023DA4);

INCLUDE_ASM("asm/nonmatchings/main", func_80023E10);

INCLUDE_ASM("asm/nonmatchings/main", func_80023E90);

INCLUDE_ASM("asm/nonmatchings/main", func_80023EF0);

INCLUDE_ASM("asm/nonmatchings/main", func_80024040);

INCLUDE_ASM("asm/nonmatchings/main", func_800240D0);

INCLUDE_ASM("asm/nonmatchings/main", func_80024110);

INCLUDE_ASM("asm/nonmatchings/main", func_80024140);

INCLUDE_ASM("asm/nonmatchings/main", func_800241A0);

INCLUDE_ASM("asm/nonmatchings/main", func_80024200);

INCLUDE_ASM("asm/nonmatchings/main", func_80024230);

INCLUDE_ASM("asm/nonmatchings/main", func_80024260);

INCLUDE_ASM("asm/nonmatchings/main", func_8002427C);

INCLUDE_ASM("asm/nonmatchings/main", func_80024298);

INCLUDE_ASM("asm/nonmatchings/main", func_800242B4);

INCLUDE_ASM("asm/nonmatchings/main", func_800242D0);

INCLUDE_ASM("asm/nonmatchings/main", func_800242EC);

INCLUDE_ASM("asm/nonmatchings/main", func_80024334);

INCLUDE_ASM("asm/nonmatchings/main", func_8002434C);

INCLUDE_ASM("asm/nonmatchings/main", func_80024368);

INCLUDE_ASM("asm/nonmatchings/main", func_80024384);

INCLUDE_ASM("asm/nonmatchings/main", func_8002439C);

INCLUDE_ASM("asm/nonmatchings/main", func_800243B4);

INCLUDE_ASM("asm/nonmatchings/main", func_800243E4);

INCLUDE_ASM("asm/nonmatchings/main", func_8002440C);

INCLUDE_ASM("asm/nonmatchings/main", func_80024434);

INCLUDE_ASM("asm/nonmatchings/main", func_80024468);

INCLUDE_ASM("asm/nonmatchings/main", func_80024498);

INCLUDE_ASM("asm/nonmatchings/main", func_80024544);

INCLUDE_ASM("asm/nonmatchings/main", func_80024660);

INCLUDE_ASM("asm/nonmatchings/main", func_80024B00);

INCLUDE_ASM("asm/nonmatchings/main", func_80024F60);

INCLUDE_ASM("asm/nonmatchings/main", func_800253E8);

INCLUDE_ASM("asm/nonmatchings/main", func_80025498);

INCLUDE_ASM("asm/nonmatchings/main", func_80025500);

INCLUDE_ASM("asm/nonmatchings/main", func_80025760);

INCLUDE_ASM("asm/nonmatchings/main", func_800257E0);

INCLUDE_ASM("asm/nonmatchings/main", func_800258B8);

INCLUDE_ASM("asm/nonmatchings/main", func_80025D38);

INCLUDE_ASM("asm/nonmatchings/main", func_80025D98);

INCLUDE_ASM("asm/nonmatchings/main", func_80025F48);

INCLUDE_ASM("asm/nonmatchings/main", func_800260CC);

INCLUDE_ASM("asm/nonmatchings/main", func_8002611C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002613C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002616C);

INCLUDE_ASM("asm/nonmatchings/main", func_80026208);

INCLUDE_ASM("asm/nonmatchings/main", func_80026254);

INCLUDE_ASM("asm/nonmatchings/main", func_800265DC);

INCLUDE_ASM("asm/nonmatchings/main", func_800266B0);

INCLUDE_ASM("asm/nonmatchings/main", func_80026A10);

INCLUDE_ASM("asm/nonmatchings/main", func_80026BB8);

INCLUDE_ASM("asm/nonmatchings/main", func_80026C14);

INCLUDE_ASM("asm/nonmatchings/main", func_80026E0C);

INCLUDE_ASM("asm/nonmatchings/main", func_80026E8C);

INCLUDE_ASM("asm/nonmatchings/main", func_80026ED4);

INCLUDE_ASM("asm/nonmatchings/main", func_80026F28);

INCLUDE_ASM("asm/nonmatchings/main", func_80027190);

INCLUDE_ASM("asm/nonmatchings/main", func_800271EC);

INCLUDE_ASM("asm/nonmatchings/main", func_80028E34);

INCLUDE_ASM("asm/nonmatchings/main", func_80028E84);

INCLUDE_ASM("asm/nonmatchings/main", func_8002918C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002935C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002982C);

INCLUDE_ASM("asm/nonmatchings/main", func_800299CC);

INCLUDE_ASM("asm/nonmatchings/main", func_800299DC);

INCLUDE_ASM("asm/nonmatchings/main", func_800299EC);

INCLUDE_ASM("asm/nonmatchings/main", func_80029A0C);

INCLUDE_ASM("asm/nonmatchings/main", func_80029A8C);

INCLUDE_ASM("asm/nonmatchings/main", func_80029E88);

INCLUDE_ASM("asm/nonmatchings/main", func_8002A4E8);

INCLUDE_ASM("asm/nonmatchings/main", func_8002A6FC);

INCLUDE_ASM("asm/nonmatchings/main", func_8002A924);

INCLUDE_ASM("asm/nonmatchings/main", func_8002AAB4);

INCLUDE_ASM("asm/nonmatchings/main", func_8002AD28);

INCLUDE_ASM("asm/nonmatchings/main", func_8002B468);

INCLUDE_ASM("asm/nonmatchings/main", func_8002B49C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002B4D4);

INCLUDE_ASM("asm/nonmatchings/main", func_8002B540);

INCLUDE_ASM("asm/nonmatchings/main", func_8002B580);

INCLUDE_ASM("asm/nonmatchings/main", func_8002C418);

INCLUDE_ASM("asm/nonmatchings/main", func_8002C478);

INCLUDE_ASM("asm/nonmatchings/main", func_8002C4E0);

INCLUDE_ASM("asm/nonmatchings/main", func_8002C554);

INCLUDE_ASM("asm/nonmatchings/main", func_8002C5BC);

INCLUDE_ASM("asm/nonmatchings/main", func_8002C61C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002C79C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002C7D0);

INCLUDE_ASM("asm/nonmatchings/main", func_8002D140);

INCLUDE_ASM("asm/nonmatchings/main", func_8002D1C4);

INCLUDE_ASM("asm/nonmatchings/main", func_8002D29C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002D4D8);

INCLUDE_ASM("asm/nonmatchings/main", func_8002D694);

INCLUDE_ASM("asm/nonmatchings/main", func_8002D82C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002DDDC);

INCLUDE_ASM("asm/nonmatchings/main", func_8002DFA4);

INCLUDE_ASM("asm/nonmatchings/main", func_8002E294);

INCLUDE_ASM("asm/nonmatchings/main", func_8002E2E8);

INCLUDE_ASM("asm/nonmatchings/main", func_8002E478);

INCLUDE_ASM("asm/nonmatchings/main", func_8002E5A4);

INCLUDE_ASM("asm/nonmatchings/main", func_8002E850);

INCLUDE_ASM("asm/nonmatchings/main", func_8002E928);

INCLUDE_ASM("asm/nonmatchings/main", func_8002E938);

INCLUDE_ASM("asm/nonmatchings/main", func_8002E958);

INCLUDE_ASM("asm/nonmatchings/main", func_8002E978);

INCLUDE_ASM("asm/nonmatchings/main", func_8002E9E4);

INCLUDE_ASM("asm/nonmatchings/main", func_8002EAB0);

INCLUDE_ASM("asm/nonmatchings/main", func_8002EBA8);

INCLUDE_ASM("asm/nonmatchings/main", func_8002ED24);

INCLUDE_ASM("asm/nonmatchings/main", func_8002ED5C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002EDA4);

INCLUDE_ASM("asm/nonmatchings/main", func_8002EF10);

INCLUDE_ASM("asm/nonmatchings/main", func_8002F134);

INCLUDE_ASM("asm/nonmatchings/main", func_8002F148);

INCLUDE_ASM("asm/nonmatchings/main", func_8002F214);

INCLUDE_ASM("asm/nonmatchings/main", func_8002F258);

INCLUDE_ASM("asm/nonmatchings/main", func_8002F58C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002F67C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002F88C);

INCLUDE_ASM("asm/nonmatchings/main", func_8002FB04);

INCLUDE_ASM("asm/nonmatchings/main", func_8002FB94);

INCLUDE_ASM("asm/nonmatchings/main", func_8002FBC8);

INCLUDE_ASM("asm/nonmatchings/main", func_8002FBE4);

INCLUDE_ASM("asm/nonmatchings/main", func_8002FC68);

INCLUDE_ASM("asm/nonmatchings/main", func_8002FED8);

INCLUDE_ASM("asm/nonmatchings/main", func_8002FF10);

INCLUDE_ASM("asm/nonmatchings/main", func_80030484);

INCLUDE_ASM("asm/nonmatchings/main", func_800305D0);

INCLUDE_ASM("asm/nonmatchings/main", func_8003071C);

INCLUDE_ASM("asm/nonmatchings/main", func_8003073C);

INCLUDE_ASM("asm/nonmatchings/main", func_80030750);

INCLUDE_ASM("asm/nonmatchings/main", func_80030770);

INCLUDE_ASM("asm/nonmatchings/main", func_80030790);

INCLUDE_ASM("asm/nonmatchings/main", func_800307B0);

INCLUDE_ASM("asm/nonmatchings/main", func_80030DF8);

INCLUDE_ASM("asm/nonmatchings/main", func_800310B4);

INCLUDE_ASM("asm/nonmatchings/main", func_80031458);

INCLUDE_ASM("asm/nonmatchings/main", func_80031800);

INCLUDE_ASM("asm/nonmatchings/main", func_80031918);

INCLUDE_ASM("asm/nonmatchings/main", func_80031938);

INCLUDE_ASM("asm/nonmatchings/main", func_800319D8);

INCLUDE_ASM("asm/nonmatchings/main", func_80031A88);

INCLUDE_ASM("asm/nonmatchings/main", func_80031D04);

INCLUDE_ASM("asm/nonmatchings/main", func_80031E48);

INCLUDE_ASM("asm/nonmatchings/main", func_80031F1C);

INCLUDE_ASM("asm/nonmatchings/main", func_80031F3C);

INCLUDE_ASM("asm/nonmatchings/main", func_80031FDC);

INCLUDE_ASM("asm/nonmatchings/main", func_800322D8);

INCLUDE_ASM("asm/nonmatchings/main", func_80032328);

INCLUDE_ASM("asm/nonmatchings/main", func_800342F8);

INCLUDE_ASM("asm/nonmatchings/main", func_80034D78);

INCLUDE_ASM("asm/nonmatchings/main", func_80034DD8);

INCLUDE_ASM("asm/nonmatchings/main", func_80034ECC);

INCLUDE_ASM("asm/nonmatchings/main", func_80034F58);

INCLUDE_ASM("asm/nonmatchings/main", func_80034F98);
