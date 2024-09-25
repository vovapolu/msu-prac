### Задача 7

*Read this in other languages: [English](README.md), [Русский](README.ru.md).*

<b> [task7.py](./task7.py): </b><br>

Это задание – конкурс! Его цель заключается в том, чтобы написать как можно более короткое (по символам) решение задачи. <br>
При подсчете пробельные символы будут автоматически удаляться, поэтому игнорировать символы табуляции и пробелы, которые сделают код более читаемым, не нужно! <br>
Дан список вложенности ровно 2. Требуется написать функцию `process(l)`, которая возвращает отсортированные по убыванию уникальные квадраты чисел, содержащихся в этом списке. <br><br>

Пример работы:
```python
print(process([[1, 2], [3], [4], [3, 1]])) # [16, 9, 4, 1]
```
<br>

Решение на 84 символа:
```python
def process(l):
    r = []
    for i in l:
        for j in i:
            if j*j not in r:
                r.append(j*j)
    return sorted(r,reverse=1)
```
<br>

Решение на 60 символов:
```python
def process(l):
    s = {a * a for b in l for a in b}
    return sorted(s,reverse=1)
```
<br>

Решение на 49 символов:
```python
process = lambda l: sorted({a * a for b in l for a in b})[::-1]
```
<br>

Запуск тестов:
```bash
python run.py test task7
```
