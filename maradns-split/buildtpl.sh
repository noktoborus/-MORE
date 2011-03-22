#!/bin/sh

CONFIG=${1}
OUTDIR=""
TEMPLATE=${2}
ER="/dev/stderr"

if ( [ -z "${CONFIG}" ] || [ -z "${TEMPLATE}" ] );
then
	printf "${0} configfile template [outdir] -> " >>$ER
	( [ -z "${CONFIG}" ] && printf "configfile not set" >>$ER ) ||
		( [ -z "${TEMPLATE}" ] && printf "template not set" >>$ER )
	printf "\n" >>$ER
	exit 1
fi

if ( [ ! -r "${CONFIG}" ] || [ ! -r "${TEMPLATE}" ] );
then
	[ ! -r "${CONFIG}" ] &&
		printf "${0}: error -> can't read configfile '${CONFIG}'\n" >>$ER
	[ ! -r "${TEMPLATE}" ] &&
		printf "${0}: error -> can't read template '${TEMPLATE}'\n" >>$ER
	exit 1
fi

CONFIG=$(realpath "${1}" 2>/dev/null)
[ ! -z "${3}" ] && OUTDIR="${3}"
[ -z "${OUTDIR}" ] && OUTDIR="."
mkdir -p ${OUTDIR}
if [ ! -d "${OUTDIR}" ];
then
	printf "${0}: error: can't find output '$OUTDIR'" >>$ER
	exit 1
fi

build ()
{
	TFILE="${1}"
	OFILE="${2}"
	NAME="${3}"
	CFG_DIR=$(dirname ${CONFIG})
	( [ -z "${TFILE}" ] || [ -z "${OFILE}" ] ) && exit 1
	[ ! -z "${OUTDIR}" ] && OFILE="${OUTDIR}/${OFILE}"
	mkdir -p $(dirname ${OFILE})
	> ${OFILE}
	while read line;
	do
		line=$(echo $line |
			sed 's/~/\\~/g' |
			sed 's/"/\\"/g' |
			sed "s/'/\\\'/g" )
		eval "echo $line" >> ${OFILE}
	done <${TFILE}
}

call ()
{
	echo " -> Build target: $1 (In: $2, Out: $3)"
	. "${CONFIG}"
	build $2 $3 $1
}

main ()
{
	. "${CONFIG}"
	echo "Output path: ${OUTDIR}"
	echo "Template: ${TEMPLATE}"
	echo "Targets: ${NAMES}"
	for target in ${NAMES};
	do
		call "${target}" "${TEMPLATE}" "${TEMPLATE}.${target}"
	done
}

main

