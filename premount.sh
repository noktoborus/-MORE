#!/bin/sh
# udev rules:
# ACTION=="add", KERNEL=="sd[b-z]*", RUN+="/bin/sh /etc/udev/scripts.d/premount.sh add /dev/%k"
# ACTION=="remove", KERNEL=="sd[b-z]*", RUN+="/bin/sh /etc/udev/scripts.d/premount.sh del /dev/%k"
PATH=/bin:/usr/bin:/sbin:/usr/sbin
POINTS="/mnt/_auto"
BLKID="blkid"

# main ()
ACT="$1"
DEV="$2"
_exit ()
{
	[ $1 -ne 0 ] && logger "$0: exit $1, mode=$ACT, dev=$DEV"
	exit $1
}

if [ "$ACT" == "add" ];
then
	POINT="" # точка монтирования
	OPTS="" # опции для монтирования
	TYPE="" # тип ФС (ntfs, vfat, ext...)
	UUID="" # UUID диска
	LABEL="" # бирка

	DESC="`${BLKID} ${DEV} 2>/dev/null`"
	# blkid вернул ошибку, дальше делать нечего
	[ $? -ne 0 ] && _exit 0
	# уф, еще ложечка плохого кодца в плохой кодец
	# заменяем все пробелы в выводе blkid на \n (т.е. перевод строки), а после сверяем awk на 
	# интересующие ключи
	TYPE="`echo ${DESC} | tr ' ' \\\\n | awk -F'"' '$1 == "TYPE=" {print $2}'`"
	UUID="`echo ${DESC} | tr ' ' \\\\n | awk -F'"' '$1 == "UUID=" {print $2}'`"
	LABEL="`echo ${DESC} | tr ' ' \\\\n | awk -F'"' '$1 == "LABEL=" {print $2}'`"
	# при не определённом TYPE уходим, ибо грусть, печаль
	[ -z "$TYPE" ] && _exit 5
	[ -z "$UUID" ] && UUID="0-0-0-0-0"
	[ -z "$LABEL" ] && LABEL="_________"

	# проверяем наш диск на существование в /etc/fstab
	TEST="`grep ${DEV} /etc/fstab`"
	[ ! -z "$TEST" ] && _exit 61
	TEST="`grep ${UUID} /etc/fstab | grep \UUID=`"
	[ ! -z "$TEST" ] && _exit 62
	TEST="`grep ${LABEL} /etc/fstab | grep \LABEL=`"
	[ ! -z "$TEST" ] && _exit 63
	
	if [ ! -z "$LABEL" ];
	then	
		POINT="$POINTS/`echo $LABEL | tr ' ' _`"
	fi
	## во избежания случайных перекрытий
	while [ -e "$POINT" ];
	do
		POINT+="_"
	done
	
	mkdir -p "$POINT"
	# если директория создастся, то mkdir завершиться с кодом 0
	[ $? -ne 0 ] && _exit 1

	# ничего такого там нет, делаем своё дело и уходим
	# генерируем конфигурацию для конкретных типов файловых систем
	case $TYPE in
		vfat)
			OPTS+="flush,iocharset=utf8,codepage=866,gid=cdrom,uid=0,umask=0002"
			;;
		ntfs)
			[ -e "/sbin/mount.ntfs-3g" ] && TYPE="ntfs-3g"
			OPTS+="sync,gid=cdrom,uid=0,umask=0002"
			;;
		ext*)
			OPTS+="sync"
	esac
	# добавим запятую, что бы user случайно не правератился во что-то вроде usersync
	[ ! -z "$OPTS" ] && OPTS=",$OPTS"
	# на всякий случай, добавляем еще одну строку, хотя их потом наберёться ного
	echo >> /etc/fstab
	#echo "UUID=$UUID $POINT $TYPE users,noauto${OPTS} 0 0 # UDEVMAUTO: $DEV" >> /etc/fstab
	echo "$DEV $POINT $TYPE users,noauto${OPTS} 0 0 # UDEVMAUTO: $DEV" >> /etc/fstab
	## сдесь вроде бы всё
elif [ "$ACT" == "del" ];
then
	FSTAB="" # содержимое /etc/fstab
	POINT="" # точка монтирования
	## теперь нужно извлечь наше устройство из /etc/fstab, попутно отмонтировав его
	INPUT="`grep \"# UDEVMAUTO: $dev\" /etc/fstab`"
	# тут всё нормально, просто уходим
	[ -z "$INPUT" ] && _exit 0
	POINT="`echo $INPUT | awk '{print $2}'`"
	# демонтируем
	## эх, поубивать всё фазером было бы хорошо, но всякие не удобные програмки, типа vlc, gmplayer
	## имеют свойство удерживать за собой путь (в частности, после использования диалогов выбора файлов)
	# fuser -k $POINT
	umount -f $POINT >/dev/null 2>&1
	# и удаляем саму точку, к чему лишний мусор разводить
	rmdir -p $POINT >/dev/null 2>&1
	# и пересоздаём /etc/fstab, не очень практично, при многократных параллельных вызовах скрипта
	# станет ясно почему, но ставить блокировки для себя тоже не очень удобно, т.к. кроме нас 
	# /etc/fstab могут использовать какие-нибудь другие програмки. Уж если система падает, то лучше
	# падать со всеми вместе.
	# вырезаем нашу строчку и, заодно, подчищаем от лишних пустых строк
	NFSTAB="`cat /etc/fstab | grep -v \"# UDEVMAUTO: $dev\" | sed '/^$/d'`"
	# видимо произошла авария при удалении строки
	[ -z "$NFSTAB" ] && _exit 1
	# всё хорошо, вносим поправку в /etc/fstab
	echo "$NFSTAB" > /etc/fstab
else
	_exit 2
fi

