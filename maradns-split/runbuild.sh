#!/bin/sh

O=$(realpath ./output)
B=$(realpath ./buildtpl.sh)
cd "./config"

build ()
{
	SRC=${1}
	${B} ${CONFIG} ${SRC} ${O}
}

CONFIG="config.samplehost.sh"
build db/db.samplehost
build mararc

