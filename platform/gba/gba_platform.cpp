#include "platform/platform.hpp"
#include "gba.h"
#include "critical_section.hpp"
#include "bootleg_cart.hpp"


#define COLD [[gnu::cold]]
#define HOT [[gnu::hot]]

#ifndef __SKYLAND_SOURCE__
#define info(PLATFORM, STRING)
#endif


IrqState critical_section_enter()
{
    int temp = 0;

    // Stop dma transfers
    DMA_TRANSFER((volatile short*)0x4000014, &temp, 1, 0, 0);
    DMA_TRANSFER((volatile short*)0x4000016, &temp, 1, 3, 0);

    REG_SOUNDCNT_H = REG_SOUNDCNT_H & ~(1 << 8);
    REG_SOUNDCNT_H = REG_SOUNDCNT_H & ~(1 << 9);

    const u16 ime = REG_IME;
    REG_IME = 0;
    const u16 ie = REG_IE;
    REG_IE = 0;

    return {ime, ie};
}



void critical_section_exit(IrqState state)
{
    REG_IE = state.second;
    REG_IME = state.first;

    REG_SOUNDCNT_H = REG_SOUNDCNT_H | (1 << 9);
    REG_SOUNDCNT_H = REG_SOUNDCNT_H | (1 << 8);
}



static u8* const cartridge_ram = (u8*)0x0E000000;


static bool
flash_byteverify(void* in_dst, const void* in_src, unsigned int length)
{
    unsigned char* src = (unsigned char*)in_src;
    unsigned char* dst = (unsigned char*)in_dst;

    for (; length > 0; length--) {

        if (*dst++ != *src++)
            return false;
    }
    return true;
}


static void
flash_bytecpy(void* in_dst, const void* in_src, unsigned int length, bool write)
{
    unsigned char* src = (unsigned char*)in_src;
    unsigned char* dst = (unsigned char*)in_dst;

    for (; length > 0; length--) {
        if (write) {
            *(volatile u8*)0x0E005555 = 0xAA;
            *(volatile u8*)0x0E002AAA = 0x55;
            *(volatile u8*)0x0E005555 = 0xA0;
        }
        *dst++ = *src++;
    }
}


int save_capacity = 32000;


int Platform::save_capacity()
{
    return ::save_capacity;
}


static void set_flash_bank(u32 bankID)
{
    if (bankID < 2) {
        *(volatile u8*)0x0E005555 = 0xAA;
        *(volatile u8*)0x0E002AAA = 0x55;
        *(volatile u8*)0x0E005555 = 0xB0;
        *(volatile u8*)0x0E000000 = bankID;
    }
}



COLD static bool flash_save(const void* data, u32 flash_offset, u32 length)
{
    if ((u32)flash_offset >= 0x10000) {
        set_flash_bank(1);
    } else {
        set_flash_bank(0);
    }

    flash_bytecpy((void*)(cartridge_ram + flash_offset), data, length, true);

    return flash_byteverify(
        (void*)(cartridge_ram + flash_offset), data, length);
}



#define MEM_FLASH 0x0E000000
#define flash_mem ((volatile u8*)MEM_FLASH)

#define FLASH_CMD_BEGIN                                                        \
    flash_mem[0x5555] = 0xAA;                                                  \
    flash_mem[0x2AAA] = 0x55;
#define FLASH_CMD(cmd)                                                         \
    FLASH_CMD_BEGIN;                                                           \
    flash_mem[0x5555] = (cmd) << 4;



enum FlashCmd {
    FLASH_CMD_ERASE_CHIP = 1,
    FLASH_CMD_ERASE_SECTOR = 3,
    FLASH_CMD_ERASE = 8,
    FLASH_CMD_ENTER_ID_MODE = 9,
    FLASH_CMD_WRITE = 0xA,
    FLASH_CMD_SWITCH_BANK = 0xB,
    FLASH_CMD_LEAVE_ID_MODE = 0xF,
};



enum FlashManufacturer {
    FLASH_MFR_ATMEL = 0x1F,
    FLASH_MFR_PANASONIC = 0x32,
    FLASH_MFR_SANYO = 0x62,
    FLASH_MFR_SST = 0xBF,
    FLASH_MFR_MACRONIX = 0xC2,
};



enum FlashDevice {
    FLASH_DEV_MX29L010 = 0x09,
    FLASH_DEV_LE26FV10N1TS = 0x13,
    FLASH_DEV_MN63F805MNP = 0x1B,
    FLASH_DEV_MX29L512 = 0x1C,
    FLASH_DEV_AT29LV512 = 0x3D,
    FLASH_DEV_LE39FW512 = 0xD4,
};



static void __attribute__((noinline)) busy_wait(unsigned max)
{
    for (unsigned i = 0; i < max; i++) {
        __asm__ volatile("" : "+g"(i) : :);
    }
}



static u32 flash_capacity(Platform& pfrm)
{
    REG_WAITCNT |= WS_SRAM_8;

    FLASH_CMD(FLASH_CMD_ENTER_ID_MODE);

    busy_wait(20000);

    auto device = *(&flash_mem[1]);
    auto manufacturer = *(&flash_mem[0]);

    FLASH_CMD(FLASH_CMD_LEAVE_ID_MODE);

    busy_wait(20000);

    if (manufacturer == FLASH_MFR_SANYO) {
        flash_mem[0x5555] = FLASH_CMD_LEAVE_ID_MODE << 4;
    }

    if ((manufacturer == FLASH_MFR_MACRONIX and device == FLASH_DEV_MX29L010) or
        (manufacturer == FLASH_MFR_SANYO and
         device == FLASH_DEV_LE26FV10N1TS)) {

        info(pfrm,
             "detected 128kb flash chip. Bank switching unimplemented, using "
             "64kb.");
    }

    info(pfrm, "detected 64kb flash chip");
    return 64000;
}



static void flash_load(void* dest, u32 flash_offset, u32 length)
{
    if (flash_offset >= 0x10000) {
        set_flash_bank(1);
    } else {
        set_flash_bank(0);
    }

    flash_bytecpy(
        dest, (const void*)(cartridge_ram + flash_offset), length, false);
}



IWRAM_CODE
void sram_save(const void* data, u32 offset, u32 length)
{
    u16 ime = REG_IME;
    REG_IME = 0;

    u8* save_mem = (u8*)0x0E000000 + offset;

    // The cartridge has an 8-bit bus, so you have to write one byte at a time,
    // otherwise it won't work!
    for (u32 i = 0; i < length; ++i) {
        *save_mem++ = ((const u8*)data)[i];
    }

    REG_IME = ime;
}



void sram_load(void* dest, u32 offset, u32 length)
{
    u8* save_mem = (u8*)cartridge_ram + offset;
    for (u32 i = 0; i < length; ++i) {
        ((u8*)dest)[i] = *save_mem++;
    }
}



static BootlegFlashType bootleg_flash_type = 0;



bool Platform::write_save_data(const void* data, u32 length, u32 offset)
{
    if (get_gflag(GlobalFlag::save_using_flash)) {
        return flash_save(data, offset, length);
    } else {
        sram_save(data, offset, length);

        if (bootleg_flash_type) {
            bool success =
                bootleg_flash_writeback(bootleg_flash_type, offset, length);
            if (not success) {
                info(*this, "flash write verification failed!");
                return false;
            }
        }

        return true;
    }
}


bool Platform::read_save_data(void* buffer, u32 data_length, u32 offset)
{
    if (get_gflag(GlobalFlag::save_using_flash)) {
        flash_load(buffer, offset, data_length);
    } else {
        sram_load(buffer, offset, data_length);
    }

    return true;
}



void Platform::erase_save_sector()
{
    if (not get_gflag(GlobalFlag::save_using_flash)) {
        u8* save_mem = (u8*)0x0E000000;
        // Simulate a flash erase.
        for (int i = 0; i < ::save_capacity; ++i) {
            save_mem[i] = 0xff;
        }
    } else {
        FLASH_CMD(FLASH_CMD_ERASE);
        FLASH_CMD(FLASH_CMD_ERASE_CHIP);

        info(*this, "begin flash erase!");

        // Wait for erase to complete.
        while (*((volatile u8*)0x0E000000) not_eq 0xff)
            ;
    }

    if (bootleg_flash_type) {
        info(*this, "flash erase!");
        bootleg_flash_erase(bootleg_flash_type);
    }
}
