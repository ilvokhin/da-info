# Инструментирование и профилирование

## Инструментирование

### Valgrind
Valgrind [1] --- фреймворк для динамического бинарного инструментирования.
Он состоит из ядра (core) и утилит на основе этого ядра. Примеры утилит:
memcheck, cachegrind, callgrind, massif. Утилита, которая запускается по
умолчанию --- memcheck, остальное можно запустить с помощью аргумента
командной строки --tool.

Пользоваться им очень просто. Никакой дополнительной подготовки не требуется. Например:

```
[d@gate 03_instrumentation_and_profiling]$ cat out_of_bound_access.cpp 
#include <iostream>
#include <cassert>
#include <vector>

using namespace std;

int main()
{
    int n;
    cin >> n;

    assert(n > 1);

    vector<int> v;
    v.reserve(n);

    cout << v[0] << endl;

    return 0;
}
[d@gate 03_instrumentation_and_profiling]$ c++11 -pedantic -Wall -Werror out_of_bound_access.cpp -o out_of_bound
[d@gate 03_instrumentation_and_profiling]$ valgrind ./out_of_bound
==4469== Memcheck, a memory error detector
==4469== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==4469== Using Valgrind-3.13.0 and LibVEX; rerun with -h for copyright info
==4469== Command: ./out_of_bound
==4469==
2
==4469== Conditional jump or move depends on uninitialised value(s)
==4469==    at 0x4EC045E: std::ostreambuf_iterator<char, std::char_traits<char> > std::num_put<char, std::ostreambuf_iterator<char, std::char_traits<char> > >::_M_insert_int<long>(std::ostreambuf_iterator<char, std::char_traits<char> >, std::ios_base&, char, long) const (in /usr/lib64/libstdc++.so.6.0.19)
==4469==    by 0x4EC0A3C: std::num_put<char, std::ostreambuf_iterator<char, std::char_traits<char> > >::do_put(std::ostreambuf_iterator<char, std::char_traits<char> >, std::ios_base&, char, long) const (in /usr/lib64/libstdc++.so.6.0.19)
==4469==    by 0x4ECCDAD: std::ostream& std::ostream::_M_insert<long>(long) (in /usr/lib64/libstdc++.so.6.0.19)
==4469==    by 0x400C67: main (in /home/d/stuff/da-info/examples/03_instrumentation_and_profiling/out_of_bound)
==4469==
...
```

Если необходимо узнать точную строчку в программе, которая вызывает проблему, то нужно
сохранить отладочную информацию при компиляции (опция -g$LEVEL).

К сожалению, замедление программы запущенной под valgrind может составить около
10-50 раз, в зависимости от программы.

### Address Sanitizer
Address sanitizer --- детектор ошибок работы с памятью для языков программирования
C и C++. Список ошибок, которые он способен обнаруживать доступен на странице [2].
В отличие от valgrind'а инструмент использует другую идею --- инструментирование
на этапе компиляции. Address sanitizer состоит из модуля инструментирования в
компиляторе и библиотеки времени выполнения.

Address sanitizer является частью LLVM с версии 3.1 и частью GCC с версии 4.8.
Чтобы использовать address sanitizer необходимо скомпилировать программу с
опцией -fsanitize=address:

```
space:tmp d$ c++ --version
Apple clang version 11.0.0 (clang-1100.0.20.17)
Target: x86_64-apple-darwin18.7.0
Thread model: posix
InstalledDir: /Library/Developer/CommandLineTools/usr/bin
space:tmp d$ c++ -std=c++11 -pedantic -Wall -Wextra -fsanitize=address out_of_bound_access_tricky.cpp
echo '5 1 2 3 4 5' | ./a.out 
1
2
3
4
=================================================================
==25575==ERROR: AddressSanitizer: container-overflow on address 0x6030000002f0 at pc 0x00010a9c954d bp 0x7ffee5237970 sp 0x7ffee5237968
READ of size 4 at 0x6030000002f0 thread T0
    #0 0x10a9c954c in main (a.out:x86_64+0x10000154c)
    #1 0x7fff7b3283d4 in start (libdyld.dylib:x86_64+0x163d4)
...
```

### Debug mode libstdc++
Не каждая ошибка может быть обнаружена с помощью рассмотренных выше утилит.
Необходимо знать о принципе их работы и ограничениях, которые накладываются на
типы ошибок, которые могут быть найдены. Иногда утилитам не хватает информации
о семантике кода, например, в GCC 4.8.5 еще не был реализован Address Sanitizer
Container Overflow [3] и поэтому ASan не способен найти ошибку подобную
out_of_bound_access_tricky.cpp. В этом случае можно воспользоваться
libstdc++ debug mode [4], который включается в GCC с помощью флага -D_GLIBCXX_DEBUG.

```
[d@gate 03_instrumentation_and_profiling]$ c++ --version
c++ (GCC) 4.8.5 20150623 (Red Hat 4.8.5-36)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE
[d@gate 03_instrumentation_and_profiling]$ c++11 -pedantic -Wall -Werror out_of_bound_access_tricky.cpp -o out_of_bound -g3
[d@gate 03_instrumentation_and_profiling]$ valgrind ./out_of_bound
==5810== Memcheck, a memory error detector
==5810== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==5810== Using Valgrind-3.13.0 and LibVEX; rerun with -h for copyright info
==5810== Command: ./out_of_bound
==5810== 
5 1 2 3 4 5
1
2
3
4
5
==5810== 
==5810== HEAP SUMMARY:
==5810==     in use at exit: 0 bytes in 0 blocks
==5810==   total heap usage: 1 allocs, 1 frees, 20 bytes allocated
==5810== 
==5810== All heap blocks were freed -- no leaks are possible
==5810== 
==5810== For counts of detected and suppressed errors, rerun with: -v
==5810== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
[d@gate 03_instrumentation_and_profiling]$ c++11 -pedantic -Wall -Werror out_of_bound_access_tricky.cpp -o out_of_bound -g3 -fsanitize=address
[d@gate 03_instrumentation_and_profiling]$ echo '5 1 2 3 4 5' | ./out_of_bound
1
2
3
4
5
c++11 -pedantic -Wall -Werror out_of_bound_access_tricky.cpp -o out_of_bound -g3 -D_GLIBCXX_DEBUG
[d@gate 03_instrumentation_and_profiling]$ echo '5 1 2 3 4 5' | ./out_of_bound
1
2
3
4
/usr/include/c++/4.8.2/debug/vector:346:error: attempt to subscript 
    container with out-of-bounds index 4, but container only holds 4     
    elements.

Objects involved in the operation:
sequence "this" @ 0x0x7ffced768250 {
  type = NSt7__debug6vectorIiSaIiEEE;
}
Aborted
```

## Профилирование

### Gprof
Gprof [5] --- инструмент анализа производительности для приложений Unix.
Он использует гибридную схему: инструментирование и сэмплирование.
Для генерации кода профилирования утилитой gprof при компиляции (и линковке)
нужно указать флаг -pg.

```
[d@gate tmp]$ c++ -std=c++11 -pedantic -Wall -Werror -pg treap.cpp -o treap
[d@gate tmp]$ head -n 100000 completely_random_test.txt | ./treap > /dev/null
[d@gate tmp]$ gprof treap gmon.out | less
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total           
 time   seconds   seconds    calls  us/call  us/call  name    
 19.47      0.29     0.29  4637529     0.06     0.06  __gnu_cxx::__atomic_add_single(int*, int)
  7.39      0.40     0.11   100000     1.10     8.68  treap<std::string, unsigned long long>::insert_impl(std::shared_ptr<treap<std::string, unsigned long long>::node>&, std::shared_ptr<treap<std::string, unsigned long long>::node>&)
  7.05      0.51     0.11 25034256     0.00     0.00  std::__shared_ptr<treap<std::string, unsigned long long>::node, (__gnu_cxx::_Lock_policy)2>::operator->() const
...
```

### Perf
Perf [6] --- инструмент для анализа производительности в ОС Linux. Нас в первую
очередь будут интересовать подкоманды, позволяющие работать с сэмплами данных:
* perf record --- сохранить отсэмплированные данные,
* perf report --- проанализировать файл, сгенерированный с помощью команды perf record.

```
[d@gate tmp]$ c++ -std=c++11 -pedantic -Wall -Werror -O2 -fno-omit-frame-pointer treap.cpp -o treap
[d@gate tmp]$ head -n 100000 completely_random_test.txt | perf record ./treap > /dev/null
[ perf record: Woken up 1 times to write data ]
[ perf record: Captured and wrote 0.030 MB perf.data (673 samples) ]
[d@gate tmp]$ perf report --stdio
# To display the perf.data header info, please use --header/--header-only options.
#
#
# Total Lost Samples: 0
#
# Samples: 673  of event 'cycles:u'
# Event count (approx.): 1207442482
#
# Overhead  Command  Shared Object        Symbol                                                                                                                                                                                                                           
# ........  .......  ...................  .................................................................................................................................................................................................................................
#
    41.21%  treap    treap                [.] insert
    19.06%  treap    libc-2.17.so         [.] __memcmp_sse4_1
    11.91%  treap    treap                [.] treap<std::string, unsigned long long>::insert_impl
     4.17%  treap    treap                [.] std::_Sp_counted_ptr_inplace<treap<std::string, unsigned long long>::node, std::allocator<treap<std::string, unsigned long long>::node>, (__gnu_cxx::_Lock_policy)2>::_M_dispose
     2.98%  treap    libc-2.17.so         [.] tolower
     2.71%  treap    libstdc++.so.6.0.19  [.] std::operator>><char, std::char_traits<char>, std::allocator<char> >
     1.98%  treap    libstdc++.so.6.0.19  [.] std::istream::sentry::sentry
     1.55%  treap    libc-2.17.so         [.] _int_free
...
```
Если необходимо собрать полные стеки вызовов, например, для понимания откуда
вызывается интересная функция, используйте флаг -g.

---
[1] http://valgrind.org

[2] https://github.com/google/sanitizers/wiki/AddressSanitizer

[3] https://github.com/google/sanitizers/wiki/AddressSanitizerContainerOverflow

[4] https://gcc.gnu.org/onlinedocs/libstdc++/manual/debug_mode_using.html#debug_mode.using.mode

[5] https://sourceware.org/binutils/docs/gprof

[6] https://perf.wiki.kernel.org/index.php/Main_Page
