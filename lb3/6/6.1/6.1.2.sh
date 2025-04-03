#!/bin/bash

echo "ЗАПУСК simple_pipe:"
echo "Эта программа демонстрирует:"
echo " - Создание pipe"
echo " - Создание дочернего процесса fork()"
echo " - Передачу одного сообщения от родителя к потомку"
echo "Ожидаемый вывод:"
echo "Родительский процесс отправляет сообщение"
echo "Дочерний процесс получает это сообщение"
echo "------------------ НАЧАЛО ВЫВОДА ------------------"
./lb3/6/6.1/simple_pipe
echo "------------------ КОНЕЦ ВЫВОДА -------------------"
echo

echo "ЗАПУСК dual_pipe:"
echo "Эта программа демонстрирует:"
echo " - Создание двух pipe для двустороннего обмена"
echo " - Родитель отправляет сообщение потомку"
echo " - Потомок отвечает на сообщение"
echo "Ожидаемый вывод:"
echo "Родитель отправляет -> потомок получает"
echo "Потомок отправляет ответ -> родитель получает"
echo "------------------ НАЧАЛО ВЫВОДА ------------------"
./lb3/6/6.1/dual_pipe
echo "------------------ КОНЕЦ ВЫВОДА -------------------"
echo

echo "ЗАПУСК pipe_exec:"
echo "Эта программа демонстрирует:"
echo " - Создание pipe"
echo " - Перенаправление вывода с помощью dup2()"
echo " - Запуск программы wc -l для подсчета строк"
echo "Ожидаемый вывод:"
echo "Программа должна вывести число 3 (три строки было записано в pipe)"
echo "------------------ НАЧАЛО ВЫВОДА ------------------"
./lb3/6/6.1/pipe_exec
echo "------------------ КОНЕЦ ВЫВОДА -------------------"
echo

