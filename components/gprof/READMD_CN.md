# Gprof

Gprof 主要用于评估软件的运行中的各个部分消耗的时间和占比，这样开发者可以针对时间开销最多的部分作优化。

# 1. 设计和挑战

这个 Gprof 是新开发的，相比 GNU 原始的 Gprof 有以下的新特性：

1. 仅作用于指定的模块，不需要所有模块都启用，这样大大地节约了内存开销
2. 可以统计 OS 的 sleep/block 接口（仅限 Xtensa 架构），基于这个新特性，用户不仅可以统计硬件 I/O 性能，还可以统计 OS 调度的性能，使得评估出来的应用各模块时间开销更加全面和精准

只不过这个 Gprof 还没有完全做好，所以还存在以下的使用限制：

1. 统计数据存储在指定的 Flash 空间，用户需要根据实际情况调整存储地址，宏定义是 components/gprof/io.c 里面的 WR_OFFSET
2. 基于实际的应用，生成的 gprof 统计数据大小不一，如果使用者的代码较大，需要增大存储 buffer 的尺寸，宏定义是 components/gprof/io.c 里面的 WR_BUFFER_MAX
3. 为了统计特定的模块，需要手动把模块添加到链接脚本里面，这个下面在使用方法里面有介绍
4. 不统计链接在 IRAM 里面的函数，即如果应用的某些函数指定链接到 IRAM，这个函数的执行时间不会被统计

# 2. 使用方法

以下主要基于在 ESP32 上面运行 examples/get-started/coremark 来介绍如何使用 Gprof。

# 2.1 修改配置

首先需要添加配置项启用 Gprof，在 `CMakeLists.txt` 里面添加 `target_compile_options(${COMPONENT_LIB} PRIVATE -pg)`，具体可以参考：examples/get-started/coremark/main/CMakeLists.txt。

接着把 coremark 放到`对应芯片`的 sections.ld 指定的空间里面，具体参考components/esp_system/ld/esp32/sections.ld.in：

```
    *libmain.a:(.literal .literal.*)
    _gprof_text_start = ABSOLUTE(.);
    *libmain.a:(.text .text.*)
    _gprof_text_end = ABSOLUTE(.);
```

把里面的 libmain.a 换成目标模块编译出来的 lib 的名字，比如 `libjson.a`。也可以同时统计多个模块的，修改参考如下：

```
    *libmain.a:(.literal .literal.*)
    *libjson.a:(.literal .literal.*)
    _gprof_text_start = ABSOLUTE(.);
    *libmain.a:(.text .text.*)
    *libjson.a:(.text .text.*)
    _gprof_text_end = ABSOLUTE(.);
```
# 2.2 编译运行

完成上面 `修改配置` 之后，和编译普通的 example 一样编译目标工程，下载并运行，运行结束之后会有如下的打印：

```
I(gprof_close_nocancel_nostatus:67): erase from 0x180000 to 0x181000
I(gprof_close_nocancel_nostatus:74): save 3777 data to 0x180000
```
其中 `0x180000` 是存储数据的 Flash 地址，`3777` 是存储数据的长度，运行下面的命令把这段数据从 Flash 读取出来：

```
esptool.py -c esp32 read_flash 0x180000 3777 gmon.out
```

# 2.3 解析数据

在解析数据之前需要修改 ELF 里面 section 的名字，这是因为 GCC 默认只解析 `.text` section，而我们默认使用的存储代码的 section 是 `flash.text`，命令如下：

```
xtensa-esp32-elf-objcopy build/coremark.elf --rename-section .flash.text=.text
```

接着运行命令解析数据：

```
xtensa-esp32-elf-gprof build/coremark.elf -b gmon.out
```

得到以下的 log：

```
Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls   s/call   s/call  name    
 37.92     12.78    12.78      500     0.03     0.03  test_sleep
 19.41     19.33     6.54  4208640     0.00     0.00  core_state_transition
  8.68     22.25     2.93   846660     0.00     0.00  core_list_find
  6.16     24.33     2.08  1200124     0.00     0.00  crcu16
  5.10     26.05     1.72     8220     0.00     0.00  core_bench_list
  3.65     27.28     1.23    16440     0.00     0.00  matrix_mul_matrix_bitextract
  3.29     28.39     1.11    16440     0.00     0.00  matrix_test
  2.76     29.32     0.93    16440     0.00     0.00  matrix_mul_matrix
  2.61     30.20     0.88    16440     0.00     0.00  core_bench_state
  2.45     31.03     0.83   855259     0.00     0.00  cmp_idx
  2.06     31.72     0.70   912936     0.00     0.00  calc_func
  1.60     32.26     0.54   456468     0.00     0.00  cmp_complex
  1.57     32.79     0.53    12331     0.00     0.00  core_list_mergesort
  1.37     33.25     0.46   550744     0.00     0.00  crc16
  0.83     33.53     0.28   263040     0.00     0.00  crcu32
  0.30     33.63     0.10    16440     0.00     0.00  matrix_mul_vect
  0.21     33.70     0.07        1     0.07     0.07  core_init_state
  0.04     33.72     0.02        1     0.02     0.02  portable_fini
  0.01     33.72     0.01        6     0.00     0.00  get_seed_args
  0.00     33.72     0.00    16440     0.00     0.00  core_bench_matrix
  0.00     33.72     0.00        8     0.00     0.00  time_in_secs
  0.00     33.72     0.00        4     0.00     0.00  get_time
  0.00     33.72     0.00        4     0.00     5.21  iterate
  0.00     33.72     0.00        4     0.00     0.00  start_time
  0.00     33.72     0.00        4     0.00     0.00  stop_time
  0.00     33.72     0.00        1     0.00     0.00  check_data_types
  0.00     33.72     0.00        1     0.00     0.00  core_init_matrix
  0.00     33.72     0.00        1     0.00     0.00  core_list_init
  0.00     33.72     0.00        1     0.00    20.94  main
  0.00     33.72     0.00        1     0.00     0.00  portable_free
  0.00     33.72     0.00        1     0.00     0.00  portable_init
  0.00     33.72     0.00        1     0.00     0.00  portable_malloc


                        Call graph


granularity: each sample hit covers 2 byte(s) for 0.03% of 33.72 seconds

index % time    self  children    called     name
                                                 <spontaneous>
[1]    100.0    0.00   33.72                 app_main [1]
                0.00   20.94       1/1           main [2]
               12.78    0.00     500/500         test_sleep [8]
-----------------------------------------------
                0.00   20.94       1/1           app_main [1]
[2]     62.1    0.00   20.94       1         main [2]
                0.00   20.85       4/4           iterate [3]
                0.07    0.00       1/1           core_init_state [21]
                0.02    0.00       1/1           portable_fini [22]
                0.01    0.00       6/6           get_seed_args [23]
                0.00    0.00       1/1           core_list_init [24]
                0.00    0.00       4/550744      crc16 [15]
                0.00    0.00       8/8           time_in_secs [25]
                0.00    0.00       4/4           start_time [27]
                0.00    0.00       4/4           get_time [26]
                0.00    0.00       4/4           stop_time [28]
                0.00    0.00       1/1           portable_init [32]
                0.00    0.00       1/1           core_init_matrix [30]
                0.00    0.00       1/1           check_data_types [29]
                0.00    0.00       1/1           portable_malloc [33]
                0.00    0.00       1/1           portable_free [31]
-----------------------------------------------
                0.00   20.85       4/4           main [2]
[3]     61.8    0.00   20.85       4         iterate [3]
                1.72   19.11    8220/8220        core_bench_list [4]
                0.01    0.00    8220/1200124     crcu16 [14]
-----------------------------------------------
                1.72   19.11    8220/8220        iterate [3]
[4]     61.8    1.72   19.11    8220         core_bench_list [4]
                0.53   14.45   12330/12331       core_list_mergesort [5]
                2.93    0.00  846660/846660      core_list_find [13]
                0.39    0.81  468540/550744      crc16 [15]
-----------------------------------------------
                0.00    0.00       1/12331       core_list_init [24]
                0.53   14.45   12330/12331       core_bench_list [4]
[5]     44.4    0.53   14.45   12331         core_list_mergesort [5]
                0.54   13.09  456468/456468      cmp_complex [6]
                0.83    0.00  855259/855259      cmp_idx [19]
-----------------------------------------------
                0.54   13.09  456468/456468      core_list_mergesort [5]
[6]     40.4    0.54   13.09  456468         cmp_complex [6]
                0.70   12.39  912936/912936      calc_func [7]
-----------------------------------------------
                0.70   12.39  912936/912936      cmp_complex [6]
[7]     38.8    0.70   12.39  912936         calc_func [7]
                0.88    7.73   16440/16440       core_bench_state [9]
                0.00    3.58   16440/16440       core_bench_matrix [11]
                0.20    0.00  115080/1200124     crcu16 [14]
-----------------------------------------------
               12.78    0.00     500/500         app_main [1]
[8]     37.9   12.78    0.00     500         test_sleep [8]
-----------------------------------------------
                0.88    7.73   16440/16440       calc_func [7]
[9]     25.5    0.88    7.73   16440         core_bench_state [9]
                6.54    0.00 4208640/4208640     core_state_transition [10]
                0.28    0.91  263040/263040      crcu32 [17]
-----------------------------------------------
                6.54    0.00 4208640/4208640     core_bench_state [9]
[10]    19.4    6.54    0.00 4208640         core_state_transition [10]
-----------------------------------------------
                0.00    3.58   16440/16440       calc_func [7]
[11]    10.6    0.00    3.58   16440         core_bench_matrix [11]
                1.11    2.43   16440/16440       matrix_test [12]
                0.01    0.03   16440/550744      crc16 [15]
-----------------------------------------------
                1.11    2.43   16440/16440       core_bench_matrix [11]
[12]    10.5    1.11    2.43   16440         matrix_test [12]
                1.23    0.00   16440/16440       matrix_mul_matrix_bitextract [16]
                0.93    0.00   16440/16440       matrix_mul_matrix [18]
                0.05    0.11   65760/550744      crc16 [15]
                0.10    0.00   16440/16440       matrix_mul_vect [20]
-----------------------------------------------
                2.93    0.00  846660/846660      core_bench_list [4]
[13]     8.7    2.93    0.00  846660         core_list_find [13]
-----------------------------------------------
                0.01    0.00    8220/1200124     iterate [3]
                0.20    0.00  115080/1200124     calc_func [7]
                0.91    0.00  526080/1200124     crcu32 [17]
                0.95    0.00  550744/1200124     crc16 [15]
[14]     6.2    2.08    0.00 1200124         crcu16 [14]
-----------------------------------------------
                0.00    0.00       4/550744      main [2]
                0.01    0.03   16440/550744      core_bench_matrix [11]
                0.05    0.11   65760/550744      matrix_test [12]
                0.39    0.81  468540/550744      core_bench_list [4]
[15]     4.2    0.46    0.95  550744         crc16 [15]
                0.95    0.00  550744/1200124     crcu16 [14]
-----------------------------------------------
                1.23    0.00   16440/16440       matrix_test [12]
[16]     3.6    1.23    0.00   16440         matrix_mul_matrix_bitextract [16]
-----------------------------------------------
                0.28    0.91  263040/263040      core_bench_state [9]
[17]     3.5    0.28    0.91  263040         crcu32 [17]
                0.91    0.00  526080/1200124     crcu16 [14]
-----------------------------------------------
                0.93    0.00   16440/16440       matrix_test [12]
[18]     2.8    0.93    0.00   16440         matrix_mul_matrix [18]
-----------------------------------------------
                0.83    0.00  855259/855259      core_list_mergesort [5]
[19]     2.4    0.83    0.00  855259         cmp_idx [19]
-----------------------------------------------
                0.10    0.00   16440/16440       matrix_test [12]
[20]     0.3    0.10    0.00   16440         matrix_mul_vect [20]
-----------------------------------------------
                0.07    0.00       1/1           main [2]
[21]     0.2    0.07    0.00       1         core_init_state [21]
-----------------------------------------------
                0.02    0.00       1/1           main [2]
[22]     0.0    0.02    0.00       1         portable_fini [22]
-----------------------------------------------
                0.01    0.00       6/6           main [2]
[23]     0.0    0.01    0.00       6         get_seed_args [23]
-----------------------------------------------
                0.00    0.00       1/1           main [2]
[24]     0.0    0.00    0.00       1         core_list_init [24]
                0.00    0.00       1/12331       core_list_mergesort [5]
-----------------------------------------------
                0.00    0.00       8/8           main [2]
[25]     0.0    0.00    0.00       8         time_in_secs [25]
-----------------------------------------------
                0.00    0.00       4/4           main [2]
[26]     0.0    0.00    0.00       4         get_time [26]
-----------------------------------------------
                0.00    0.00       4/4           main [2]
[27]     0.0    0.00    0.00       4         start_time [27]
-----------------------------------------------
                0.00    0.00       4/4           main [2]
[28]     0.0    0.00    0.00       4         stop_time [28]
-----------------------------------------------
                0.00    0.00       1/1           main [2]
[29]     0.0    0.00    0.00       1         check_data_types [29]
-----------------------------------------------
                0.00    0.00       1/1           main [2]
[30]     0.0    0.00    0.00       1         core_init_matrix [30]
-----------------------------------------------
                0.00    0.00       1/1           main [2]
[31]     0.0    0.00    0.00       1         portable_free [31]
-----------------------------------------------
                0.00    0.00       1/1           main [2]
[32]     0.0    0.00    0.00       1         portable_init [32]
-----------------------------------------------
                0.00    0.00       1/1           main [2]
[33]     0.0    0.00    0.00       1         portable_malloc [33]
-----------------------------------------------


Index by function name

   [7] calc_func               [5] core_list_mergesort    [20] matrix_mul_vect
  [29] check_data_types       [10] core_state_transition  [12] matrix_test
   [6] cmp_complex            [15] crc16                  [22] portable_fini
  [19] cmp_idx                [14] crcu16                 [31] portable_free
   [4] core_bench_list        [17] crcu32                 [32] portable_init
  [11] core_bench_matrix      [23] get_seed_args          [33] portable_malloc
   [9] core_bench_state       [26] get_time               [27] start_time
  [30] core_init_matrix        [3] iterate                [28] stop_time
  [21] core_init_state         [2] main                    [8] test_sleep
  [13] core_list_find         [18] matrix_mul_matrix      [25] time_in_secs
  [24] core_list_init         [16] matrix_mul_matrix_bitextract
```

以上的数据看起来比较直观，所以这里就不做详细解释了。

# 2.3.1 图形化解析数据

图形化分析更加直观一点，首先需要安装以下的组件：

```
sudo apt install graphviz
pip3 install gprof2dot
```

接着运行：

```
xtensa-esp32-elf-gprof build/coremark.elf -b gmon.out | gprof2dot -n0 -e0  |dot -Tpng -o output.png
```

生成图片 `output.png`。
