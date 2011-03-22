#!/bin/sh

FI1="$(mktemp)"
FI2="$(mktemp)"
FI="$FI1"
FIN="$FI2"

[ $? -ne 0 ] && exit 2

echo "" > "$FI"
# format temp: "hash of watched files|pid|proccess cmd|cmd|list of watched files"
# format config: "cmd|list of watched files"
while read line;
do
	echo "|1||$line" >> "$FI"
done <<!
./te.sh|distfiles
./te.sh|man-interest
./te.sh|distfiles man-interest
!

echo "$FI1:$FI2"
while true
do
	echo "" > "$FIN"
	while read line;
	do
		[ -z "$line" ] && continue
		echo "$line"
		SHSN=""
		SHS=$(echo $line | cut -d'|' -f 1)
		PID=$(echo $line | cut -d'|' -f 2)
		PCMD=$(echo $line | cut -d'|' -f 3)
		CMD=$(echo $line | cut -d'|' -f 4)
		WFS=$(echo $line | cut -d'|' -f 5)
		[ -z "$CMD" ] && continue
		RESULT=$(ps -p ${PID} -o cmd | grep "^${PCMD}$")
		if [ -z "${RESULT}" ];
		then
			eval "$CMD &"
			PID="$!"
			PCMD=$(ps -p "$PID" -o cmd | tail -n+2)
			echo "NEW PID $PID"
		fi
		if [ ! -z "$WFS" ];
		then
			SHSN=""
			for FILE in $(echo $WFS);
			do
				SHSN+=$(stat -c %Y "$FILE")
			done
			if [ "$SHSN" != "$SHS" ];
			then
				[ ! -z "$SHS" ] && kill "$PID"
				SHS="$SHSN"
			fi
		fi
		echo "$SHS|$PID|$PCMD|$CMD|$WFS" >> $FIN
	done <$FI
	if [ "$FI" == "$FI1" ];
	then
		FI="$FI2"
		FIN="$FI1"
	else
		FI="$FI1"
		FIN="$FI2"
	fi
	echo "END"
	sleep 1
done

