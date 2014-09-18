#!/bin/sh
# vim: ft=sh ff=unix fenc=utf-8
# file: client.sh

host=localhost
port=5000
path=/v0/european

if [ -z "$1" ];
then
	cat <<EOF
 $0 [<x> <y>|<outside> <val>]

Пример:
 $0 red
 $0 column 1
 $0 2.0 10

 * Ставки на поле (inside bets)
 координаты указываются от верхнего левого угла поля (field.png)
 примеры:
  x = 2.0, y = 2.0 -> 5 (ставка на число)
  x = 2.5, y = 2.0 -> 4, 5, 6 (street bet)
  x = 0.5, y = 1.5 -> 1, 2, 3, 4 (corner)
  x = 2.5, y = 3.5 -> 7, 8, 9, 10, 11, 12 (line bet)
  x = 0.5, y = 6.0 -> 16, 17 (split bet)

 * Внешние ставки (outside bets)
 используются ключевые слова и индекс
 outside | возможные значения
 red     | 0        ставка на красные
 black   | 0        ставка на чёрные
 column  | 0,1,2    ставка на колонку (0 первая колонка, 1 вторая колонка, ...)
 dozen   | 0,1,2    ставка на дюжину (0 первая дюжина, 1 вторая дюжина, ...)
 even    | 0        ставка на чётные
 odd     | 0        ставка на нечётные
 low     | 0        ставка на малые (<= 18)
 high    | 0        ставка на большие (>= 19)

EOF

exit 1
fi

curl http://$host:$port$path/${1},${2}.txt

