#ifndef MEMMAP_H
#define MEMMAP_H

/**************************************************************************
    RAM MAP
    Total: 2MB (0x80000000 - 0x801FFFFF)
    
    +------------------+----------+--------+----------------------------+
    | Region           | Start    | Size   | Contents                   |
    +------------------+----------+--------+----------------------------+
    | Kernel           | 80000000 | 64KB   | PS1 BIOS kernel            |
    | Code + Globals   | 80010000 | ~1.25MB| Your game executable       |
    | Audio VH         | 800C8000 | 60KB   | VAG header                 |
    | Audio VB         | 800D7000 | 136KB  | VAG audio data             |
    | Audio SEQ        | 800F9000 | 36KB   | SEQ music sequence         |
    | Font TIM         | 80108000 | 32KB   | 8dot font sprite sheet     |
    | Icon TIM 1       | 80110000 | 4KB    | Memcard icon frame 1       |
    | Icon TIM 2       | 80111000 | 4KB    | Memcard icon frame 2       |
    | Icon TIM 3       | 80112000 | 4KB    | Memcard icon frame 3       |
    | Game Assets      | 80113000 | ???    | Sprites, levels, etc       |
    | Stack            | 801FFF00 | ~32KB  | Grows downward             |
    +------------------+----------+--------+----------------------------+

**************************************************************************/

// Kernel
#define MEM_KERNEL_START    0x80000000
#define MEM_KERNEL_SIZE     0x00010000   // 64KB

// Audio — loaded by AUTO dload
#define MEM_VH_ADDR         0x800C8000
#define MEM_VH_SIZE         0x0000F000   // 60KB
#define MEM_VB_ADDR         0x800D7000
#define MEM_VB_SIZE         0x00022000   // 136KB
#define MEM_SEQ_ADDR        0x800F9000
#define MEM_SEQ_SIZE        0x0000F000   // 60KB

// Font
#define MEM_FONT_ADDR       0x80108000
#define MEM_FONT_SIZE       0x00008000   // 32KB

// Memcard icons
#define MEM_ICON1_ADDR      0x80110000
#define MEM_ICON1_SIZE      0x00001000   // 4KB
#define MEM_ICON2_ADDR      0x80111000
#define MEM_ICON2_SIZE      0x00001000   // 4KB
#define MEM_ICON3_ADDR      0x80112000
#define MEM_ICON3_SIZE      0x00001000   // 4KB

// Game asset pool — available for level/sprite data
#define MEM_ASSET_POOL_START 0x80113000
#define MEM_ASSET_POOL_END   0x801F0000
#define MEM_ASSET_POOL_SIZE  (MEM_ASSET_POOL_END - MEM_ASSET_POOL_START)

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