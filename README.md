# ТЗ
2 модуля 
Стационарный и мобильный 
Компоненты: 
Стационарный: 
Разъем для подключения к мобильному модулю 
3 тумблера(переключателя) с 3 мя светодиодами на каждый (красный, желтый, зеленый) 
2 регулятора (поворачиваемые ручки как у радио) 
Динамик 
Шаговый двигатель 
Подключение светодиодной ленты 
Мобильный: 
Разъем для подключения к стационарному модулю 
LCD экран 2 на 16 
Клавиатура 3 на 4 
 
Логика работы 
При подаче питания на стационарный модуль загораются 3 красных светодиода, ожидания включения первого тумблера, если при включении подключён мобильный модуль то вместо первого красного загорается жёлтый светодиод, на мобильном модуле на экран выводится надпись инициализация и через некоторое время ввод кода доступа, при этом стационарный модуль передает ключ(4 цифры его отобразить на экране) и нужен парный к нему ответ вводится с клавиатуры на мобильном (4 цифры отображать ввод на экране звездочками) # конец ввода и передача пароля на стационарный, там проверка и при успехе первый зелёный светодиод вместо желтого. 
Ожидание включения второго тумблера 
При включении второго тумблера соответствующий светодиод переходит из красного в желтый, ожидание настройки, на экран передавать уровень текущей настройки, целевая настройка это случайное положение шагового двигателя и нужно его повернуть до него и настроить второй регулятор “частота сигнала” (как на ради волну ловить). Прогресс настройки отображать на экране, допустить небольш погрешность, при стопроцентной настройке светодиод зелёный и ожидать включения второго 3го тумблера. 
При ведении 3го светодиод ереходит в желтый и начинается передача данных, первым передать количество пакетов с данными, и отобразить прогресс передачи на экране мобильного, затем с случайным периодом в заданном диапазоне передавать пакеты с данными, передача сопровождается звуковыми сигналами, после передачи последнего пакета светодиод зелёным, и ожиданте отключения кабеля и первому всех тумблеров в состояние выкл, в мобильном модуле должно быть сохранено сколько из скольки пакетов передано, и возможно вывести информацию эту на экран(обязательно) и компьютер при подключении по USB(не обязательно) 
Ограничить количество попыток ввода пароля, при нарушении последовательности
