#!/bin/sh
NAMES="internal external"

case $1 in
	internal)
		PORT="10053"
		HOST_A="192.168.2.5"
		;;
	external)
		PORT="53"
		HOST_A="10.0.0.5"
		;;
esac

