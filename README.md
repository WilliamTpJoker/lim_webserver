# 2024/01/01
## 多线程性能下滑问题->前端锁导致(已解决)
异步日志构造完成，进行初次压力测试（测试环境为ubuntu 4核心，100length c-string）
``` bash
测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:2.38891s
每秒输出日志数量：418601条
每秒输出日志大小：40879KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程1:  输出数量333333 耗时:2.77151s
线程0:  输出数量333333 耗时:2.92703s
线程2:  输出数量333333 耗时:2.93051s
每秒输出日志数量：341237条
每秒输出日志大小：33323KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程3:  输出数量100000 耗时:7.02885s
线程8:  输出数量100000 耗时:7.64639s
线程2:  输出数量100000 耗时:7.69952s
线程9:  输出数量100000 耗时:7.7421s
线程4:  输出数量100000 耗时:7.92021s
线程5:  输出数量100000 耗时:8.06304s
线程1:  输出数量100000 耗时:8.06482s
线程7:  输出数量100000 耗时:8.09969s
线程6:  输出数量100000 耗时:8.11577s
线程0:  输出数量100000 耗时:8.14125s
每秒输出日志数量：122831条
每秒输出日志大小：11995KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:0.366028s
每秒输出日志数量：2732031条
每秒输出日志大小：266799KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程1:  输出数量333333 耗时:0.335103s
线程0:  输出数量333333 耗时:0.350824s
线程2:  输出数量333333 耗时:0.394806s
每秒输出日志数量：2532887条
每秒输出日志大小：247352KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量100000 耗时:0.803354s
线程3:  输出数量100000 耗时:0.900471s
线程1:  输出数量100000 耗时:0.914625s
线程2:  输出数量100000 耗时:1.02606s
线程4:  输出数量100000 耗时:1.01842s
线程5:  输出数量100000 耗时:1.01299s
线程9:  输出数量100000 耗时:1.04669s
线程8:  输出数量100000 耗时:1.04988s
线程6:  输出数量100000 耗时:1.0489s
线程7:  输出数量100000 耗时:1.07094s
每秒输出日志数量：933763条
每秒输出日志大小：91187KB
```
> 实验数据显示：
> + 异步日志的实现方式较同步日志的实现方式吞吐量强一个量级（同步40m/s，异步280m/s）
> + 在多线程环境下，同步日志与异步日志的性能都有非常严重的下滑



# 2024/01/02
## 同步日志无序性问题
在之前的设计中考虑到数据写入的有序性以及成员变量的安全性，使用了自旋锁来保护临界区。
但是处于以下两点考量
+ 在实际应用中，日志模块的设置一旦完成，很少有对其成员的改动
+ **TODO:日志模块的日志写入有序性是否有必要由日志模块对其进行约束？**

则对日志模块的锁部分进行优化，前端日志的构造采用无锁的方式进行，以此来增加并发量

``` bash
测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:2.56819s
每秒输出日志数量：389379条
每秒输出日志大小：38025KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量333333 耗时:4.71559s
线程2:  输出数量333333 耗时:4.76937s
线程1:  输出数量333333 耗时:4.85498s
每秒输出日志数量：205973条
每秒输出日志大小：20114KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:0.330609s
每秒输出日志数量：3024724条
每秒输出日志大小：295383KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程2:  输出数量333333 耗时:0.316169s
线程1:  输出数量333333 耗时:0.328701s
线程0:  输出数量333333 耗时:0.334467s
每秒输出日志数量：2989831条
每秒输出日志大小：291975KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程5:  输出数量100000 耗时:0.319204s
线程0:  输出数量100000 耗时:0.341175s
线程4:  输出数量100000 耗时:0.343998s
线程8:  输出数量100000 耗时:0.35173s
线程9:  输出数量100000 耗时:0.353207s
线程2:  输出数量100000 耗时:0.354964s
线程1:  输出数量100000 耗时:0.360241s
线程7:  输出数量100000 耗时:0.365926s
线程6:  输出数量100000 耗时:0.361651s
线程3:  输出数量100000 耗时:0.382031s
每秒输出日志数量：2617591条
每秒输出日志大小：255624KB
```

> 实验结果表明，前端线程的去锁化对异步日志的多线程运行优化明显(90M/s->250M/s)
> 但是实验过程中发现了两点异常
> + 当并发程度非常高时，会存在内存dump的情况
> + 根据对打印出日志的观察，确实**存在假想的乱序现象**，并且存在了**打印丢失问题**
> 
> 针对以上问题，分析如下
> + TODO：在日志message构造到字符串时在某些地方发生了临界区冲突，需要针对每个构造体设置更有细度的锁。或是在写入时发生了安全性问题。
> + TODO：打印丢失的现象需要分析是否是因为写入时出错还是其他问题，并进一步甄别异步日志是否有同样问题（之前时同步日志中发现的问题），并重新审视锁的应用。



# 2024/01/05
## 字符串构造耗时问题（已解决）
根据陈硕老师所言，在字符串构造时，将时间构造成字符串时非常耗时的任务，针对这一问题进行优化探讨

测试打印时间字符串的输出效率
``` bash
测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:3.40098s
每秒输出日志数量：294033条
每秒输出日志大小：28714KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程2:  输出数量333333 耗时:7.20461s
线程1:  输出数量333333 耗时:7.37501s
线程0:  输出数量333333 耗时:7.37811s
每秒输出日志数量：135536条
每秒输出日志大小：13235KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:0.746579s
每秒输出日志数量：1339443条
每秒输出日志大小：130805KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量333333 耗时:1.07706s
线程1:  输出数量333333 耗时:1.36384s
线程2:  输出数量333333 耗时:1.38611s
每秒输出日志数量：721445条
每秒输出日志大小：70453KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程5:  输出数量100000 耗时:1.23487s
线程6:  输出数量100000 耗时:1.32486s
线程9:  输出数量100000 耗时:1.33905s
线程0:  输出数量100000 耗时:1.39812s
线程3:  输出数量100000 耗时:1.40304s
线程4:  输出数量100000 耗时:1.40427s
线程2:  输出数量100000 耗时:1.41166s
线程8:  输出数量100000 耗时:1.41256s
线程7:  输出数量100000 耗时:1.42238s
线程1:  输出数量100000 耗时:1.43051s
每秒输出日志数量：699050条
每秒输出日志大小：68266KB
```
> 实验结果表明，所有模式下的并发输出效率都受到了时间字符串构造的严重的影响
> 
> 这是由于**重复构造字符串**导致，在同一秒内所有时间字符串应当具有相同字符串格式（以秒为最小单位粒度下），优化方案为 **设计时间管理单例**，当需要打印时间字符串时向其申请，如果时间相同则直接使用其存储的字符串缓存；若时间改变，则构造新的字符串缓存。

实验结果如下
``` bash
测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:2.47128s
每秒输出日志数量：404649条
每秒输出日志大小：39516KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程2:  输出数量333333 耗时:4.76527s
线程1:  输出数量333333 耗时:4.86739s
线程0:  输出数量333333 耗时:4.9727s
每秒输出日志数量：201097条
每秒输出日志大小：19638KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:0.415794s
每秒输出日志数量：2405039条
每秒输出日志大小：234867KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量333333 耗时:0.350894s
线程2:  输出数量333333 耗时:0.347898s
线程1:  输出数量333333 耗时:0.360088s
每秒输出日志数量：2777098条
每秒输出日志大小：271201KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程3:  输出数量100000 耗时:0.379318s
线程7:  输出数量100000 耗时:0.431363s
线程0:  输出数量100000 耗时:0.399491s
线程8:  输出数量100000 耗时:0.431745s
线程6:  输出数量100000 耗时:0.447099s
线程2:  输出数量100000 耗时:0.4504s
线程4:  输出数量100000 耗时:0.448399s
线程9:  输出数量100000 耗时:0.461774s
线程1:  输出数量100000 耗时:0.471244s
线程5:  输出数量100000 耗时:0.468347s
每秒输出日志数量：2122041条
每秒输出日志大小：207230KB
```

> 实验结果表明，缓存方案提高了日志输出性能（60m/s->200m/s）

## 无锁导致的内存dump问题（已解决）

在实验过程中也发生了非常频繁的内存dump情况（尤其是在10线程时），则一定是时间管理
单例中没有加锁导致，以上实验结果为已经添加了自旋锁的结果，考虑到每一秒改动一次时间字符串，而并发的读取操作会非常多，则同时进行读写锁的设计实现，结果如下：

```bash
测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:2.5765s
每秒输出日志数量：388123条
每秒输出日志大小：37902KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量333333 耗时:4.92998s
线程2:  输出数量333333 耗时:5.04528s
线程1:  输出数量333333 耗时:5.10949s
每秒输出日志数量：195714条
每秒输出日志大小：19112KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:0.437606s
每秒输出日志数量：2285160条
每秒输出日志大小：223160KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程2:  输出数量333333 耗时:0.382744s
线程0:  输出数量333333 耗时:0.380229s
线程1:  输出数量333333 耗时:0.397285s
每秒输出日志数量：2517084条
每秒输出日志大小：245809KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程6:  输出数量100000 耗时:0.353977s
线程4:  输出数量100000 耗时:0.373159s
线程3:  输出数量100000 耗时:0.38916s
线程0:  输出数量100000 耗时:0.391656s
线程5:  输出数量100000 耗时:0.395675s
线程1:  输出数量100000 耗时:0.399618s
线程8:  输出数量100000 耗时:0.39981s
线程2:  输出数量100000 耗时:0.409114s
线程9:  输出数量100000 耗时:0.396821s
线程7:  输出数量100000 耗时:0.412939s
每秒输出日志数量：2421665条
每秒输出日志大小：236490KB
```

> 实验结果表明，读写锁这种基于线程阻塞的锁在少线程时的并发效果不如自旋锁，但工作线程达到10个时，其性能则更优，这是由于大量的读锁并不需要进入阻塞状态，具有更好的并发性。

# 时间字符串粒度问题（已解决）

针对时间字符串的粒度问题，即陈硕老师在muduo设计中，使用了以微妙粒度的服务器框架，但根据我的实验测试，实现微秒粒度的系统函数开销较大，对并发性是一个非常大的挑战，所以在我的日志框架中不予采用。实现结果如下：

```bash
test on 1000000 iter
Method: system_clock - Time taken: 1235448 microseconds
Method: high_resolution_clock - Time taken: 1247874 microseconds
Method: gettimeofday - Time taken: 1212794 microseconds
Method: localtime - Time taken: 21093 microseconds

Method: system_clock - 1704474525385838
Method: high_resolution_clock - 1704474525385856
Method: gettimeofday - 1704474525385873
Method: localtime - 1704474525

TimeString: 1704474525.385896
TimeFormatStringUs: 2024-01-06 01:08:45.385896

test on 100000 iter
FormatString: 2024-01-06 01:08:47
Direct construction duration: 1644808 microseconds
FormatString: 2024-01-06 01:08:47
TimeManager duration: 4186 microseconds
```

> 如上实验可以看到，chrono库的方法和sys/time的方法都可以得到us粒度，但是运行耗时是localtime_r的60倍。同时在构造字符串的任务测试中，使用了缓存法的localtime_r效率是chrono的400倍。最终，日志库采用**秒为粒度**的方案。