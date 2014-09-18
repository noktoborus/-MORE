#!/usr/bin/env python3
# -*- encoding: utf-8 -*
# vim: ft=python ff=unix fenc=utf-8
# file: roulette.py

import random
from math import floor, ceil


class European:
    def __init__(self, app):
        # размеры поля
        self.x = 3
        self.y = 13
        # игровое поле, y+ вниз, x+ влево
        # положительные красные, отрицательные чёрные
        # 0---> x (3)
        # |
        # |
        # |
        # |
        # v y (13)
        #
        self.field = (
            0, 0, 0,
            1, -2, 3,
            -4,   5,  -6,
            7,   -8,   9,
            -10, -11,  12,
            -13,  14, -15,
            16,  -17,  18,
            19,  -20,  21,
            -22,  23, -24,
            25,  -26,  27,
            -28, -29,  30,
            -31,  32, -33,
            34,  -35,  36)
        # список внешний ставок
        self.outside = {
            "red": lambda s, v: [i for i in s.field if i > 0],
            "black": lambda s, v: [abs(i) for i in s.field if i < 0],
            "column": lambda s, v: 0 <= v < 3 and [abs(i) - v for i in s.field if i != 0 and not i % 3] or [],
            "dozen": lambda s, v: 0 <= v < 3 and [abs(i) for i in s.field if 12 * v + 13 > abs(i) > 12 * v],
            "even": lambda s, v: [abs(i) for i in self.field if i != 0 and not i % 2],
            "odd": lambda s, v: [abs(i) for i in self.field if i % 2],
            "low": lambda s, v: list(range(1, 19)),
            "high": lambda s, v: list(range(19, 37))
            }
        # типа игрок и его занятые поля
        self.gamer = []

    def get_nums(self, x, y):
        """ генерация списка для ставок на номера
            (inside bets)
            ищет подходящие номера под позицию:
            1x1y => 1
            2x2y => 5
            значения должны лежать в диапазоне
             0.5 ... x ... self.x + 0.5
             0.5 ... y ... self.y + 0.5
            """
        nums = []
        # выход за границы поля
        if x >= self.x or y >= self.y or x < 0.0 or y < 0.0:
            return []
        # ставка на ноль (straight up(1))
        if y == 0:
            return [0]
        # точная позиция (straight up(1))
        if (int(x), int(y)) == (x, y):
            return [abs(self.field[int(self.x * y + (x % self.x))])]
        # позиции на границе по x (один из [street bet(3), line bet(4)])
        if x > self.x - 1:
            # line bet может быть только в промежутке 1.5 ... 11.5
            # и y должно быть дробным
            if int(y) != y and 1 < y < self.y - 1.0:
                for n in range(int(self.x * floor(y)), int(self.x * ceil(y) + self.x)):
                    nums += [abs(self.field[n])]
                return nums
            # street bet может быть в промежутке 1 ... 12
            if int(y) == y and y >= 1 and y <= self.y - 1.0:
                for n in range(int(self.x * y), int(self.x * y + self.x)):
                    nums += [abs(self.field[n])]
                return nums
            return []
        # для corner(4) x и y должны быть пограничными
        # и в диапазонах 0.5 ... x ... 1.5, 1.5 ... y ... 10.5
        if int(x) != x and int(y) != y and 0 < x < self.x - 1.0\
                and 1.0 < y < self.y - 2.0:
            for n in list(range(
                int(self.x * floor(y) + floor(x)),
                int(self.x * floor(y) + ceil(x)) + 1)) +\
                list(range(
                    int(self.x * ceil(y) + floor(x)),
                    int(self.x * ceil(y) + ceil(x)) + 1)):
                nums += [abs(self.field[n])]
            return nums
        # самое странное, split bet(2)
        # вроде может быть как горизонтальный (x пограничный)
        # так и вертикальный (y пограничный)
        if 0 <= x <= self.x - 0.0 and 0 <= y < self.y - 1.0:
            # горизонтальный сплит
            if (int(x) != x and int(y) == y):
                for n in (self.x * y + floor(x), (self.x * y + ceil(x))):
                    nums += [abs(self.field[int(n)])]
                return nums
            # вертикальный сплит
            if (int(x) == x and int(y) != y):
                for n in (self.x * floor(y) + x, self.x * floor(y + 1) + x):
                    nums += [abs(self.field[int(n)])]
                return nums
        return []

    def get_factor(self, numbers):
        factors = {
            18: 1,
            12: 2,
            6: 5,
            4: 6,
            3: 11,
            2: 17,
            1: 35
            }
        return factors.get(len(numbers), 0)

    def game(self, ams):
        """ приём ставки: возможные форматы params:
                '1,5': ставка на поле
                'red': ставка на красные
                'dozen,1': ставка на столбец 1
                'dozen,0': ставка на столбец 2
        """
        ams = ams.split(',', 1)
        a = ams[0]
        b = None
        if a in self.outside:
            try:
                b = int(ams[1])
            except:
                b = -1
            self.gamer = self.outside[a](self, b)
        elif len(ams) == 2:
            try:
                a = float(ams[0])
                b = float(ams[1])
            except:
                # возвращаем какое-то дефолтное значение, прислали ерунду
                return self.gamer
            self.gamer = self.get_nums(a, b)
        return self.gamer

    def rol(self):
        """ вращаем барабан и возвращаем выпавшее число """
        # пропуск первых двух нулей,
        # хотя никому хуже не должно стать если у нуля повысится шанс
        return abs(self.field[random.Random().randint(2, len(self.field) - 1)])

    def result(self):
        """ непосредственно сам выбор выйгравших
            возвращает:
            (положение шарика, множитель выигрыша)
        """
        luckynum = self.rol()
        if luckynum in self.gamer:
            return (luckynum, self.get_factor(self.gamer))
        else:
            return (luckynum, 0)

if __name__ == "__main__":
    # непонятный кусок ничего
    print("European")
    c = European(None)
    y = 0.0
    while y < c.y:
        x = 0.0
        while x < c.x:
            sa = c.get_nums(x, y)
            fa = c.get_factor(sa)
            print("x: %s, y: %s => {%s} x%s %s" % (x, y, len(sa), fa, sa))
            x += 0.5
        y += 0.5
    for q in 'red', 'black', 'dozen', 'column', 'even', 'odd', 'low', 'high':
        for n in (0, 1, 2):
            sa = c.outside[q](c, n)
            fa = c.get_factor(sa)
            print("%s:%s -> {%s} x%s %s" % (q, n, len(sa), fa, sa))
    for q in range(1, 15):
        print("rol#%s -> %s" % (q, c.rol()))
