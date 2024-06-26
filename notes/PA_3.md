1. **为什么批处理系统会出现？**

为了让计算机自动运行一组程序





2. **TRM + IOE 的系统有什么缺陷？**

程序会独占整个计算机系统，一次只能有一个程序在运行



3. **早期计算机工作模式？**

系统管理员给计算机加载一个特定的程序(其实是上古时期的打孔卡片)，计算机就会一直执行这个程序，直到程序结束或者是管理员手动终止，然后再由管理员来手动加载下一个程序。





4. **批处理系统的诞生？**

让管理员事先准备好一组程序，让计算机执行完一个程序之后，就自动执行下一个程序

要有一个后台程序，当一个前台程序执行结束的时候，后台程序就会自动加载一个新的前台程序来执行



5. **后台处理程序就是操作系统！**



6. **最简单的操作系统？**

在PA中使用的操作系统叫Nanos-lite，它是南京大学操作系统Nanos的裁剪版



7. **获取nanos-lite的方法**

```bash
cd ics2022
bash init.sh nanos-lite
```



8. **将操作系统看作是AM上的一个普通程序！**

和超级玛丽没什么区别

Nanos-lite的实现可以是架构无关的



9. **进程和程序的区别？**

如果你打开了记事本3次，计算机上就会有3个记事本进程在运行，但磁盘中的记事本程序只有一个



10. **NEMU的log和Nanos-lite的log并不是一个东西**

在Nanos-lite中，`Log()`宏通过你在`klib`中编写的`printf()`输出，最终会调用TRM的`putch()`



11. **设备初始化 -> init_device -> ioe_init**



12. **disk 磁盘？**

先让Nanos-lite把其中的一段内存作为磁盘来使用

这样的磁盘有一个专门的名字，叫ramdisk



13. **运行 nanos-lite**

```bash
make ARCH=riscv64-nemu run
make ARCH=native run
```



14. **操作系统就是一个C程序**

GNU/Linux只不过是个巨大的C程序而已



15. **一个最简单的操作系统？**

- 用户程序执行结束之后，可以跳转到操作系统的代码继续执行
- 操作系统可以加载一个新的用户程序来执行



16. **执行流的切换？**

函数调用一般是在一个程序内部发生的(动态链接库除外)，属于程序内部的执行流切换，使用call/jal指令即可实现。

执行流切换的本质，也只是把PC从一个值修改成另一个值而已。



17. **为什么操作系统和用户程序之间的切换不能简单点，将操作系统作为一个特殊的库函数，通过函数调用来进行执行流切换？**

- 一个用户程序出错了，操作系统可以运行下一个用户程序。
- 但如果操作系统崩溃了，整个计算机系统都将无法工作。
- 人们还是希望能把操作系统保护起来，尽量保证它可以正确工作。
- 我们都不希望用户程序可以把执行流切换到操作系统中的任意函数





18. **为什么会出现硬件上的权限管理机制？**

为了阻止程序将执行流切换到操作系统的任意位置

- i386中引入了保护模式(protected mode)和特权级(privilege level)
- mips32处理器可以运行在内核模式和用户模式
- riscv32则有机器模式(M-mode)，监控者模式(S-mode)和用户模式(U-mode)

有高特权级的程序才能去执行一些系统级别的操作

如果一个特权级低的程序尝试执行它没有权限执行的操作，CPU将会抛出一个异常信号，来阻止这一非法行为的发生





19. **操作系统和用户程序的特权级？**

- 操作系统拥有最高的特权级
- 运行在操作系统上的用户程序一般都处于最低的特权级





20. **RISCV 的特权级？**

它存在M，S，U三个特权模式，分别代表机器模式，监管者模式和用户模式。

M模式特权级最高，U模式特权级最低，低特权级能访问的资源，高特权级也能访问。

- 操作系统运行在S模式，因此有权限访问所有的代码和数据。
- 而一般的程序运行在U模式，这就决定了它只能访问U模式的代码和数据。
- 只要操作系统将其私有代码和数据放S模式中，恶意程序就永远没有办法访问到它们。





21. **如何判定进程执行了无权限的操作？即跳转到了不应该访问的函数？**

在硬件上维护一个用于**标识当前特权模式的寄存器**

在访问那些高特权级才能访问的资源时，对当前特权模式进行检查



22. **如何出现了非法的特权级访问，会发生什么？**

CPU将会抛出异常信号，并跳转到一个和操作系统约定好的内存位置，交由操作系统进行后续处理



23. **其他指令集的架构的特权级？**

-  x86的操作系统运行在ring 0，用户进程运行在ring 3。
- mips32的操作系统运行在内核模式，用户进程运行在用户模式。

这些保护相关的概念和检查过程都是通过硬件实现的。



24. **特权级保护机制的本质？**

 在硬件中加入一些与特权级检查相关的门电路(例如比较器电路)，如果发现了非法操作，就会抛出一个异常信号，让CPU跳转到一个约定好的目标位置，并进行后续处理。



25. **硬件漏洞？**

https://meltdownattack.com/

Meltdown and Spectre

它们打破了特权级的边界:  恶意程序在特定的条件下可以以极高的速率窃取操作系统的信息



26. **性能还是安全？**

没有安全，芯片跑得再快，也是徒然



27. **trap指令的实现**

程序执行自陷指令之后，就会陷入到操作系统预先设置好的跳转目标。

这个跳转目标也称为异常入口地址。

这一过程是ISA规范的一部分，称为中断/异常响应机制。

大部分ISA并不区分CPU的异常和自陷！



28. **三种常见指令集的trap机制？**

- **x86** 

提供`int`指令作为自陷指令

异常入口地址是通过门描述符(Gate Descriptor)来指示的

门描述符是一个8字节的结构体，简化图：

```
   31                23                15                7                0
  +-----------------+-----------------+---+-------------------------------+
  |           OFFSET 31..16           | P |          Don't care           |4
  +-----------------------------------+---+-------------------------------+
  |             Don't care            |           OFFSET 15..0            |0
  +-----------------+-----------------+-----------------+-----------------+
```

- **P位**来用表示这一个门描述符是否有效
- **OFFSET**用来指示异常入口地址

用户程序就只能跳转到门描述符中OFFSET所指定的位置

x86把内存中的某一段数据专门解释成一个数组，叫IDT(Interrupt Descriptor Table，中断描述符表)。

```
           |               |
           |   Entry Point |<----+
           |               |     |
           |               |     |
           |               |     |
           +---------------+     |
           |               |     |
           |               |     |
           |               |     |
           +---------------+     |
           |offset |       |     |
           |-------+-------|     |
           |       | offset|-----+
  index--->+---------------+
           |               |
           |Gate Descriptor|
           |               |
    IDT--->+---------------+
           |               |
           |               |
```

数组的一个元素就是一个门描述符。

为了从数组中找到一个门描述符，我们还需要一个索引。

这个索引**由CPU内部产生(例如除零异常为0号异常)**，或者**由`int`指令给出(例如`int $0x80`)**

x86使用**IDTR寄存器来存放IDT的首地址和长度**

执行一条特殊的指令`lidt`，来在IDTR中设置好IDT的首地址和长度

ldt + offset  -> entry



通过`int3`触发的断点异常需要返回到程序的当前状态继续执行，所以需要在进行响应异常的时候保存好程序当前的状态。



**触发异常后硬件的响应过程如下:**

1. 从IDTR中读出IDT的首地址
2. 根据异常号在IDT中进行索引, **找到一个门描述符**
3. 将门描述符中的offset域组合成**异常入口地址**
4. 依次将eflags，cs(代码段寄存器)，**eip(也就是PC)寄存器**的值压栈
5. 跳转到异常入口地址





- **mips**

提供`syscall`指令作为自陷指令

mips32约定，异常入口地址总是`0x80000180`

 mips32提供了一些特殊的系统寄存器，

这些寄存器位于0号协处理器(Co-Processor 0)中，因此也称CP0寄存器。

重要的有三个：

- epc寄存器 - 存放触发异常的PC
- status寄存器 - 存放处理器的状态
- cause寄存器 - 存放触发异常的原因

触发异常后硬件的响应过程如下：

1. 将当前PC值保存到epc寄存器
2. 在cause寄存器中**设置异常号**
3. 在status寄存器中**设置异常标志,**，使处理器进入内核态
4. 跳转到`0x80000180`



- **riscv**

提供`ecall`指令作为自陷指令

提供一个mtvec寄存器来存放异常入口地址

提供了一些特殊的系统寄存器，叫控制状态寄存器(CSR寄存器)。

三个特殊的CSR寄存器：

- mepc寄存器 - 存放触发异常的**PC**
- mstatus寄存器 - 存放处理器的**状态**
- mcause寄存器 - 存放触发异常的**原因**

触发异常后硬件的响应过程如下：

1. 将当前P**C值保存**到mepc寄存器
2. 在mcause寄存器中**设置异常号**
3. 从mtvec寄存器中**取出异常入口地址**
4. **跳转到异常入口**地址



上述保存程序状态以及跳转到异常入口地址的工作，**都是硬件自动完成的**，不需要程序员编写指令来完成相应的内容。

处理过程将会由操作系统来接管，操作系统将视情况决定是否终止当前程序的运行**(例如触发段错误的程序将会被杀死)**

若决定无需杀死当前程序，等到异常处理结束之后，**就根据之前保存的信息恢复程序的状态**，并从异常处理过程中返回到程序触发异常之前的状态。





29. **三种指令集的异常处理返回？**

- x86通过**`iret`**指令从异常处理过程中返回，它**将栈顶的三个元素来依次解释成eip，cs，eflags，并恢复它们**。
- mips32通过**`eret`**指令从异常处理过程中返回，它将清除status寄存器中的异常标志，**并根据epc寄存器恢复PC。**
- riscv32通过**`mret`**指令从异常处理过程中返回，它将**根据mepc寄存器恢复PC。**





30. **状态机视角的异常响应机制？**

TRM + IOE -> S = <R, M> 

R = {GPR, PC, SR}

 异常响应机制和内存无关，因此我们无需对`M`的含义进行修改。



（1）**对R进行扩充**

除了PC和通用寄存器之外，还需要添加上文提到的**一些特殊寄存器**

系统寄存器(System Register)

**R = {GPR, PC, SR}**



（2）**状态转移的修正**

为了描述指令执行失败的行为，我们可以假设CPU有一条虚构的指令`raise_intr`

```
SR[mepc] <- PC
SR[mcause] <- 一个描述失败原因的号码
PC <- SR[mtvec]
```

- 如果一条指令执行成功，其行为和之前介绍的TRM与IOE相同。
- 如果一条指令执行失败，其行为**等价于执行了虚构的`raise_intr`指令**。

==通过引入虚构的raise_intr 指令可以保证每次指令执行都是“成功”的！==

 "一条指令的执行是否会失败"这件事是不是确定性的呢?

- 非法指令可以定义成"不属于ISA手册描述范围的指令"
- 而自陷指令可以认为是一种特殊的无条件失败

RISC-V手册就不认为除0是一种失败。

**因此即使除数为0，在RISC-V处理器中这条指令也会按照指令手册的描述来执行**



**（3）失败条件的数学表示**

```
fex: S -> {0, 1}
```

给定状态机的任意状态`S`，`fex(S)`都可以**唯一表示当前PC指向的指令是否可以成功执行**

- 一个程序运行多次，还是会在相同的地方抛出相同的异常，从而进行相同的状态转移
- IOE的输入指令会引入一些不确定性



**（4）添加了CTE后的计算机系统状态机表示**

![image-20231222085754834](PA_3.assets/image-20231222085754834.png)



**（5）为了管理系统寄存器，需要添加特殊的系统指令**

例如x86的`lidt`，`iret`，riscv的`csrrw`，`mret`等。

这些指令**除了用于专门对状态机中的`SR`进行操作之外**，它们本质上和TRM的计算指令没有太大区别。

TRM的普通计算指令作用于GPR 和 PC，而为了实现CTE的系统指令作用于SR！



31. **上下文管理的抽象？CTE**

ConText Extension

硬件提供的上述在操作系统和用户程序之间切换执行流的功能，

在操作系统看来，都可以划入上下文管理的一部分。

与IOE一样，上下文管理的具体实现也是架构相关的。

x86/mips32/riscv32中分别通过`int`/`syscall`/`ecall`指令来进行自陷。





32. **操作系统处理一个异常需要哪些信息？**

**（1）引发这次执行流切换的原因**

是程序除0，非法指令，还是触发断点，又或者是程序自愿陷入操作系统

根据不同的原因，操作系统都会进行不同的处理



**（2）程序的上下文**

读出上下文中的一些寄存器，根据它们的信息来进行进一步的处理





33. **通过异常机制，通过软件模拟浮点指令的执行？**

去掉浮点处理单元FPU来节省功耗。

如果软件要执行一条浮点指令，处理器就会抛出一个非法指令的异常。

有了异常响应机制，我们**就可以在异常处理的过程中模拟这条非法指令的执行了。**



34. **目前的AM中的浮点指令的执行是UB**

你也可以考虑实现一个简化版的FPU。

如果你的FPU行为正确，也不算违反规定。



35. **stackoverflow 也是UB**



36. **AM究竟给程序提供了多大的栈空间呢?** 



37. **实现CTE的数据结构？**

CTE定义了名为"事件"的如下数据结构。

```c#
typedef struct Event {
  enum { ... } event;
  uintptr_t cause, ref;
  const char *msg;
} Event;
```

其中`event`表示事件编号，`cause`和`ref`是一些描述事件的补充信息，`msg`是事件信息字符串



在AM中，`Context`的具体成员也是由不同的架构自己定义的。

- 操作系统并不需要单独访问`Context`结构中的成员。
- CTE也提供了一些的接口，来让操作系统在必要的时候访问它们，
- 从而**保证操作系统的相关代码与架构无关**。





38. **实现CTE的统一API？**

**（1）cte_init**

```c
bool cte_init(Context* (*handler)(Event ev, Context *ctx))
```

**定义某个事件的处理回调函数，参数就是event和上下文ctx。**





**（2）yield**

```c
void yield()
```

**会触发一个编号为`EVENT_YIELD`事件。不同的ISA会使用不同的自陷指令来触发自陷操作。**





39. **一个完整的触发trap的操作过程？**

**（1）设置异常的入口地址是架构相关的行为，不能放入操作系统，需要放入AM的CTE中。**

使用HAS_CTE宏，然后就会通过init_irq 执行 cte_init，完成异常入口地址的初始化。

- 对x86来说，就是要准备一个有意义的IDT。
- 对于mips32来说，由于异常入口地址是固定在`0x80000180`，因此我们需要在`0x80000180`放置一条无条件跳转指令，使得这一指令的跳转目标是我们希望的真正的异常入口地址即可。
- 对于riscv32来说，**直接将异常入口地址设置到mtvec寄存器中即可。**

```c
asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));
```

`cte_init()`函数做的第二件事是注册一个事件处理回调函数，这个回调函数由Nanos-lite提供



**（2）我们还需要真正触发一次自陷操作**

需要在NEMU中实现`isa_raise_intr()`函数。

模拟异常响应机制。



40. **让DiffTest支持异常响应机制**

- 针对riscv64，你需要将mstatus初始化为`0xa00001800`.





41. **实现指令**

```c
30 57 10 73 csrrw   zero, mtvec, a4
0011 0000 0101 0111 0001 0000 0111 0011
0011_0000_0101 01110 001 00000 1110011
```

![image-20231222094556308](PA_3.assets/image-20231222094556308.png)

![image-20231222095437218](PA_3.assets/image-20231222095437218.png)

All CSR instructions atomically **read-modify-write** a single CSR, **whose CSR specifier is encoded in the 12-bit csr field of the instruction held in bits 31–20**. The immediate forms use a 5-bit **zero-extended** immediate encoded in the rs1 field.

The **CSRRW** (Atomic Read/Write CSR) instruction atomically **swaps values** in the CSRs and integer registers. CSRRW **reads the old value of the CSR, zero-extends the value to XLEN bits**, then **writes it to integer register rd**. The initial value in rs1 is written to the CSR. **If rd=x0, then the instruction shall not read the CSR and shall not cause any of the side effects that might occur on a CSR read**.

**The assembler pseudoinstruction** to write a CSR, CSRW csr, rs1, is encoded as **CSRRW x0, csr, rs1**

```assembly
csrrw rs1, csr, rd
```

```c
if(rd!=0) {
    csr -> tmp (zero-extends) -> rd
    rs1 -> csr
}
```



42. **设置寄存器 mstatus为 0xa00001800**

The mstatus register keeps track of and controls **the hart’s current operating state**. A restricted view of mstatus appears as the sstatus register in the S-level ISA.

![image-20231222112009267](PA_3.assets/image-20231222112009267.png)

```
0xa00001800
1010_0000_0000_0000_0000_0001_1000_0000_0000
```

- MPP = 11
- SXL = 10
- UXL = 10

For RV64 systems, the **SXL** and **UXL** fields are WARL fields that control the value of XLEN for S-mode and U-mode, respectively.

the MPP field is set to the (nominal) **privilege** mode **at the time of the trap**, the MPV bit is set to the value of the virtualization mode V at the time of the trap

<img src="PA_3.assets/image-20231222113240068.png" alt="image-20231222113240068" style="zoom:67%;" />



43. **实现ecall指令**

```c
00000073          	ecall
```

![image-20231222131354811](PA_3.assets/image-20231222131354811.png)

The ECALL instruction is used to make a service request to the execution environment.

![image-20231222131442424](PA_3.assets/image-20231222131442424.png)

We use the term **exception** to refer to **an unusual condition occurring** at run time associated **with an instruction** in the current RISC-V hart. 

We use the term **interrupt** to refer to an **external asynchronous event** that may cause a RISC-V hart to experience an unexpected transfer of control. 

We use the term **trap** to refer to the transfer of control **to a trap handler** caused by either an exception or an interrupt.

For example, in an EEI providing both **supervisor** and **user** mode on harts, an **ECALL** by a user-mode hart will generally result in **a transfer of control to a supervisor-mode handler** running on the same hart





44. **mcause**

When a trap is taken into M-mode, mcause is written with a code indicating the event that caused the trap. Otherwise, mcause is never written by the implementation, though it may be explicitly written by software.

![image-20231222190536977](PA_3.assets/image-20231222190536977.png)

**The Interrupt bit** in the mcause register is set if the trap was caused by an interrupt. 

**The Exception Code field** contains a code identifying the last exception or interrupt.

<img src="PA_3.assets/image-20231222190649195.png" alt="image-20231222190649195" style="zoom: 67%;" />



**45. 实现指令 csrrs**

```c
34 20 22 f3 csrrs   t0, mcause, zero
```

![image-20231222191154567](PA_3.assets/image-20231222191154567.png)

![image-20231222191206890](PA_3.assets/image-20231222191206890.png)

The CSRRS (Atomic Read and Set Bits in CSR) instruction **reads the value of the CSR**, **zero-extends the value to XLEN bits**, and writes it to integer register **rd**. The initial value in integer register rs1 is treated as a **bit mask** that specifies bit positions to be set in the CSR. **Any bit that is high in rs1 will cause the corresponding bit to be set in the CSR**, if that CSR bit is writable. **Other bits** in the CSR **are not explicitly written**

- csr -> tmp (zero-extends) -> rd
- csr = csr | rs1





46. **进入traphandler的之前需要保存上下文寄存器信息，需要注意顺序！**

x86提供了`pusha`指令，用于把通用寄存器的值压栈。

mips32和riscv32则通过`sw`指令将各个通用寄存器依次压栈。





47. **上下文信息包括？**

**（1）通用寄存器**

**（2）触发异常时的PC和处理器状态**

对于x86来说就是eflags，cs和eip

对于mips32和riscv32来说，就是**epc/mepc**和**status/mstatus**寄存器

**（3）异常号**

对于x86，异常号由软件保存

对于mips32和riscv32，异常号已经由硬件保存在cause/mcause寄存器中，我们还需要将其保存在堆栈上

**（4）地址空间**

暂时忽略



异常处理过程可以根据上下文来诊断并进行处理，同时，将来恢复上下文的时候也需要这些信息



48. **异常处理和函数调用的区别？**

异常处理还需要保存额外的信息



49. **触发异常，跳转到 __ am_asm_trap ，然后跳转到  __ am_irq_handle**



50. **do_event进行事件的分发，也就是实际的异常处理回调函数**



51. **上下文的恢复**

`__am_asm_trap()`将根据之前保存的上下文内容，恢复程序的状态。

最后执行"**异常返回指令**"返回到程序触发异常之前的状态。





52. **trap的时候保存的PC对于不同的指令集架构是不同的**

对于x86的`int`指令，保存的是指向其下一条指令的PC，这有点像函数调用。

对于mips32的`syscall`和riscv32的`ecall`，**保存的是自陷指令的PC，因此软件需要在适当的地方对保存的PC加上4，使得将来返回到自陷指令的下一条指令。**





53. **缺页异常返回后的PC不能加4**

- 在系统将故障排除后，将会重新执行相同的指令进行重试，因此异常返回的PC无需加4。
- 所以根据异常类型的不同，有时候需要加4，有时候则不需要加。



54. **是否需要加4？**

1）CISC

交给硬件来做

2）RISC

交给软件来做





55. **添加新的指令 mret**

![image-20231223104518882](PA_3.assets/image-20231223104518882.png)

就是将pc 设置为 epc，但是具体epc是否需要加4，需要在软件中进新设置！



56. **添加etrace，即描述异常处理的踪迹**

不要在AM的 CTE 中添加etace，而是在NEMU中添加！





57. **加载用户程序**

程序中包括代码和数据，它们都是存储在可执行文件中。

为了实现loader函数，需要考虑下面4个问题：

- 可执行文件在哪里?
- 代码和数据在可执行文件的哪个位置?
- 代码和数据有多少?
- "正确的内存位置"在哪里?



由于运行时环境的差异，我们不能把编译到AM上的程序放到操作系统上运行

需要将程序转化为 **“能够运行在操作系统上的用户程序”**！



58. **添加Navy-apps**

https://github.com/NJU-ProjectN/navy-apps

```bash
cd ics2022
bash init.sh navy-apps
```



59. **newlib**

https://sourceware.org/newlib/

navy-apps/libs/libc



60. **程序入口**

navy-apps/libs/libos/src/crt0/start/$ISA.S 的 _start 函数

ctr0 的 ctr 就是 C RunTime

`_start()`函数会调用`navy-apps/libs/libos/src/crt0/crt0.c`中的`call_main()`函数，

然后调用用户程序的`main()`函数，从`main()`函数返回后会调用`exit()`结束运行。



61. **运行第一个用户程序 dummy**

在`navy-apps/tests/dummy/`目录下执行

```shell
make ISA = riscv64
```

编译成功后把`navy-apps/tests/dummy/build/dummy-riscv64`手动复制并重命名为`nanos-lite/build/ramdisk.img`, 然后在`nanos-lite/`目录下执行

```shell
make ARCH=riscv64-nemu
```



ramdisk镜像文件包含在了nanos-lite中，成为了其一部分。

`nanos-lite/src/resources.S`



62. **可执行文件在哪里？**

位于ramdisk偏移为0处，访问它就可以得到用户程序的第一个字节。



63. **可执行文件的组织？**

ELF文件格式包含程序本身的代码和静态数据，还包括一些用来描述它们的信息

不同组织形式形成了不同格式的可执行文件

例如

- Windows主流的可执行文件是PE(Portable Executable)格式
- GNU/Linux主要使用ELF(Executable and Linkable Format)格式

ELF是GNU/Linux可执行文件的标准格式

https://stackoverflow.com/questions/2171177/what-is-an-application-binary-interface-abi



64. **堆和栈没有放入可执行文件中，而是通过AM进行管理**



65. **两种视角看待一个可执行ELF文件**

- 面向链接过程的section视角，这个视角提供了用于**链接与重定位**的信息(例如符号表)
- 面向执行的segment视角，这个视角提供了用于**加载可执行文件**的信息

一个segment可能由0个或多个section组成，但一个section可能不被包含于任何segment中



66. **加载程序需要关注segment的视角**

ELF中采用program header table来管理segment，program header table的一个表项描述了一个segment的所有属性，包括**类型，虚拟地址，标志，对齐方式，以及文件内偏移量和segment大小**。



67. **通过PT_LOAD来判断一个segment是否需要加载**

加载一个可执行文件并不是加载它所包含的所有内容，只要加载那些与运行时刻相关的内容就可以了，例如调试信息和符号表就不必加载。



68. **FileSize 和 MemSize ？**

一些bss段在磁盘上是不占用空间的，但是一旦加载到内存就会占用空间。

所以 MemSize >= FileSize



69. **程序从何而来？**

https://www.tenouk.com/ModuleW.html#google_vignette



70. **如何加载一个segment**

```
      +-------+---------------+-----------------------+
      |       |...............|                       |
      |       |...............|                       |  ELF file
      |       |...............|                       |
      +-------+---------------+-----------------------+
      0       ^               |              
              |<------+------>|       
              |       |       |             
              |       |                            
              |       +----------------------------+       
              |                                    |       
   Type       |   Offset    VirtAddr    PhysAddr   |FileSiz  MemSiz   Flg  Align
   LOAD       +-- 0x001000  0x03000000  0x03000000 +0x1d600  0x27240  RWE  0x1000
                               |                       |       |     
                               |   +-------------------+       |     
                               |   |                           |     
                               |   |     |           |         |       
                               |   |     |           |         |      
                               |   |     +-----------+ ---     |     
                               |   |     |00000000000|  ^      |   
                               |   | --- |00000000000|  |      |    
                               |   |  ^  |...........|  |      |  
                               |   |  |  |...........|  +------+
                               |   +--+  |...........|  |      
                               |      |  |...........|  |     
                               |      v  |...........|  v    
                               +-------> +-----------+ ---  
                                         |           |     
                                         |           |    
                                            Memory
```

相对文件偏移`Offset`指出相应segment的内容从ELF文件的第`Offset`字节开始，在文件中的大小为`FileSiz`，它需要被分配到以`VirtAddr`为首地址的虚拟内存位置，在内存中它占用大小为`MemSiz`。

注意MemSiz 多出来的部分需要清0

这个segment使用的内存就是`[VirtAddr, VirtAddr + MemSiz)`这一连续区间

- 然后将segment的内容从ELF文件中**读入到这一内存区间**，

- 并**将**`[VirtAddr + FileSiz, VirtAddr + MemSiz)`**对应的物理区间清零**。



71. **实现一个 loader可能需要用的接口**

```c
// 从ramdisk中`offset`偏移处的`len`字节读入到`buf`中
size_t ramdisk_read(void *buf, size_t offset, size_t len);

// 把`buf`中的`len`字节写入到ramdisk中`offset`偏移处
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

// 返回ramdisk的大小, 单位为字节
size_t get_ramdisk_size();
```



72. **程序的本质？**

程序的最为原始的状态：比特串

加载程序其实就是**把这一毫不起眼的比特串放置在正确的位置**，但这其中又折射出"**存储程序**"的划时代思想。

操作系统将控制权交给它的时候, 计算机把它解释成指令并逐条执行。

loader让计算机的生命周期突破程序的边界：

一个程序结束并不意味着计算机停止工作，**计算机将终其一生履行执行程序的使命**。



73. **添加一个判定魔数的assert**

```c
assert(*(uint32_t *)elf->e_ident == 0xBadC0de);
```



74. **不仅需要让 nanos-lite在riscv64-nemu上跑起来，还需要在native上跑起来**





75. **elf文件的手册和资料**

```shell
man 5 elf
```



76. **实现loader需要看的**

- init_proc
- -> loader
- -> naive_uload



77. **Executable File format**

![image-20231223163447470](PA_3.assets/image-20231223163447470.png)





78. **操作系统作为最高权限，用于管理计算机的资源**

作为资源管理者管理着系统中的所有资源，操作系统还需要为用户程序提供相应的服务。这些服务需要以一种统一的接口来呈现，用户程序也只能通过这一接口来请求服务。

这一接口就是系统调用。这是操作系统从诞生那一刻就被赋予的使命。





79. **操作系统的内核区和用户区**

- 访问系统资源的功能会放到内核区中实现。
- 用户区则保留一些无需使用系统资源的功能。

以及用于**请求系统资源相关服务的系统调用接口**

用户程序只能在用户区安分守己地"计算"，任何超越纯粹计算能力之外的任务，**都需要通过系统调用向操作系统请求服务**。

如果用户程序尝试**进行任何非法操作**，CPU就会向操作系统抛出一个异常信号，让非法操作的指令执行"失败"，并交由操作系统进行处理。



80. **系统调用不能提供操作系统的私有数据**

如果一个恶意程序获得了这些信息，可能会为恶意攻击提供了信息基础。

保证安全是OS首先需要做到的！



81. **系统调用的参数获取？**

对用户程序来说，用来向操作系统描述需求的最方便手段就是**使用通用寄存器**。

因为执行自陷指令之后, **执行流就会马上切换到事先设置好的入口**，通用寄存器也会作为上下文的一部分被保存起来。

系统调用处理函数只需要**从上下文中获取必要的信息**，就能知道用户程序发出的服务请求是什么了。



82. **具体是哪个系统调用的识别？**

看系统调用号，RISCV64是a7寄存器

使用 void do_syscall(Context *c) 这个接口！



83. **查询不同架构的系统调用约定**

```shell
man syscall
man syscalls
```



84. **添加strace观察系统调用的行为**

计算机只做两件事：

1. 本地计算
2. 通过系统调用请求操作系统来完成那些本地计算无法完成的工作

Linux下安装strace

```shell
sudo apt install strace
```

Linux 下使用strace

```shell
strace ls
strace strace l
```



85. **操作系统的TRM**

为了满足程序的基本计算能力, 需要有哪些条件:

- **机器提供基本的运算指令**
- 能输出字符
- 有堆区可以动态申请内存
- **可以结束运行**

基本的运算指令还是得靠机器提供，也就是已经实现的指令系统。

结束运行，`SYS_exit`系统调用也已经提供了。

为了**向用户程序提供输出字符和内存动态申请的功能**，

我们需要实现更多的系统调用。



86. **实现标准输出**

查看手册：

```shell
man 2 write
```

```c
ssize_t write(int fd, const void *buf, size_t count);
```

检查`fd`的值，如果`fd`是`1`或`2`(分别代表`stdout`和`stderr`)，则将`buf`为首地址的`len`字节输出到串口(使用`putch()`即可)。

记得修改接口函数 _write()

库函数中的printf内部就会调用系统调用write ！



87. **系统调用对比库函数**

系统调用本身对操作系统的各种资源进行了抽象。

但为了给上层的程序员提供**更好的接口(beautiful interface)**。

库函数会再次**对部分系统调用再次进行抽象**。



具体的例子：

（1）`fwrite()`这个库函数用于往文件中写入数据，在GNU/Linux中，它封装了`write()`系统调用。

（2）系统调用依赖于具体的操作系统，因此库函数的封装也提高了程序的可移植性。





88. **为什么需要有库函数？**

 库函数的抽象确实方便了程序员，使得他们不必关心系统调用的细节。



89. **如何测试实现的write系统调用？**

运行navy-apps/tests/hello 即可。





90. **如何实现malloc和free函数？**

klibc 中的malloc和free？

**它们的作用是在用户程序的堆区中申请/释放一块内存区域。**

一个用户程序可用的内存区域是需要经过操作系统的分配和管理的。

操作系统做的事情就是通过sbrk对用户程序的堆区大小进行调整。

```c
void* sbrk(intptr_t increment);
```

`malloc()`被第一次调用的时候，会通过`sbrk(0)`来查询用户程序当前program break的位置，之后就可以通过后续的`sbrk()`调用来**动态调整用户程序program break的位置了**。



91. **malloc和sbrk？**

用户程序在第一次调用`printf()`的时候会尝试通过`malloc()`申请一片缓冲区，来存放格式化的内容。

若申请失败，就会**逐个字符进行输出**。



92. **编写sbrk系统调用**

`_sbrk()`通过记录的方式来对用户程序的program break位置进行管理。

1. program break一开始的位置位于`_end`
2. 被调用时，根据记录的program break位置和参数`increment`，计算出新program break
3. 通过`SYS_brk`系统调用来让操作系统设置新program break
4. 若`SYS_brk`系统调用成功，该系统调用会返回`0`，此时更新之前记录的program break的位置，**并将旧program break的位置作为`_sbrk()`的返回值返回**
5. 若该系统调用失败，`_sbrk()`会返回`-1`



93. **查询手册**

```shell
man 2 sbrk
man 3 end
```

**etext**

This is the first address past the end of the text segment (the program code).   

**edata**

This is the first address past the end of the initialized data segment.

**end**

This is the first address past the end of the uninitialized data segment (also known as the BSS segment).



94. **注意事项**

- 不要在`_sbrk()`中通过`printf()`进行输出
- 可以通过`sprintf()`先把调试信息输出到一个字符串缓冲区中，然后通过`_write()`进行输出



95. **实现sbrk 的目标**

`printf()`不再是逐个字符地通过`write()`进行输出，而是将格式化完毕的字符串通过一次性进行输出



96. **使用批处理（batching）技术加速系统调用**

 将一些简单的任务累积起来，然后再一次性进行处理

缓冲区是批处理技术的核心，libc中的`fread()`和`fwrite()`正是通过缓冲区来将数据累积起来，然后再通过一次系统调用进行处理。

例如通过一个1024字节的缓冲区，**就可以通过一次系统调用直接输出1024个字符，而不需要通过1024次系统调用来逐个字符地输出**。

显然，后者的开销比前者大得多。

http://arkanis.de/weblog/2017-01-05-measurements-of-system-call-performance-and-overhead



97. **为什么再printf后面最好加上 `\n`？**

`fwrite()`的实现中有缓冲区，`printf()`打印的字符不一定会马上通过`write()`系统调用输出，但遇到`\n`时**可以强行将缓冲区中的内容进行输出。**

navy-apps/libs/libc/src/stdio/wbuf.c



98. **必答的问题**

我们知道`navy-apps/tests/hello/hello.c`只是一个C源文件，它会被编译链接成一个ELF文件。

- hello程序一开始在哪里？
- 它是怎么出现内存中的？
- 为什么会出现在目前的内存位置？
- 它的第一条指令在哪里？
- 究竟是怎么执行到它的第一条指令的？
- hello程序在不断地打印字符串，每一个字符又是经历了什么才会最终出现在终端上？



99. **支持多个ELF的ftrace**

我们可以让NEMU的ftrace支持多个ELF，如果一个地址不属于某个ELF中的任何一个函数，那就尝试下一个ELF。ftrace就可以**同时追踪Nanos-lite和用户程序的函数调用**了。

这个问题不好解决。。。

Nanos-lite 的 ELF 文件在 NEMU中读取，但是 用户程序的ELF呢？？



用户程序的ELF和nanos-lite 的elf文件在同一个目录下，这就好办了！



100. **ftrace需要哦修改一下**

只需要关注一个函数的调用还有返回

所以

- call 就是 某个地址 调用了某个函数
- ret 就是 某个地址 返回了某个函数





101. **文件系统的设计**

要实现一个完整的批处理系统，我们还需要向系统提供多个程序。

操作系统还需要在存储介质的驱动程序之上为用户程序提供一种更高级的抽象，那就是文件。





102. **文件？**

文件的本质就是字节序列，另外还由一些额外的属性构成。

这样，那些**额外的属性就维护了文件到ramdisk存储位置的映射**。



103. **实现的简易文件系统功能？**

sfs(Simple File System)

- 
  每个文件的大小是固定的
- 写文件时不允许超过原有文件的大小
- 文件的数量是固定的，不能创建新文件
- 没有目录



从ramdisk的最开始一个挨着一个地存放：

```c
0
+-------------+---------+----------+-----------+--
|    file0    |  file1  |  ......  |   filen   |
+-------------+---------+----------+-----------+--
 \           / \       /            \         /
  +  size0  +   +size1+              + sizen +
```



104. **记录文件的名字和大小？**

使用文件记录表

```c
typedef struct {
    char *name;         // 文件名
    size_t size;        // 文件大小
    size_t disk_offset;  // 文件在ramdisk中的偏移
} Finfo;
```

开启 HAS_NAVY = 1

```shell
make ARCH=riscv64-nemu update
```



105. **如果你修改了Navy中的内容，记得通过update更新镜像文件**





106. **sfs 的特别之处**

直接使用绝对路径作为文件名。

由于sfs没有目录，我们把目录分隔符`/`也认为是文件名的一部分，**例如`/bin/hello`是一个完整的文件名**。

这种做法其实也隐含了目录的层次结构，对于文件数量不多的情况，这种做法既简单又奏效



107. **文件读写接口**

```c
size_t read(const char *filename, void *buf, size_t len);
size_t write(const char *filename, const void *buf, size_t len);
```

上面的接口有缺陷：无法用文件名标识标准输入和输出，所以接口需要修改为：

```c
int open(const char *pathname, int flags, int mode);
size_t read(int fd, void *buf, size_t len);
size_t write(int fd, const void *buf, size_t len);
int close(int fd);
```

通过一个编号来表示文件，这个编号就是文件描述符(file descriptor)。

一个文件描述符对应一个正在打开的文件，由操作系统来维护文件描述符到具体文件的映射。

于是我们很自然地通过`open()`系统调用来打开一个文件，并返回相应的文件描述符。



108. **引入文件的读写字节偏移量**

不希望每次读写操作都需要从头开始，于是我们需要为每一个已经打开的文件引入偏移量属性`open_offset`，来记录目前文件操作的位置。

每次对文件读写了多少个字节，偏移量就前进多少。

为了简化实现，我们还是把偏移量放在文件记录表中进行维护。





109. **使用lseek调整offset字节偏移量**

````c
size_t lseek(int fd, size_t offset, int whence);
````



110. **三个特殊的文件描述符**

```c
#define FD_STDIN 0
#define FD_STDOUT 1
#define FD_STDERR 2
```

标准输入`stdin`，标准输出`stdout`和标准错误`stderr`

- printf 调用 write(FD_STDOUT, buf, len)
- scanf 调用 read(FD_STDIN, buf, len)



111. **文件的通用接口**

```c
int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);
```

- 因此"`fs_open()`**没有找到`pathname`所指示的文件"属于异常情况**，需要使用assertion**终止程序运行**。
- 允许所有用户程序都可以对所有已存在的文件进行读写，在实现`fs_open()`的时候**就可以忽略`flags`和`mode`了**。
- 使用`ramdisk_read()`和`ramdisk_write()`**来进行文件的真正读写**。
- 由于文件的大小固定，在实现`fs_read()`， `fs_write()`和`fs_lseek()`的时候，**注意偏移量不要越过文件的边界**。
- 除了写入`stdout`和`stderr`之外(用`putch()`输出到串口)，其余对于`stdin`，`stdout`和`stderr`这三个特殊文件的操作**可以直接忽略。**
- sfs没有维护文件打开的状态，`fs_close()`可以直接返回`0`，**表示总是关闭成功**。



112. **查manual**

```shell
man 2 open
man 2 read
man 2 write
man 2 lseek
man 2 close
```

其中`2`表示查阅和系统调用相关的manual page、



113. **通过 navy-apps/tests/file-test**

添加到`navy-apps/Makefile`的`TESTS`变量中





114. **为了让strace可读性高，可以将fd翻译成 字符串，即每次输出的不是fd，而是fd对应的文件名**





115. **如何更加深刻理解 Unix 中 的一切都是文件？**

Everything is a file

（1）计算机系统中的对象都是**字节序列**。也就是文件的本质。

（2）用一种方式对设备的功能进行抽象，向用户程序提供统一的接口。

（3）**内存**就是通过字节编址，那么内存也就是一个字节序列。

（4）**管道**(shell命令中的`|`)是一种先进先出的字节序列，本质上它是内存中的一个队列缓冲区。

（5）**磁盘**也可以看成一个字节序列，为磁盘上的每一个字节进行编号，例如第x柱面第y磁头第z扇区中的第n字节，把磁盘上的所有字节按照编号的大小进行排列，便得到了一个字节序列。

（6）**socket(网络套接字)**也是一种字节序列，它有一个缓冲区，负责存放接收到的网络数据包，上层应用将socket中的内容看做是字节序列，并通过一些特殊的文件操作来处理它们。可以通过fgetc 和 fputc 对socket进行操作。

（7）操作系统的一些信息可以以字节序列的方式暴露给用户，例如CPU的配置信息。

（8）随机数生成器，也可以看成一个无穷长的字节序列。

（9）非存储类型的硬件也可以看成是字节序列。

- 在键盘上**按顺序敲入按键的编码**形成了一个字节序列。
- 显示器上**每一个像素的内容按照其顺序**也可以看做是字节序列。



为不同的事物提供了统一的接口：

我们可以使用文件的接口来操作计算机上的一切，而不必对它们进行详细的区分。

每个程序采用文本文件作为输入输出，这样可以使程序之间易于合作。





116. **一切皆文件的好处？**

以十六进制的方式查看磁盘上的内容

````shell
head -c 512 /dev/sda | hd
````

查看CPU是否有Spectre漏洞

```bash
cat /proc/cpuinfo | grep 'spectre'
```

"小星星"示例音，也是通过简单的文件操作暴力拼接而成的

```bash
cat Do.ogg Do.ogg So.ogg So.ogg La.ogg La.ogg So.ogg > little-star.ogg
```

#include "/dev/urandom"

将urandom设备中的内容ddd包含到源文件中。





117. **虚拟文件系统？**

我们不仅需要对普通文件进行读写，还需要支持各种"特殊文件"的操作。

扩展后的API不仅可以对普通文件进行读写，还可以对特殊文件进行读写！

这组扩展语义之后的API有一个酷炫的名字，叫VFS(虚拟文件系统)。





118. **真实的文件系统？**

- 比如熟悉Windows的你应该知道管理普通文件的NTFS，
- 目前在GNU/Linux上比较流行的则是EXT4。
- 至于特殊文件的种类就更多了，于是相应地有`procfs`, `tmpfs`, `devfs`, `sysfs`, `initramfs`。



119. **VFS其实是对不同种类的真实文件系统的抽象**

它用一组API来描述了这些真实文件系统的抽象行为，屏蔽了真实文件系统之间的差异。

**上层模块(比如系统调用处理函数)不必关心当前操作的文件具体是什么类型**。

只要调用这一组API即可完成相应的文件操作。



120. **有了VFS，就可以十分容易地支持一个新的文件系统。**

只要把真实文件系统的访问方式包装成VFS的API，上层模块无需修改任何代码。



121. **Finfo 中的read 和 write 读写指针？**

```c
typedef struct {
  char *name;         // 文件名
  size_t size;        // 文件大小
  size_t disk_offset;  // 文件在ramdisk中的偏移
  ReadFn read;        // 读函数指针
  WriteFn write;      // 写函数指针
} Finfo;
```

有了这两个函数指针，我们只需要在文件记录表中对不同的文件设置不同的读写函数，就可以通过`f->read()`和`f->write()`的方式来调用具体的读写函数。用于指向真正进行读写的函数，并返回成功读写的字节数。



122. **C 语言也可以实现面向对象！**

例如通过**结构体**来实现**类的定义**，结构体中的**普通变量**可以看作**类的成员**，**函数指针**就可以看作**类的方法**，给**函数指针设置不同的函数**可以实现**方法的重载**。

OOP中那些看似虚无缥缈的概念也没比C语言高级到哪里去，**只不过是OOP的编译器帮我们做了更多的事情，编译成机器代码之后，OOP也就不存在了**。



123. **如何用ANSI-C来模拟OOP的各种概念和功能。**

https://www.cs.rit.edu/~ats/books/ooc.pdf



124. **不同文件的读写函数指针？**

当上述的函数指针为`NULL`时，表示相应文件是一个普通文件。



125. **块设备 VS. 字符设备**

**块设备：**

把文件看成字节序列，大部分字节序列都是"静止"的。例如对于ramdisk和磁盘上的文件，如果我们不对它们进行修改，它们就会一直位于同一个地方，这样的字节序列具有"位置"的概念。**支持lseek操作。**

**字符设备：**

一些特殊的字节序列并不是这样，例如键入按键的字节序列是"流动"的，被读出之后就不存在了，这样的字节序列中的字节之间只有顺序关系，但无法编号，因此它们没有"位置"的概念。**不支持lseek操作。**



126. **通过VFS 将IOE抽象为文件**

**（1）串口**

`stdout`和`stderr`都会输出到串口。

修改文件读写函数调用 read 和 write指针就可以了。



**（2）时钟**

实现sys_gettimeofday这个系统调用。

新增一个timer-test测试，通过gettimeofday获取当前事件，每0.5s输出一句话。

==需要注意，printf的最后一定要通过\n 换行强制输出==

实现 NDL 的和时钟相关的API

```c
// 以毫秒为单位返回系统时间
uint32_t NDL_GetTicks();
```

\#include <NDL.h> 需要在Makfile中添加一个 LIBS += libndl



**（3）键盘**

按键信息本质就是事件，可以将事件转化为文本。

- 按下按键事件, 如`kd RETURN`表示按下回车键
- 松开按键事件, 如`ku A`表示松开`A`键

kd -> key do

ku -> key undo

按键名称与AM中的定义的按键名相同，**均为大写**。

此外，一个事件**以换行符`\n`结束**。

上述事件抽象成一个特殊文件`/dev/events`，它**需要支持读操作**。

用户程序可以从中读出按键事件，但它不必支持`lseek`，因为**它是一个字符设备**。

```c
// 读出一条事件信息, 将其写入`buf`中, 最长写入`len`字节
// 若读出了有效的事件, 函数返回1, 否则返回0
int NDL_PollEvent(char *buf, int len);
```

实现将按键输入抽象为文件：

**实现 events_read -> 支持 /dev/events 这个设备 -> 实现 NDL_PollEvent。**

**写完后，需要通过 event-test ！**



**（4）VGA**

支持设备：

**`/dev/fb`(fb为frame buffer之意)**

程序为了更新屏幕，只需要将像素信息写入VGA的显存即可

**显存本身也是一段存储空间，它以行优先的方式存储了将要在屏幕上显示的像素**

它需要支持写操作和`lseek`，以便于把像素更新到屏幕的指定位置上。

```c
// 打开一张(*w) X (*h)的画布
// 如果*w和*h均为0, 则将系统全屏幕作为画布, 并将*w和*h分别设为系统屏幕的大小
void NDL_OpenCanvas(int *w, int *h);

// 向画布`(x, y)`坐标处绘制`w*h`的矩形图像, 并将该绘制区域同步到屏幕上
// 图像像素按行优先方式存储在`pixels`中, 每个像素用32位整数以`00RRGGBB`的方式描述颜色
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h);
```

什么是画布？

这样程序就**无需关心系统屏幕的大小**，以及需要将图像绘制到系统屏幕的**哪一个位置**

根据系统**屏幕大小以及画布大小**，来**决定将画布"贴"到哪里**

例如贴到**屏幕左上角或者居中**，从而**将画布的内容写入到frame buffer中正确的位置**



屏幕大小的信息怎么读取？

通过`/proc/dispinfo`文件来获得，它需要支持读操作

具体数值需要通过IOE的接口实现。



（1）实现 **dispinfo_read**

这个文件不支持`lseek`，可忽略`offset`



（2）实现 **NDL_OpenCanvas**

只需要**记录画布的大小**就可以了，当然我们要求画布大小不能超过屏幕大小。

运行 bmp-test ，可以通过printf 输出解析的屏幕大小。



（3）在 **init_fs** 中完成 /dev/fb 的初始化



（4）实现 **fb_write**

把`buf`中的`len`字节**写到屏幕上`offset`处**。

**先从`offset`计算出屏幕上的坐标**，然后调用IOE来进行绘图。

每次绘图后总是马上将frame buffer中的内容同步到屏幕上。



（5）实现 **NDL_DrawRect**

往`/dev/fb`中的正确位置写入像素信息来绘制图像。

需要梳理清楚

- 系统屏幕(即frame buffer)，
-  `NDL_OpenCanvas()`打开的画布，
- 以及`NDL_DrawRect()`指示的绘制区域

之间的位置关系。



127. **需要注意，malloc如果分配失败，并不会自动调用  sbrk，所以可能会导致分配失败时返回一个NULL！**

