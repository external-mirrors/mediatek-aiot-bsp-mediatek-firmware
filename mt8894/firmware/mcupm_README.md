
# MCUPM introduction
MCUPM is a hardware module which is used for MCUSYS Power Management.
MCUPM firmware (`mcupm.bin`) is loaded into MCUPM SRAM at system initialization.

## Who uses it
On MT8894 the firmware is shipped gzip-compressed as `mcupm.bin.gz` and embedded
in the FIT image inside `mt8894-fip.img` by the U-Boot binman recipes. U-Boot SPL
loads it from there.

## How to load `mcupm.bin`
Copy `mcupm.bin` to MCUPM SRAM, then set normal boot flag and release software
reset pin of MCUPM.

## Return values
No return value.

## Version
`$ zcat mcupm.bin.gz | strings | grep "MCUPM firmware"`
