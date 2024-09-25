### Задача 1

*Read this in other languages: [English](README.md), [Русский](README.ru.md).*

<b> [task1.py](./task1.py): </b><br>

Требуется написать функцию `hello(x)`, которая принимает 1 параметр, равный по умолчанию None. <br>
Если в качестве этого параметра передается пустая строка или происходит вызов без аргументов, то функция
возвращает строку "Hello!", иначе, функция выводит "Hello, x!", где x - значение параметра функции. <br><br>

Пример работы:
```python
print(hello()) # Hello!
print(hello('')) # Hello!
print(hello('Masha')) # Hello, Masha!
```
<br>

Запуск тестов:
```bash
python run.py unittest task1
```