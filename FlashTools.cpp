/* **************************************************************************************************************************************************************
 * FlashTools.cpp                                                                                                                                               *
 * Created by Dave Dorzback                                                                                                                                     *
 * Copyright (C) Dave Dorzback                                                                                                                                  *
 *                                                                                                                                                              *
 * FlashTools is for use with Atmel's ATSAM3X8E MCU and the Arduino Due. It provides a number of functions for various internal flash operations.               *
 * These include retrieving the MCU's unique identifier, getting/setting the GPNVN bits, internal flash writes/erases, and reading/copying data from flash.     *
 *                                                                                                                                                              *
 * Moreover, it provides an interface with the Cortex-M3's integrated Memory Protection Unit (MPU) and functionality for memory protection via the MPU.         *
 * Memory attributes and access permissions can be set using this API, and a full list of encodings for the MPU's registers can be found in the datasheet.      *                                                                                                                                                            
 * Additional functionality for memory management may be added in the future.                                                                                   *
 *                                                                                                                                                              *
 * **************************************************************************************************************************************************************/

#include "FlashTools.h"

/*** Function pointer for IAP routine ***/
FlashTools::IAP_FPTR FlashTools::IAP = NULL;

/*
 * Constructor: Initialize IAP function and EFC controllers.
 * Save flash access mode and flash wait state values.
 */
__attribute__ ((noinline, section(".ramfunc"))) FlashTools::FlashTools(void) {
    /* Set EFC, MPU, and SCB instances */
    efc = EFC0;
    mpu = ((MpuInstance*)MPU_ADDR);
    scb = ((ScbInstance*)SCB_ADDR);
    
    /* Enable mem fault exceptions */
    scb->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
    
    /* Retrieve IAP function entry by reading NMI vector in ROM (address 0x00100008) */
    IAP = (uint32_t(*)(uint32_t EFCidx, uint32_t cmd)) *((uint32_t *)IAP_ENTRY_ADDRESS);
    
    /* Save flash wait state/access mode values */
    FWS0 = ((EFC0->EEFC_FMR & EEFC_FMR_FWS_Msk) >> EEFC_FMR_FWS_Pos);
    FWS1 = ((EFC1->EEFC_FMR & EEFC_FMR_FWS_Msk) >> EEFC_FMR_FWS_Pos);
    FAM0 = (EFC0->EEFC_FMR & EEFC_FMR_FAM);
    FAM1 = (EFC1->EEFC_FMR & EEFC_FMR_FAM);
    
    /* Initialize EFC controllers; set flash access mode and wait state values in Flash Mode Register */
    EFC0->EEFC_FMR = FLASH_ACCESS_MODE_128 | EEFC_FMR_FWS(CHIP_FLASH_WAIT_STATE);
    EFC1->EEFC_FMR = FLASH_ACCESS_MODE_128 | EEFC_FMR_FWS(CHIP_FLASH_WAIT_STATE);
    
    /* Initialize unique Id array member */
    for (size_t i {0}; i < UNIQUE_ID_SIZE; ++i) {
        uniqueID[i] = 0;
    }
    
    /* Initialize flash descriptor array member */
    for (size_t i {0}; i < FLASH_DESCRIPTOR_SIZE; ++i) {
        flash_descriptor[i] = 0;
    }
    flash_descriptor[FLASH_DESCRIPTOR_SIZE] = 0xFFFFFFFF;
}

/*
 * Destructor: Restore flash access mode and flash wait state values.
 */
__attribute__ ((noinline, section(".ramfunc"))) FlashTools::~FlashTools(void) {
    EFC0->EEFC_FMR = FAM0 | EEFC_FMR_FWS(FWS0);
    EFC1->EEFC_FMR = FAM1 | EEFC_FMR_FWS(FWS1);
}

/*
 * setfws: Set flash wait state in the current EFC instance's FLash Mode Register
 *  fws - Flash wait state value (number of wait states in cycle).
 */
__attribute__ ((noinline, section(".ramfunc"))) void FlashTools::setfws(uint32_t fws) {
    efc->EEFC_FMR = ((efc->EEFC_FMR & (~EEFC_FMR_FWS_Msk)) | EEFC_FMR_FWS(fws));
}

/*
 * setfam: Set flash access mode in the current EFC instance's Flash Mode Register
 *  fa_mode - Flash access mode value: FLASH_ACCESS_MODE_128 or FLASH_ACCESS_MODE_64
 */
__attribute__ ((noinline, section(".ramfunc"))) void FlashTools::setfam(uint32_t fa_mode) {
    efc->EEFC_FMR = (efc->EEFC_FMR & (~EEFC_FMR_FAM)) | fa_mode;
}

/*
 * getfws: Read wait state value from the current EFC instance's Flash Mode Register
 * Returns wait state value.
 */
uint32_t FlashTools::getfws(void) {
    return ((efc->EEFC_FMR & EEFC_FMR_FWS_Msk) >> EEFC_FMR_FWS_Pos);
}

/*
 * getfam: Read flash access mode from the current EFC instance's Flash Mode Register
 * Returns flash access mode value.
 */
uint32_t FlashTools::getfam(void) {
    return (efc->EEFC_FMR & EEFC_FMR_FAM);
}

/*
 * cmd: Write command to EEFC using IAP routine located in ROM. Commands must be written with write protection key (0x5A).
 *  cmd - Command (FCMD)
 *  arg - Flash command argument (FARG)
 * Returns 0 if successful or error flags in Flash Status Register
 */
uint32_t FlashTools::cmd(uint32_t cmd, uint32_t arg) {
    
    /* EFC Flash Command Register definition */
    EEFC_FCR_Type EFC_FCR_REGISTER;
    
    EFC_FCR_REGISTER.FULL = 0;               // Init. all bits to 0
    EFC_FCR_REGISTER.SECTION.FCMD = cmd;     // Set bits 0-7 with flash command
    EFC_FCR_REGISTER.SECTION.FKEY = FWP_KEY; // Set bits 8-23 with flash argument
    EFC_FCR_REGISTER.SECTION.FARG = arg;     // Set bits 23-31 with flash write protection key
    
    /* Send the corresponding EFC index and command */
    IAP((efc == EFC0 ? 0 : 1), EFC_FCR_REGISTER.FULL);
    
    /* Return Flash Status Register value -- 0 if successful or error flags */
    return (efc->EEFC_FSR & EEFC_ERROR_FLAGS);
}

/*
 * flashcpy: Copies from
 *  page_address - Address of page to be written
 *  write_data   - Data buffer containing new data to be written to page
 *  offset       - Amount data is offset from the beginning of page
 *  write_size   - Size of data in write_data
 *  padding_size - Size of padding (remaining space on page after copying offset and write_data)
 *  Returns pointer to flash page
 */
uint32_t *FlashTools::flashcpy(uint32_t page_address, const void *write_data,
                               uint32_t offset, uint32_t write_size, uint32_t padding_size) {

    // Static page buffer -- All data is copied to page_buffer byte
    // by byte first, then written to flash in 32-bit words
    static uint32_t page_buffer[IFLASH_WORDS_PER_PAGE] {0};

    // Data is copied from flash page and data buffer to page_buffer in 3 parts: offset, data, padding
    uint32_t sizes[3] {offset, write_size, padding_size};
    
    // Copy destination is page buffer
    uint8_t *dest = reinterpret_cast<uint8_t*>(page_buffer);
    
    // Page data located at page address
    uint8_t *page_data = reinterpret_cast<uint8_t*>(page_address);
    
    // Validate page data and copy data pointers
    if (page_data == NULL || write_data == NULL) {
        return NULL;
    }
    
    // Copy data to be written to flash to page_buffer
    // Part 1: Copy offset from flash page to page buffer
    // Part 2: Copy write data from data buffer to page buffer
    // Part 3: Copy padding from flash page to page buffer
    for (size_t i {0}; i < 3; ++i) {
        
        // When copying offset and padding, we copy from the flash page to the page buffer
        //  to avoid unneccessarily overwriting data in these parts of the page.
        // When copying the new data, we copy from data buffer
        memcpy(dest, (i==1 ? write_data : page_data), sizes[i]);
        
        // Increment dest address and page data address by size of previous copy
        dest += sizes[i];
        page_data += sizes[i];
    }
    
    // Copy data from page_buffer to flash page in 32-bit words
    uint32_t *flash {reinterpret_cast<uint32_t *>(page_address)};
    uint32_t *src {page_buffer};
    for (size_t t {0}; t < IFLASH_WORDS_PER_PAGE; ++t) {
        *flash++ = *src++;
    }
    
    // Return flash page start address
    return reinterpret_cast<uint32_t *>(page_address);
}

/*
 * setEFC: Set the EFC controller; EFC0 for flash bank 0, EFC1 for flash bank 1
 *  efc_idx - EFC number (0 or 1)
 * Returns 0 on success or invalid code on failure
 */
uint32_t FlashTools::setEFC(uint32_t efc_idx) {
    if (efc_idx != 0 && efc_idx != 1) {
        return INVALID;
    } else {
        efc = !efc_idx ? EFC0 : EFC1;
        return SUCCESS;
    }
}

/*
 * getEFC: Get current EFC number
 * Returns EFC_IDX_0 (0) for EFC0 or EDC_IDX_1 (1) for EFC1
 */
uint32_t FlashTools::getEFC(void) {
    return efc == EFC0 ? 0 : 1;
}


/*
 * getUniqueID: Get the MCU's 4-part, 128-bit unique ID
 * Returns array containing 128-bit unique ID
 */
__attribute__ ((noinline, section(".ramfunc"))) uint32_t FlashTools::getUniqueID(uint32_t *uBuff) {
    
    // Validate buffer. See if unique ID has already been read and copy to buffer if it has been
    if (uBuff == NULL) {
        return INVALID;
    } else if (uniqueID[0]) {
        for (uint32_t i {0}; i < UNIQUE_ID_SIZE; ++i) {
            uBuff[i] = uniqueID[i];
        }
        return SUCCESS;
    }
    
    /* Get wait state value, then set wait states to 6 */
    uint32_t fws {getfws()};
    setfws(CHIP_FLASH_WAIT_STATE);
    
    /*  Get address for read operation */
    uint32_t *tmpUniqueID {reinterpret_cast<uint32_t*>((efc == EFC0) ? IFLASH0_ADDR : IFLASH1_ADDR)};
    
    /* Disable code loops optimization */
    efc->EEFC_FMR |= EEFC_FMR_SCOD;
    
    /* Start read command - write directly to EEFC flash command register */
    EEFC_FCR_Type eefc_fcr_value;
    eefc_fcr_value.FULL = 0;
    eefc_fcr_value.SECTION.FCMD = EFC_FCMD_STUI;
    eefc_fcr_value.SECTION.FARG = 0;
    eefc_fcr_value.SECTION.FKEY = FWP_KEY;
    efc->EEFC_FCR = eefc_fcr_value.FULL;
    
    /* Wait for FRDY bit to fall */
    for (volatile uint32_t stat {efc->EEFC_FSR}; (stat & EEFC_FSR_FRDY) == EEFC_FSR_FRDY; stat = efc->EEFC_FSR);
    
    /* Copy data from flash */
    for (uint32_t i {0}; i < UNIQUE_ID_SIZE; ++i) {
        uBuff[i] = uniqueID[i] = tmpUniqueID[i];
    }
    
    /* Stop read command */
    eefc_fcr_value.FULL = 0;
    eefc_fcr_value.SECTION.FCMD = EFC_FCMD_SPUI;
    eefc_fcr_value.SECTION.FARG = 0;
    eefc_fcr_value.SECTION.FKEY = FWP_KEY;
    efc->EEFC_FCR = eefc_fcr_value.FULL;
    
    /* Wait for FRDY bit to rise */
    for (volatile uint32_t stat = efc->EEFC_FSR; (stat & EEFC_FSR_FRDY) != EEFC_FSR_FRDY; stat = efc->EEFC_FSR);
    
    /* Enable code loops optimization */
    efc->EEFC_FMR &= ~EEFC_FMR_SCOD;
    
    /* Restore wait state value. Return error code on read failure */
    setfws(fws);
    
    return SUCCESS;
}

/*
 * setSecurityBit: Set security bit (GPNVM bit 0). Note that enabling security bit will prohibit read/writes.
 * Security bit can be cleared by manually asserting the erase pin.
 * Returns 0 on success or error code on failure.
 */
uint32_t FlashTools::setSecurityBit(void) {
    
    /* Get security bit (GPNVM bit 0) and see if set. Return 0 if set. */
    if ((cmd(EFC_FCMD_GGPB, 0) == SUCCESS) && (efc->EEFC_FRR & (1 << 0))) {
        return SUCCESS;
    }
    
    /* Send the set bit command. Return 0 if successful, otherwise return error code. */
    return cmd(EFC_FCMD_SGPB, 0);
}

/*
 * setBootModeSAMBA: Set boot mode to SAMBA (clear GPNVM bit 1)
 * Returns 0 on success or error code on failure
 */
uint32_t FlashTools::setBootModeSAMBA(void) {
    
    if ((cmd(EFC_FCMD_GGPB, 0) == SUCCESS) && !(efc->EEFC_FRR & (1 << 1))) {
        return SUCCESS;
    }
    
    return cmd(EFC_FCMD_CGPB, 1);
}

/*
 * setBootModeFlash: Set boot mode to flash (set GPNVM bit 1).
 * Returns 0 on success or error code on failure.
 */
uint32_t FlashTools::setBootModeFlash(void) {
    
    if ((cmd(EFC_FCMD_GGPB, 0) == SUCCESS) && (efc->EEFC_FRR & (1 << 1))) {
        return SUCCESS;
    }
    
    return cmd(EFC_FCMD_SGPB, 1);
}

/*
 * setBootFlash0: Set boot point to flash 0 (clear GPNVM bit 2).
 * Returns 0 on success or error code on failure.
 */
uint32_t FlashTools::setBootFlash0(void) {
    
    if ((cmd(EFC_FCMD_GGPB, 0) == SUCCESS) && !(efc->EEFC_FRR & (1 << 2))) {
        return SUCCESS;
    }
    
    return cmd(EFC_FCMD_CGPB, 2);
}

/*
 * setBootFlash1: Set boot point to flash 1 (set GPNVM bit 2).
 * Returns 0 on success or error code on failure.
 */
uint32_t FlashTools::setBootFlash1(void) {
    
    if ((cmd(EFC_FCMD_GGPB, 0) == SUCCESS) && (efc->EEFC_FRR & (1 << 2))) {
        return SUCCESS;
    }
    
    return cmd(EFC_FCMD_SGPB, 2);
}

/*
 * setSecurityBit: Gets the security bit (GPNVM bit 0).
 * Returns 1 if the bit is set, 0 if the bit is unset, or an error code on cmd fail
 */
uint32_t FlashTools::getSecurityBit(void) {
    
    if (cmd(EFC_FCMD_GGPB, 0) != SUCCESS) {
        return ERROR;
    }
    
    return (efc->EEFC_FRR & (1 << 0)) ? BIT_IS_SET : BIT_IS_CLEARED;
}

/*
 * getBootSelectBit: Gets the security bit (GPNVM bit 1).
 * Returns 1 if the bit is set, 0 if the bit is unset, or an error code on cmd fail
 */
uint32_t FlashTools::getBootSelectBit(void) {
    
    if (cmd(EFC_FCMD_GGPB, 0) != SUCCESS) {
        return ERROR;
    }
    
    return (efc->EEFC_FRR & (1 << 1)) ? BIT_IS_SET : BIT_IS_CLEARED;
}

/*
 * getFlashSelectBit: Gets the security bit (GPNVM bit 2).
 * Returns 1 if the bit is set, 0 if the bit is unset, or an error code on cmd fail
 */
uint32_t FlashTools::getFlashSelectBit(void) {
    
    if (cmd(EFC_FCMD_GGPB, 0) != SUCCESS) {
        return ERROR;
    }
    
    return (efc->EEFC_FRR & (1 << 2)) ? BIT_IS_SET : BIT_IS_CLEARED;
}


/*
 * getFlashDescriptor - Gets the 128-bit flash descriptor for the specified address
 * Returns flash descriptor array on success or null on failure
 */
uint32_t *FlashTools::getFlashDescriptor(uint32_t addr) {
    
    /* Determine EFC based on flash address */
    if (addr > IFLASH_LAST_PAGE_ADDRESS) {
        return NULL;
    } else {
        efc = (addr >= IFLASH1_ADDR) ? EFC1 : EFC0;
    }
    
    /* Send the get flash descriptor command. Return error on cmd failure */
    if (cmd(EFC_FCMD_GETD, 0) != SUCCESS) {
        return NULL;
    }
    
    /* Read the data and save it to buf. Return SUCCESS once all results have been read */
    for (uint32_t i {0}, res; i < FLASH_DESCRIPTOR_SIZE && (res=efc->EEFC_FRR) != 0; ++i) {
        flash_descriptor[i] = res;
    }
    
    flash_descriptor[FLASH_DESCRIPTOR_SIZE] = addr;
    
    return flash_descriptor;
}

/*
 * getFlashId: Gets the flash id for the specified address
 */
uint32_t FlashTools::getFlashId(uint32_t addr) {
    return flash_descriptor[FLASH_DESCRIPTOR_SIZE] == addr || getFlashDescriptor(addr) != NULL ? flash_descriptor[0] : INVALID;
}

/*
 * getFlashSize: Gets flash bank size given a flash descriptor.
 */
uint32_t FlashTools::getFlashSize(uint32_t addr) {
    return flash_descriptor[FLASH_DESCRIPTOR_SIZE] == addr || getFlashDescriptor(addr) != NULL ? flash_descriptor[1] : INVALID;
}

/*
 * getPageSize: Gets flash page size given a flash descriptor.
 */
uint32_t FlashTools::getPageSize(uint32_t addr) {
    return flash_descriptor[FLASH_DESCRIPTOR_SIZE] == addr || getFlashDescriptor(addr) != NULL ? flash_descriptor[2] : INVALID;
}

/*
 * getRegionCount: Gets region count for flash bank given an address
 */
uint32_t FlashTools::getRegionCount(uint32_t addr) {
    return flash_descriptor[FLASH_DESCRIPTOR_SIZE] == addr || getFlashDescriptor(addr) != NULL ? flash_descriptor[3] : INVALID;
}

/*
 * getRegionSize: Gets region size given an address                                                                                                   *
 */
uint32_t FlashTools::getRegionSize(uint32_t addr) {
    return flash_descriptor[FLASH_DESCRIPTOR_SIZE] == addr || getFlashDescriptor(addr) != NULL ? flash_descriptor[4] : INVALID;
}

/*
 * getPageCount: Gets total page count for flash bank given an address                                                                                *
 */
uint32_t FlashTools::getPageCount(uint32_t addr) {
    return flash_descriptor[FLASH_DESCRIPTOR_SIZE] == addr || getFlashDescriptor(addr) != NULL ? flash_descriptor[1] / flash_descriptor[2] : INVALID;
}

/*
 * getPageCountPerRegion: Gets page count per region for flash bank                                                                  *
 */
uint32_t FlashTools::getPageCountPerRegion(uint32_t addr) {
    return flash_descriptor[FLASH_DESCRIPTOR_SIZE] == addr || getFlashDescriptor(addr) != NULL ? flash_descriptor[4] / flash_descriptor[2] : INVALID;
}

/*
 * lock: Lock all regions of flash within specified address range
 *  start_addr - Beginning flash address
 *  end_addr   - Ending flash address
 * Returns 0 if successful or Flash Status Register error flag
 */
uint32_t FlashTools::lock(uint32_t start_addr, uint32_t end_addr) {
    
    uint32_t actual_start_addr, actual_end_addr;
    uint16_t start_page, end_page, pages_in_region;
    
    /* Calculate number of pages in region and the actual start/end addresses for lock range */
    pages_in_region   = IFLASH_LOCK_REGION_SIZE / IFLASH_PAGE_SIZE;
    actual_start_addr = start_addr - (start_addr % IFLASH_LOCK_REGION_SIZE);
    actual_end_addr   = end_addr   - (end_addr   % IFLASH_LOCK_REGION_SIZE)  +  IFLASH_LOCK_REGION_SIZE - 1;
    
    /* Calculate start/end page numbers in lock region */
    if (actual_start_addr >= IFLASH1_ADDR) {
        efc        = EFC1;
        start_page = (actual_start_addr - IFLASH1_ADDR) / IFLASH_PAGE_SIZE;
        end_page   = (actual_end_addr   - IFLASH1_ADDR) / IFLASH_PAGE_SIZE;
    } else {
        efc        = EFC0;
        start_page = (actual_start_addr - IFLASH0_ADDR) / IFLASH_PAGE_SIZE;
        end_page   = (actual_end_addr   - IFLASH0_ADDR) / IFLASH_PAGE_SIZE;
    }

    /* Lock all pages in region by setting lock bit. If command fails, return the error code */
    while (start_page < end_page) {
        if (cmd(EFC_FCMD_SLB, start_page) != SUCCESS) {
            return efc->EEFC_FSR & EEFC_ERROR_FLAGS;
        }
        start_page += pages_in_region;
    }
    
    return SUCCESS;
}

/*
 * unlock: Unlocks all regions of flash within specified address range
 *  start_addr - Start flash address
 *  end_addr   - End flash address
 * Returns 0 if successful or Flash Status Register error flag(s)
 */
uint32_t FlashTools::unlock(uint32_t start_addr, uint32_t end_addr) {
    
    uint32_t actual_start_addr, actual_end_addr;
    uint16_t start_page, end_page, pages_in_region;
    
    /* Calculate number of pages in region and the actual start/end addresses for lock range */
    pages_in_region   = IFLASH_LOCK_REGION_SIZE / IFLASH_PAGE_SIZE;
    actual_start_addr = start_addr - (start_addr % IFLASH_LOCK_REGION_SIZE);
    actual_end_addr   = end_addr   - (end_addr   % IFLASH_LOCK_REGION_SIZE)  +  IFLASH_LOCK_REGION_SIZE - 1;
    
    /* Calculate start/end page numbers in lock region */
    if (actual_start_addr >= IFLASH1_ADDR) {
        efc        = EFC1;
        start_page = (actual_start_addr - IFLASH1_ADDR) / IFLASH_PAGE_SIZE;
        end_page   = (actual_end_addr   - IFLASH1_ADDR) / IFLASH_PAGE_SIZE;
    } else {
        efc        = EFC0;
        start_page = (actual_start_addr - IFLASH0_ADDR) / IFLASH_PAGE_SIZE;
        end_page   = (actual_end_addr   - IFLASH0_ADDR) / IFLASH_PAGE_SIZE;
    }

    /* Clear lock bit for all pages in region. If command fails, return the error code */
    while (start_page < end_page) {
        if (cmd(EFC_FCMD_CLB, start_page) != SUCCESS) {
            return efc->EEFC_FSR & EEFC_ERROR_FLAGS;
        } else start_page += pages_in_region;
    }
    
    return SUCCESS;
}

/*
 * islocked: Get the number of locked flash regions within specified address range
 *  start_addr - Start flash address
 *  end_addr   - End flash address
 * Returns the number of locked flash regions on success, error flags in Flash Status Register on failure
 */
uint32_t FlashTools::islocked(uint32_t start_addr, uint32_t end_addr) {
    
    const uint32_t READ_SIZE {32};
    uint16_t start_page, end_page, start_region, end_region;
    
    /* Calculate the start/end page numbers of lock region  */
    if (start_addr >= IFLASH1_ADDR) {
        efc        = EFC1;
        start_page = (start_addr - IFLASH1_ADDR) / IFLASH_PAGE_SIZE;
        end_page   = (end_addr   - IFLASH1_ADDR) / IFLASH_PAGE_SIZE;
    } else {
        efc        = EFC0;
        start_page = (start_addr - IFLASH0_ADDR) / IFLASH_PAGE_SIZE;
        end_page   = (end_addr   - IFLASH0_ADDR) / IFLASH_PAGE_SIZE;
    }
    
    /* Calculate the start/end regions */
    start_region = start_page / (IFLASH_LOCK_REGION_SIZE / IFLASH_PAGE_SIZE);
    end_region   = end_page   / (IFLASH_LOCK_REGION_SIZE / IFLASH_PAGE_SIZE);
    
    /* Send get lock bit command to flash cmd register */
    if (cmd(EFC_FCMD_GLB,0) != SUCCESS) {
        return efc->EEFC_FSR & EEFC_ERROR_FLAGS;
    }
    
    /* Each read corresponds to 32 lock bits - Exclude unrequested regions  */
    uint32_t stat;
    uint32_t involved;
    for (involved = 0, stat = efc->EEFC_FRR; !(involved <= start_region && start_region < (involved + READ_SIZE)); involved += READ_SIZE) {
        stat = efc->EEFC_FRR;
    }
    
    /* Check status of each involved region (must be at least 1 involved region) */
    uint32_t locked_regions {0};
    for (uint32_t bit {start_region - involved}, idx {end_region - start_region + 1}; idx > 0; --idx) {
        if (stat & (1 << (bit))) {
            ++locked_regions;
        }
        if (++bit == READ_SIZE) {
            stat = efc->EEFC_FRR;
            bit  = 0;
        }
    }
    
    /* Return the number of set lock bits for region */
    return locked_regions;
}

/*
 * erase: Erase the entire flash bank at the specified address
 *  addr - Flash bank address
 * Returns 0 if successful or Flash Status Register error flags
 */
uint32_t FlashTools::erase(uint32_t addr) {
    efc = (addr >= IFLASH1_ADDR) ? EFC1 : EFC0;
    return cmd(EFC_FCMD_EA, 0);
}

/*
 * MPUConfigureRegion - Configure a region of memory (main memory or flash)
 *  addr - memory address
 *  size - size of region
 *  region - region to configure (0-7)
 *  tex, c, b, s, ap, xn - access permission parameters - see datasheet pg. 205-209
 */
uint32_t FlashTools::MPUConfigureRegion(uint32_t *addr, uint32_t size, uint32_t region,
                                        uint32_t tex, uint32_t c, uint32_t b, uint32_t s,
                                        uint32_t ap, uint32_t xn) {
    
    /* Data Synchronization Barrier -- see datasheet pg. 75, 149 */
    /* Instruction ensures effect of MPU takes place immediately at the end of context switching */
    __DSB();
    
    /* Instruction Synchronization Barrier -- see datasheet pg. 75, 150 */
    /* Instruction ensures new MPU setting takes effect immediately after programming MPU regions */
    __ISB();
    
    /* MPU CTRL Register -- see datasheet page 202 */
    union {
        uint32_t FULL;
        struct {
            uint32_t ENABLE:1;
            uint32_t HFNMIENA:1;
            uint32_t PRIVDEFENA:1;
            uint32_t RESERVED0:29;
        } SECTION;
    } MPU_CTRL_REGISTER;
    
    /* MPU RBAR Register -- see datasheet page 205 */
    union {
        uint32_t FULL;
        struct {
            uint32_t REGION:4;
            uint32_t VALID:1;
            uint32_t ADDRESS:27;
        } SECTION;
    } MPU_RBAR_REGISTER;
    
    /* MPU RASR Register -- see datasheet page 206 */
    union {
        uint32_t FULL;
        struct {
            uint32_t ENABLE:1;
            uint32_t SIZE:5;
            uint32_t RESERVED0:2;
            uint32_t SRD:8;
            uint32_t B:1;
            uint32_t C:1;
            uint32_t S:1;
            uint32_t TEX:3;
            uint32_t RESERVED1:2;
            uint32_t AP:3;
            uint32_t RESERVED2:1;
            uint32_t XN:1;
            uint32_t RESERVED3:3;
        } SECTION;
    } MPU_RASR_REGISTER;
    
    /* Set MPU Register Base Address Register -- see datasheet pg. 205 */
    MPU_RBAR_REGISTER.FULL            = 0;
    MPU_RBAR_REGISTER.SECTION.REGION  = region;
    MPU_RBAR_REGISTER.SECTION.VALID   = 1;
    // Region size in bytes = 2^(size+1) -- datasheet pg. 207
    MPU_RBAR_REGISTER.SECTION.ADDRESS = ((uint32_t)addr >> 5) & (0xffffffff << (size - 4));
    
    /* MPU Register Attribute and Size Register -- see datasheet pg. 206 */
    MPU_RASR_REGISTER.FULL           = 0;
    MPU_RASR_REGISTER.SECTION.SIZE   = size;
    MPU_RASR_REGISTER.SECTION.ENABLE = 1;
    MPU_RASR_REGISTER.SECTION.SRD    = 0;
    /* See datasheet pg. 207-209 for attribute tables */
    MPU_RASR_REGISTER.SECTION.TEX = tex;
    MPU_RASR_REGISTER.SECTION.C   = c;
    MPU_RASR_REGISTER.SECTION.B   = b;
    MPU_RASR_REGISTER.SECTION.S   = s;
    MPU_RASR_REGISTER.SECTION.AP  = ap;
    MPU_RASR_REGISTER.SECTION.XN  = xn;
    
    /* MPU Control Register -- see datasheet pg. 202 */
    MPU_CTRL_REGISTER.SECTION.PRIVDEFENA = 1;
    MPU_CTRL_REGISTER.SECTION.HFNMIENA   = 0;
    MPU_CTRL_REGISTER.SECTION.ENABLE     = 1;
    
    /* Set MPU Registers */
    mpu->RBAR = MPU_RBAR_REGISTER.FULL;
    mpu->RASR = MPU_RASR_REGISTER.FULL;
    mpu->CTRL = MPU_CTRL_REGISTER.FULL;
    
    return SUCCESS;
}
