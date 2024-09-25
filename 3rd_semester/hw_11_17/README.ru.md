## Домашнее задание на 17.11

*Read this in other languages: [English](README.md), [Русский](README.ru.md).*

<b> [task_1](./task_1.c): </b><br>
```cpp
Программа моделирует команду SHELL: 
(здесь pr_i – имена процессов, arg_j- аргументы процессов, f.dat – файл входных данных, f.res – файл результатов).
Аргументы, необходимые этой программе, задаются в командной строке:
    a) pr1 arg1 arg2 | pr2; pr3 >> f.res
    b) pr1 < f.dat | pr2 > f.res; pr3
    c) pr1 | pr2 | ... | pr_n
    d) pr1 arg1 > f.res; pr2 | pr3 | pr4 >> f.res
```

<br> <b> [task_2](./task_2.c): </b><br>
```cpp
Программа подсчитывает поступившие сигналы SIGTRAP между 2-ым и 4-ым нажатием CTRL+C. 
Завершает работу по 7-му нажатию CTRL+C.
```
