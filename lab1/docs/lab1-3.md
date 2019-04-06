1-3 利用汇编实现 LED 闪烁
===

## 问题探究

1.在这一部分实验中 /dev/sdc2 是否被用到？为什么？

没有被用到，因为kernel7.img中的代码直接被执行，和/dev/sdc2无关。

2.生成 led.img 的过程中用到了 as, ld, objcopy 这三个工具，他们分别有什么作用，我们平时编译程序会用到其中的哪些？

as是汇编器，ld是链接器，objcopy用于将程序转为raw binary格式，去掉symbols和relocation信息。平时编译程序会用到as和ld。