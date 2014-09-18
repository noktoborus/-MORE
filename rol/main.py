#!/usr/bin/env python3
# -*- encoding: utf-8 -*
# vim: ft=python ff=unix fenc=utf-8
# file: main.py

import flask
import roulette

app = flask.Flask(__name__)

cfg = {
    "prefix": "/v0"
    }

roulettes = {"european": roulette.European}


@app.route("/")
def root():
    return "no way"

documents = {
    "txt": {
        "404L1": "Неподдерживаемый тип рулетки!",
        "403L1": "Неверная ставка! Проверьте правила игры.",
        "200L1": "Ваша ставка на номера: {0}\nУдачный номер: {1}\nВаш множитель: {2}\n"
        },
    "json": {
        "404L1": "{{'error': 'unknown roulette'}}",
        "403L1": "{{'error': 'invalid bet'}}",
        "200L1": "{{'bet': {0}, 'lucky': {1}, 'factor': {2}}}"
        }
    }


@app.route(cfg["prefix"] + "/<roulette_type>/<bet>.<ext>", methods=["GET"])
def roulette_choose(roulette_type, bet, ext):
    if ext not in ["json", "txt"]:
        return("unknown document type, support types: txt, json\n", 404)
    if roulette_type not in roulettes:
        return(documents[ext]["404L1"], 404)
    else:
        rol = roulettes[roulette_type](app)
        game = rol.game(bet)
        if not game:
            return(documents[ext]["403L1"], 403)
        return(documents[ext]["200L1"].format(*(game,) + rol.result()))

if __name__ == "__main__":
    app.run()
