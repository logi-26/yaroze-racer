#ifndef MEMMAP_H
#define MEMMAP_H

/**************************************************************************
    RAM MAP
    Total: 2MB (0x80000000 - 0x801FFFFF)
    
    +------------------+----------+--------+----------------------------+
    | Region           | Start    | Size   | Contents                   |
    +------------------+----------+--------+----------------------------+
    | Kernel           | 80000000 | 64KB   | PS1 BIOS kernel            |
    | Monitor + libps  | 80010000 | 512KB  | Net Yaroze monitor, libs   |
    | Assets (AUTO)    | 80090000 | 412KB  | Models, textures, font,    |
    |                  |          |        | icons: packed one after    |
    |                  |          |        | another (to 800F6F90)      |
    | Audio VH         | 800F7000 | 60KB   | VAB header   } reserved:   |
    | Audio VB         | 80106000 | 136KB  | VAB samples  } not loaded  |
    | Audio SEQ        | 80128000 | 60KB   | SEQ music    } yet         |
    | Free             | 80137000 | 164KB  |                            |
    | Program          | 80160000 | ~560KB | Code, globals, GPU buffers |
    | Stack            | 801FFF00 |        | Grows downward             |
    +------------------+----------+--------+----------------------------+


**************************************************************************/

// Kernel
#define MEM_KERNEL_START    0x80000000
#define MEM_KERNEL_SIZE     0x00010000   // 64KB

// Audio — to be loaded by AUTO dload (not loaded yet)
#define MEM_VH_ADDR         0x800F7000
#define MEM_VH_SIZE         0x0000F000   // 60KB
#define MEM_VB_ADDR         0x80106000
#define MEM_VB_SIZE         0x00022000   // 136KB
#define MEM_SEQ_ADDR        0x80128000
#define MEM_SEQ_SIZE        0x0000F000   // 60KB

// Font
#define MEM_FONT_ADDR       0x800C4000
#define MEM_FONT_SIZE       0x00008000   // 32KB

// Memcard icons
#define MEM_ICON1_ADDR      0x800C8040
#define MEM_ICON1_SIZE      0x00001000   // 4KB
#define MEM_ICON2_ADDR      0x800C8100
#define MEM_ICON2_SIZE      0x00001000   // 4KB
#define MEM_ICON3_ADDR      0x800C81C0
#define MEM_ICON3_SIZE      0x00001000   // 4KB

// Stack grows down from top of RAM
#define MEM_STACK_TOP       0x801FFF00

// Total RAM
#define MEM_RAM_START       0x80000000
#define MEM_RAM_END         0x80200000
#define MEM_RAM_TOTAL       0x00200000   // 2MB


/******************************************************************************
TYPED ACCESSORS
******************************************************************************/

#define MEM_VH_PTR      ((unsigned char*)MEM_VH_ADDR)
#define MEM_VB_PTR      ((unsigned char*)MEM_VB_ADDR)
#define MEM_SEQ_PTR     ((unsigned char*)MEM_SEQ_ADDR)
#define MEM_FONT_PTR    ((unsigned char*)MEM_FONT_ADDR)
#define MEM_ICON1_PTR   ((unsigned char*)MEM_ICON1_ADDR)
#define MEM_ICON2_PTR   ((unsigned char*)MEM_ICON2_ADDR)
#define MEM_ICON3_PTR   ((unsigned char*)MEM_ICON3_ADDR)

#endif // MEMMAP_H