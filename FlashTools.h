/* **************************************************************************************************************************************************************
 * FlashTools.h                                                                                                                                                 *
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

#ifndef FlashTools_h
#define FlashTools_h

#include <Arduino.h>

/* ---------------- Register Definitions ---------------- */
typedef volatile       uint32_t RWREG;  /* Read-Write Register */
typedef const volatile uint32_t ROREG;  /* Read-Only Register */

/* ---------------- Embedded Flash Controller Definition -- Datasheet pg. 310 ---------------- */
typedef struct {
    RWREG EEFC_FMR;                /* Flash Mode Register -- Read-Write Register */
    RWREG EEFC_FCR;                /* Flash Command Register -- Write-Only Register */
    ROREG EEFC_FSR;                /* Flash Status Register -- Read-Only Register */
    ROREG EEFC_FRR;                /* Flash Result Register -- Read-Only Register */
} EfcInstance;

/* ---------------- MPU Definition -- Datasheet pg. 200 ---------------- */
typedef struct {
    ROREG TYPE;                    /* MPU Type Register */
    RWREG CTRL;                    /* MPU Control Register */
    RWREG RNR;                     /* MPU Region RNRber Register */
    RWREG RBAR;                    /* MPU Region Base Address Register */
    RWREG RASR;                    /* MPU Region Attribute and Size Register */
    RWREG RBAR_A1;                 /* MPU Alias 1 Region Base Address Register */
    RWREG RASR_A1;                 /* MPU Alias 1 Region Attribute and Size Register */
    RWREG RBAR_A2;                 /* MPU Alias 2 Region Base Address Register */
    RWREG RASR_A2;                 /* MPU Alias 2 Region Attribute and Size Register */
    RWREG RBAR_A3;                 /* MPU Alias 3 Region Base Address Register */
    RWREG RASR_A3;                 /* MPU Alias 3 Region Attribute and Size Register */
} MpuInstance;

/* ---------------- System Control Block  Definition -- Datasheet pg. 200 ---------------- */
typedef struct {
    const volatile uint32_t CPUID; /* CPUID Base Register */
    volatile uint32_t ICSR;        /* Interrupt Control and State Register */
    volatile uint32_t VTOR;        /* Vector Table Offset Register */
    volatile uint32_t AIRCR;       /* Application Interrupt and Reset Control Register */
    volatile uint32_t SCR;         /* System Control Register */
    volatile uint32_t CCR;         /* Configuration Control Register */
    volatile uint8_t  SHP[12];     /* System Handlers Priority Registers (4-7, 8-11, 12-15) */
    volatile uint32_t SHCSR;       /* System Handler Control and State Register */
    volatile uint32_t CFSR;        /* Configurable Fault Status Register */
    volatile uint32_t HFSR;        /* HardFault Status Register */
    volatile uint32_t DFSR;        /* Debug Fault Status Register */
    volatile uint32_t MMFAR;       /* MemManage Fault Address Register */
    volatile uint32_t BFAR;        /* BusFault Address Register */
    volatile uint32_t AFSR;        /* Auxiliary Fault Status Register */
    const volatile uint32_t PFR[2];    /* Processor Feature Register */
    const volatile uint32_t DFR;       /* Debug Feature Register */
    const volatile uint32_t ADR;       /* Auxiliary Feature Register */
    const volatile uint32_t MMFR[4];   /* Memory Model Feature Register */
    const volatile uint32_t ISAR[5];   /* Instruction Set Attributes Register */
    uint32_t RESERVED0[5];
    volatile uint32_t CPACR;            /* Coprocessor Access Control Register */
} ScbInstance;

/* ---------------- Memory mapping definitions -- Datasheet pg. 37 ---------------- */
#define IFLASH0_ADDR             (0x00080000u)             /* Internal Flash 0 Base Address */
#define IFLASH1_ADDR             (0x000C0000u)             /* Internal Flash 1 Base Address */
#define IROM_ADDR                (0x00100000u)             /* Internal ROM Base Address */
#define EFC0_ADDR                (0x400E0A00u)             /* Embedded Flash Controller 0 Base Address */
#define EFC1_ADDR                (0x400E0C00u)             /* Embedded Flash Controller 1 Base Address */
#define MPU_ADDR                 (0xE000E000ul + 0x0D90ul) /* Memory Protection Unit Base Address */
#define SCB_ADDR                 (0xE000E000ul + 0x0D00ul) /* System Control Block Base Address  */

/* --------- Additional flash definitions -- Dtasheet pg. 38 section 9.1.3  --------- */
/* Flash bank size  = 0x000C0000 - 0x00080000 = 0x40000     */
/* Page size        = 256 bytes                             */
/* Lock region size = 256 bytes * 64 pages    = 16384 bytes */
#define IFLASH0_SIZE             (IFLASH1_ADDR - IFLASH0_ADDR)  /* Size of internal flash 1 */
#define IFLASH1_SIZE             (IFLASH0_SIZE)                 /* Size of internal flash 2 */
#define IFLASH_ADDR              (IFLASH0_ADDR)                 /* Internal flash base address */
#define IFLASH_PAGE_SIZE         (256u)                         /* Flash page size */
#define IFLASH_NB_OF_PAGES       (1024u)                        /* Pages per flash bank */
#define IFLASH_LOCK_REGION_PAGES (64u)                          /* Pages per lock region */
#define IFLASH_WORD_SIZE         (sizeof(uint32_t))             /* Word size */
#define IFLASH_LOCK_REGION_SIZE  (IFLASH_PAGE_SIZE * IFLASH_LOCK_REGION_PAGES)      /* Lock region size */
#define IFLASH_WORDS_PER_PAGE    (IFLASH_PAGE_SIZE / IFLASH_WORD_SIZE)              /* Max words per flash page */
#define IFLASH_LAST_PAGE_ADDRESS (IFLASH1_ADDR + IFLASH1_SIZE - IFLASH_PAGE_SIZE)   /* Flash last page address */
#define IFLASH_TOTAL_PAGES       (IFLASH0_NB_OF_PAGES + IFLASH1_NB_OF_PAGES)        /* Total number of pages */
#define CHIP_FLASH_WAIT_STATE    (6u)                                               /* Wait states for flash oeprations */
#define UNIQUE_ID_SIZE           (4u)
#define FLASH_DESCRIPTOR_SIZE    (4u)

/* ---------------- EEFC Flash Mode Register - Datasheet pg. 311 ---------------- */
#define EEFC_FMR_FWS_Pos      8                             /* Flash Wait State - bits 8-11 */
#define EEFC_FMR_FWS_Msk     (0xfu << EEFC_FMR_FWS_Pos)     /* Shift 8 bits to FWS and mask bits on */
#define EEFC_FMR_FWS(value)  ((EEFC_FMR_FWS_Msk & ((value) << EEFC_FMR_FWS_Pos))) /* FWS00000000 */
#define EEFC_FMR_FAM         (0x1u << 24)                   /* Flash Access Mode  - bit 24 */
#define EEFC_FMR_FRDY        (0x1u << 0)                    /* Ready interrupt enable */
#define EEFC_FMR_SCOD        (0x1u << 16)                   /* Optimization disable */
#define EEFC_FMR_CLOE        (0x1u << 26)                   /* Optimization enable */

/* ---------------- EEFC Flash Status Register - Datasheet pg. 313 ---------------- */
#define EEFC_FSR_FRDY        (0x1u << 0)                     /* Flash ready status */
#define EEFC_FSR_FCMDE       (0x1u << 1)                     /* Flash command error status - bit 1 */
#define EEFC_FSR_FLOCKE      (0x1u << 2)                     /* Flash lock error status - bit 2 */
#define EEFC_ERROR_FLAGS     (EEFC_FSR_FLOCKE | EEFC_FSR_FCMDE)  /* EEFC error flag */

/* ---------------- SCB Memory Management Fault Exception Enable ---------------- */
#define SCB_SHCSR_MEMFAULTENA_Pos 16                                  /* SCB SHCSR MEMFAULTENA Position */
#define SCB_SHCSR_MEMFAULTENA_Msk (0x1u << SCB_SHCSR_MEMFAULTENA_Pos) /* SCB SHCSR MEMFAULTENA Mask */

/* ---------------- EFC Commands - Datasheet pg. 303 ---------------- */
#define EFC_FCMD_GETD    0x00  /* Get flash descriptor */
#define EFC_FCMD_WP      0x01  /* Write page */
#define EFC_FCMD_WPL     0x02  /* Write page and lock */
#define EFC_FCMD_EWP     0x03  /* Erase page and write page */
#define EFC_FCMD_EWPL    0x04  /* Erase page and write page then lock */
#define EFC_FCMD_EA      0x05  /* Erase all */
#define EFC_FCMD_SLB     0x08  /* Set lock bit */
#define EFC_FCMD_CLB     0x09  /* Clear lock bit */
#define EFC_FCMD_GLB     0x0A  /* Get lock bit */
#define EFC_FCMD_SGPB    0x0B  /* Set GPNVM bit */
#define EFC_FCMD_CGPB    0x0C  /* Clear GPNVM bit */
#define EFC_FCMD_GGPB    0x0D  /* Get GPNVM bit */
#define EFC_FCMD_STUI    0x0E  /* Start unique ID */
#define EFC_FCMD_SPUI    0x0F  /* Stop unique ID */
#define EFC_FCMD_GCALB   0x10  /* Get CALIB bit */

/* ---------------- Flash Writing Protection Key - Datasheet pg. 312 ---------------- */
#define FWP_KEY (0x5Au)

/* ---------------- EFC / Flash Access Modes - Datasheet pg. 311 ---------------- */
#define FLASH_ACCESS_MODE_128  0
#define FLASH_ACCESS_MODE_64   EEFC_FMR_FAM

/* ---------------- In-Application Programming (IAP) Routine Address - Datasheet pg. 331 ---------------- */
#define IAP_ENTRY_ADDRESS (IROM_ADDR + 8)                 

/* ---------------- Embedded Flash Controller Instances ---------------- */
#define EFC0 ((EfcInstance*)EFC0_ADDR)
#define EFC1 ((EfcInstance*)EFC1_ADDR)

/* ---------------- Return Codes ---------------- */
typedef enum {
    SUCCESS        = 0,
    ERROR          = 0x10,
    INVALID        = 0xFFFFFFFF,
    BIT_IS_SET     = 0x1,
    BIT_IS_CLEARED = 0,
} ReturnCodes;

/* ---------------- FlashTools Class ---------------- */
class FlashTools {
    
    private:
    
        /* EFC Flash Command Register type declaration */
        typedef union {
            uint32_t FULL;
            struct {
                uint32_t FCMD:8;  // Flash Command (8) - bits 0-7
                uint32_t FARG:16; // Flash Argument (16) - bits 8-23
                uint32_t FKEY:8;  // Flash Write Protection Key (8) - bits 23-31
            } SECTION;
        } EEFC_FCR_Type;
    
        /* EFC, MPU, SCB instance pointers */
        EfcInstance *efc;
        MpuInstance *mpu;
        ScbInstance *scb;
    
        /* Function pointer for the IAP routine */
        typedef uint32_t (*IAP_FPTR)(uint32_t EFCidx, uint32_t cmd);
        static IAP_FPTR IAP;
    
        /* Flash wait state and flash access mode values for each EFC instance */
        uint32_t FWS0, FWS1;
        uint32_t FAM0, FAM1;
    
        /* Array for unique ID */
        uint32_t uniqueID[UNIQUE_ID_SIZE];
    
        /* Array for flash descriptor */
        uint32_t flash_descriptor[FLASH_DESCRIPTOR_SIZE + 1];
    
        /* Set flash wait state / set flash access mode */
        void setfws(uint32_t fws);
        void setfam(uint32_t fa_mode);
    
        /* Get flash wait state / get flash access mode */
        uint32_t getfws(void);
        uint32_t getfam(void);
    
        /* Write a command to EFC using IAP routine */
        uint32_t cmd(uint32_t cmd, uint32_t arg);
    
        /* Copy data from write_data to a page of flash */
        uint32_t *flashcpy(uint32_t page_address, const void *write_data,
                           uint32_t offset, uint32_t write_size, uint32_t padding_size);
    
    public:
        /* Constructor / Destructor */
        FlashTools(void);
        ~FlashTools(void);
    
        /* Set EFC instance / Get EFC instance */
        uint32_t setEFC(uint32_t efc_idx);
        uint32_t getEFC(void);
    
        /* Get the MCU's unique ID */
        uint32_t getUniqueID(uint32_t *uBuff);
    
        /* Set/Get GPNVM bits */
        uint32_t setSecurityBit(void);
        uint32_t setBootModeSAMBA(void);
        uint32_t setBootModeFlash(void);
        uint32_t setBootFlash0(void);
        uint32_t setBootFlash1(void);
        uint32_t getSecurityBit(void);
        uint32_t getBootSelectBit(void);
        uint32_t getFlashSelectBit(void);
    
        /* Get flash descriptor / flash information */
        uint32_t *getFlashDescriptor(uint32_t addr);
        uint32_t getFlashId(uint32_t addr);
        uint32_t getFlashSize(uint32_t addr);
        uint32_t getPageSize(uint32_t addr);
        uint32_t getRegionCount(uint32_t addr);
        uint32_t getRegionSize(uint32_t addr);
        uint32_t getPageCount(uint32_t addr);
        uint32_t getPageCountPerRegion(uint32_t addr);
    
        /* Check of region of flash is locked */
        uint32_t islocked(uint32_t start_addr, uint32_t end_addr);

        /* Lock / unlock flash from start_addr to end_addr */
        uint32_t lock(uint32_t start_addr, uint32_t end_addr);
        uint32_t unlock(uint32_t start_addr, uint32_t end_addr);
    
        /* Erase flash at addr */
        uint32_t erase(uint32_t addr);
    
        /* Enable MPU and configure memory region */
        uint32_t MPUConfigureRegion(uint32_t *addr, uint32_t size, uint32_t region,
                                    uint32_t tex, uint32_t c, uint32_t b,
                                    uint32_t s, uint32_t ap, uint32_t xn);
    
        /* Get the adress given page number and (optional) offset a*/
        template <typename Type>
        Type *getPageAddress(uint32_t page_num, uint32_t offset);
    
        /* Get offset from flash bank start at specified page and (optional) offset */
        template <typename Type>
        uint32_t getOffset(uint32_t page_num, uint32_t offset);
    
        /* Write data to flash at addr */
        template<typename Type>
        uint32_t write(uint32_t addr, Type *data, uint32_t size, bool erase, bool lock);
        template<typename Type>
        uint32_t write(Type *addr, Type *data, uint32_t size, bool erase, bool lock);
    
        /* Read single chunk of flash at specified address */
        template <typename Type>
        Type read(uint32_t addr);
        template <typename Type>
        Type read(Type *addr);
    
};

/*
 * getPageAddress: Returns type pointer to flash memory at the beginning of specified page
 *  page_num - Flash page number (flash bank 1: 0-1023, flash bank 2: 1024-2048)
 *  offset (optional) - Offset of memory location in sizeof(Type); default 0
 * Returns pointer to first flash page address or NULL if page number out of bounds
 */
template <typename Type>
Type *FlashTools::getPageAddress(uint32_t page_num, uint32_t offset = 0) {
    // If page number invalid, return NULL
    if (page_num < 0 || page_num >= IFLASH_TOTAL_PAGES) {
        return NULL;
    }
    // Return address
    // Page in flash bank 0 => bank 0 start address + (page size * page number) + offset (optional)
    // Page in flash bank 1 => bank 1 start address + (page size * page number) + offset (optional)
    return (page_num <= IFLASH_NB_OF_PAGES) ? reinterpret_cast<Type *>(IFLASH0_ADDR + (IFLASH_PAGE_SIZE * page_num)) + offset
        : reinterpret_cast<Type *>(IFLASH1_ADDR + (IFLASH_PAGE_SIZE * (page_num % IFLASH_NB_OF_PAGES))) + offset;
}

/*
 * getOffset: Returns the offset of the specified page in sizeof(Type) from the beginning of flash bank
 *  page_num          - Flash page number (flash bank 1: 0-1023, flash bank 2: 1024-2048)
 *  offset (optional) - Offset of the location (in sizeof(Type)) from the beginning of the page
 * Returns the offset of specified page from start of flash bank.
 *
 */
template <typename Type>
uint32_t FlashTools::getOffset(uint32_t page_num, uint32_t offset = 0) {
    // If page number invalid, return NULL
    if (page_num < 0 || page_num >= IFLASH_TOTAL_PAGES) {
        return NULL;
    }
    // Return offset
    // Page in flash bank 0 => (bank 0 size + (page size * page number) / type size) + offset (optional)
    // Page in flash bank 1 => (bank 1 size + (page size * page number) / type size) + offset (optional)
    return (page_num <= IFLASH_NB_OF_PAGES) ? ((IFLASH_PAGE_SIZE * page_num) / sizeof(Type)) + offset
        : ((IFLASH0_SIZE + (IFLASH_PAGE_SIZE * page_num)) / sizeof(Type)) + offset;
}

/*
 * write: Unlocks flash region and writes data to it (1 page at a time)
 *  addr      - Flash address for write to occur
 *  data      - Pointer to data buffer containing data to be written
 *  data_size - Size of data buffer to be written in bytes
 *  erase     - Optional, deafult = true. Erase page before writing
 *  lock      - Optional, deafult = false. Lock page after writing
 * Returns 0 if successful or Flash Status Register error flag
 */
template<typename Type>
uint32_t FlashTools::write(uint32_t addr, Type *data, uint32_t data_size, bool erase = true, bool lock = false) {
    
    /* Validate flash address then unlock flash region */
    if (addr >= IFLASH_LAST_PAGE_ADDRESS + IFLASH_PAGE_SIZE || addr < IFLASH_ADDR || addr & 3) {
        return INVALID;
    } else if (islocked(addr, addr + data_size - 1) && unlock(addr, addr + data_size - 1) != SUCCESS) {
        return ERROR;
    }
    
    /* Determine whether addr is in flash bank 0 or 1 and set appropriate flash bank start
       address and EFC instance (EFC0 for flash bank 0, EFC1 for flash bank 1)             */
    const uint32_t FLASH_START_ADDR {addr >= IFLASH1_ADDR ? IFLASH1_ADDR : IFLASH0_ADDR};
    efc = addr >= IFLASH1_ADDR ? EFC1 : EFC0;

    /* Calcuate page number of addr and offset of addr from start of page */
    uint16_t page_num {(addr - FLASH_START_ADDR) / IFLASH_PAGE_SIZE};
    uint16_t offset   {(addr - FLASH_START_ADDR) % IFLASH_PAGE_SIZE};

    /* Set wait state - 6 wait states for flash operations - datasheet pg. 303 */
    uint32_t fws {getfws()};
    setfws(CHIP_FLASH_WAIT_STATE);

    /* Write all data one flash page at a time until all data has been written */
    for (uint32_t write_size; data_size > 0; data_size -= write_size) {
        
        // Calculate write size: (page size - offset) if >1 page needs to be written, else data_size
        write_size = IFLASH_PAGE_SIZE - offset < data_size ? IFLASH_PAGE_SIZE - offset : data_size;
        
        // Calculate page address and padding size
        uint32_t page_address {FLASH_START_ADDR + page_num * IFLASH_PAGE_SIZE};
        uint16_t padding_size {IFLASH_PAGE_SIZE - offset - write_size};
    
        // Copy 1 page of data to flash in 3 parts: offset, data, padding
        flashcpy(page_address, data, offset, write_size, padding_size);
    
        // Send EFC command. Return error flag on failure
        if (cmd((erase && lock) ? EFC_FCMD_EWPL : (erase) ? EFC_FCMD_EWP : EFC_FCMD_WP, page_num) != SUCCESS) {
            return efc->EEFC_FSR & EEFC_ERROR_FLAGS;
        }
        
        // Adjust data pointer by size of last write and pg num by 1
        // Set offset = 0 after 1st iteration
        data += (write_size/sizeof(Type));
        ++page_num;
        offset = 0;
    }

    /* Restore flash wait state value */
    setfws(fws);
    return SUCCESS;
}

/*
 * write: Pointer version
 */
template<typename Type>
uint32_t FlashTools::write(Type *addr, Type *data, uint32_t size, bool erase = true, bool lock = false) {
    return write<Type>(reinterpret_cast<uint32_t>(addr), data, size, erase, lock);
}

/*
 * read: Reads a single chunk of data from flash
 *  addr - Flash address to be read
 * Returns data stored at flash address or INVALID if address is out of bounds
 */
template <typename Type>
Type FlashTools::read(uint32_t addr) {
    return addr > IFLASH_LAST_PAGE_ADDRESS + IFLASH_PAGE_SIZE || addr < IFLASH0_ADDR ? INVALID : *reinterpret_cast<Type *>(addr);
}

/*
 * read: Pointer version
 */
template <typename Type>
Type FlashTools::read(Type *addr) {
    return read<Type>(reinterpret_cast<uint32_t>(addr));
}

#endif /* FlashTools_h */
