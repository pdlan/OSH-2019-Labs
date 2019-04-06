1-1 树莓派启动过程和原理
===

## 树莓派启动阶段

树莓派启动分为三个阶段。

1.First stage bootloader

树莓派SoC中的ROM储存了第一阶段的启动程序。树莓派通电后，这个程序会初始化一些设备，包括挂载SD卡上的启动分区，并加载第二阶段的启动程序bootcode.bin。

2.Second stage bootloader (bootcode.bin)

这个阶段的启动程序会初始化SDRAM并运行SD卡上的GPU固件start.elf以启动GPU。

3.GPU firmware (start.elf)

start.elf读取config.txt、cmdline.txt等配置文件，将内核加载到内存中，再启动CPU。

4.User code (kernel7.img)

CPU启动后开始执行内核的指令。

## 树莓派的启动和主流的计算机有何不同？

主流的计算机有BIOS或UEFI而树莓派没有。

## 树莓派启动过程中至少用到了哪些文件系统？为什么？

至少用到了FAT32，因为启动分区必须是FAT32文件系统。如果需要启动Linux，一般还需要一个ext2/3/4分区以存放系统根目录的文件。