#!/bin/bash

cd 3/3.2

echo "Проверим wait для существуюшего фонового процесса"
# Запускаем фоновый процесс
echo hello &
# Ожидаем завершения процесса и сохраняем статус
wait $! #Переменная $! содержит PID последнего запущенного фонового процесса

# Анализируем статус завершения
status=$? #Переменная $? содержит статус завершения последней выполненной команды
if [ $status -eq 0 ]; then
    echo "Процесс завершился успешно."
else
    echo "Отловили процесс завёршенный с ошибкой. Статус: $status"
fi

echo "Проверим wait для несуществующего фонового процесса"
nonexistent_command &
wait $! 

status=$?
if [ $status -eq 0 ]; then
    echo "Процесс завершился успешно."
else
    echo "Отловили процесс завёршенный с ошибкой. Статус: $status"
fi

echo "Создаём сыновей с разными кодами завершения и waitpid ждём их завершения"
files=("son1.c" "son2.c" "son3.c" "son4.c" "son5.c" "father.c")
for file in "${files[@]}"
do
    output="${file%.*}"
    gcc "$file" -o "$output"
done

./father

for file in "${files[@]}"
do
    output="${file%.*}"
    rm $output

done
