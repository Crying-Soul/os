#!/bin/bash
"""
Функциональность:
1. Запуск основной программы с трассировкой системных вызовов:
   - Мониторинг сигналов (strace -e trace=signal)
   - Подробный вывод (-tt -v -s 1000)
   - Требует root-прав (sudo)

2. Анализ результатов:
   - Вывод содержимого тестовых файлов
   - Автоматическая очистка (удаление тестовых файлов)

3. Особенности:
   - Комплексная проверка всех аспектов наследования
   - Наглядное представление результатов
   - Минимизация ручной работы
"""
sudo strace -tt -v -s 1000 -e trace=signal ./src/6/6 

echo -e "\ntestfile_fork.txt:\n"
cat testfile_fork.txt
echo -e "\ntestfile_exec.txt:\n"
cat testfile_exec.txt
echo -e "\ntestfile_clone:\n"
cat testfile_clone.txt


rm testfile_clone.txt testfile_fork.txt testfile_exec.txt
