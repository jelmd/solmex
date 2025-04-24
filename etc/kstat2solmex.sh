#!/bin/ksh93

typeset -r VERSION='1.0' FPROG=${.sh.file} PROG=${FPROG##*/} SDIR=${FPROG%/*}

function showUsage {
	[[ -n $1 ]] && X='-?' ||  X='--man'
	getopts -a ${PROG} "${ print ${USAGE} ; }" OPT $X
}

function doNet {
	typeset IGNORE=1
	typeset -u PREFIX='SOLMEX_NET' IPREFIX="${PREFIX#*_}_IDX"
	typeset -l MPREFIX='solmex_node_net'
	typeset	A B C D E M SFX METRIC UNIT MOD MODLINK MODNIC
	typeset -A SNAME

	while read A B C D E; do
		if [[ $A == 'module:' ]]; then
			if [[ $B == 'link' ]]; then
				M='LINK'
				[[ -n ${MOD} ]] && MODNIC="${MOD}" MOD=
				read -A E
				[[ $E != 'name:' ]] && break
				IGNORE=0
			else
				read -A E
				[[ ${E[1]} != 'link' ]] && IGNORE=1 && continue
				[[ -n ${MOD} ]] && MODLINK="${MOD}" MOD=
				IGNORE=0
			fi
			continue
		fi
		(( IGNORE )) && continue
		[[ -z $A ]] && IGNORE=1 && continue
		[[ $A == 'crtime' || $A == 'snaptime' ]] && continue
		[[ $M == 'LINK' && $A =~ ^[ior](bytes|packets)$ ]] && continue
		METRIC="${A}" UNIT='pkts' SFX=
		if [[ ${A: -2:2} == '64' ]]; then
			METRIC="${A%64}" SFX='64'
		fi
		if [[ ${METRIC: -5:5} == 'bytes' ]]; then
			METRIC="${METRIC%bytes}"
			UNIT='bytes'
		elif [[ ${METRIC: -4:4} == 'pkts' ]]; then
			METRIC="${METRIC%pkts}"
		elif [[ ${METRIC: -7:7} == 'packets' ]]; then
			METRIC="${METRIC%packets}"
		fi
		if [[ ${METRIC} == 'txerrors' ]]; then
			# mms_txerrors (mac misc stats): vid_check, tag needed errors
			METRIC='tx_misc_error'
		elif [[ ${METRIC:1} == 'errors' ]]; then
			# {i,o}errors = mib2errors
			[[ ${METRIC:0:1} == 'o' ]] && METRIC='t' || METRIC='r'
			METRIC+='x_error'
		elif [[ ${METRIC: -7:7} == 'spoofed' || ${METRIC} =~ ^(dhcpdropped|restricted)$ ]]; then
			# link protection stats:
			# dhcpdropped dhcpspoofed ipspoofed macspoofed restricted
			METRIC="sec_${METRIC}"
		elif [[ ${METRIC} == 'r' || ${METRIC} == 'i' ]]; then
			METRIC='rx'
		elif [[ ${METRIC} == 'o' ]]; then
			METRIC='tx'
		elif [[ ${METRIC:0:4} == 'poll' ]]; then
			# mrs_pollbytes, mrs_pollcnt
			[[ ${METRIC} == 'polls' ]] && UNIT=''
			METRIC='rx_polls'
		elif [[ ${METRIC:0:4} == 'bcst' || ${METRIC:0:6} == 'brdcst' ]]; then
			[[ ${METRIC:4:3} == 'xmt' || ${METRIC:6:3} == 'xmt' ]] && \
				METRIC='t' || METRIC='r'
			METRIC+='x_broadcast'
		elif [[ ${METRIC:0:5} == 'multi' ]]; then
			[[ ${METRIC:5:3} == 'xmt' ]] && METRIC='t' || METRIC='r'
			METRIC+='x_multicast'
		elif [[ ${METRIC:0:2} == 'rx' || ${METRIC:0:2} == 'tx' ]]; then
			METRIC="${METRIC:0:2}_${METRIC:2}"
		elif [[ ${METRIC:1:4} == 'drop' ]]; then
			[[ ${METRIC:0:1} == 'o' ]] && METRIC='t' || METRIC='r'
			METRIC+='x_drop'
		elif [[ ${METRIC:4:4} == 'drop' ]]; then
			[[ ${METRIC:3:1} == 'o' ]] && METRIC='t' || METRIC='r'
			METRIC+='x_drop'
		elif [[ ${METRIC} == 'blockcnt' ]]; then
			METRIC='tx_block'		# mts: times blocked for Tx descs
		elif [[ ${METRIC} == 'unblockcnt' ]]; then
			METRIC='tx_unblock'	# mts: unblock calls from driver
			UNIT='calls'
		elif [[ ${METRIC:0:3} == 'lro' ]]; then
			METRIC="lro_${METRIC:3}"
		elif [[ ${METRIC:0:4} == 'intr' ]]; then
			# mrs_intrbytes, mrs_intrcnt
			[[ ${METRIC} == 'intrs' ]] && UNIT=''
			METRIC='rx_intrs'
		elif [[ ${METRIC} == 'ifspeed' ]]; then
			UNIT='bps'
		elif [[ ${METRIC} == 'unknowns' ]]; then
			METRIC='rx_unknown'
		elif [[ ${METRIC} == 'collisions' ]]; then
			METRIC='tx_colls' UNIT=
		elif [[ ${METRIC: -6:6} == '_state' ]]; then
			UNIT=
		elif [[ ${METRIC:0:2} == 'no' ]]; then
			[[  ${METRIC:2:3} == 'xmt' ]] && METRIC='t' || METRIC='r'
			METRIC+='x_nobuf'
		elif [[ ${METRIC} == 'zonename' || ${METRIC} == 'link_duplex' ]]; then
			METRIC=
		fi
		[[ -z ${METRIC} ]] && continue
		[[ -z ${UNIT} ]] || METRIC+='_'
		METRIC+="${UNIT}${SFX}"
		(( VERB )) && print -u2 -f "%16s\t%s\n" "$A" "${METRIC%64}"
		MOD+=" ${METRIC}"
		SNAME["${METRIC}"]="$A"	
	done<"$1"
	(( VERB )) && print -u2
	[[ $M == 'LINK' ]] && MODLINK="${MOD}" || MODNIC="${MOD}"
	printIdx SNAME "${PREFIX}" "${MPREFIX}" "${IPREFIX}"
	printAll MODLINK alnkstats ${IPREFIX}
	printAll MODNIC  anicstats ${IPREFIX}
}

function printAll {
	typeset -n A=$1
	typeset -u U
	integer I=0
	typeset IPREFIX="$3" V="$2" N S
	typeset -l ITYPE="${IPREFIX}"

	(( ${#A[@]} == 0 )) && return

	S="\nstatic ${ITYPE}_t $V[] = {\n"
	set -s ${A[@]}
	for N ; do
		U="${IPREFIX}_${N}"
		S+="\t$U,"
		(( I++ ))
		(( I == 3 )) && S+='\n' && I=0
	done
	(( I )) && S+='\n'
	print "$S\};\nstatic uint32_t ${V}_sz = ARRAY_SIZE($V);"
}

function printIdx {
	typeset -n A=$1
	typeset -u U
	typeset PREFIX="$2" MPREFIX="$3" IPREFIX="$4" T=${PREFIX%%_*}
	integer I=0
	typeset -l ITYPE="${IPREFIX}"
	typeset H= \
		S='/* solmex metric names */\nstatic const char *snames[] = {\n' \
		M='/* kstat names */\nstatic const char *knames[] = {\n' \
		D='/* metric HELP text */\nstatic const char *sdesc[] = {\n' \
		E="/* index into snames/knames/sdesc */\ntypedef enum ${ITYPE} {\n"

	S+='#define STRINGIFY(x) #x\n#define _S(x) STRINGIFY(x)\n'
	(( PLEN++ ))
	set -s ${!A[@]}
	for N ; do
		U="${N}"
		E+="\t${IPREFIX}_${U},"
		S+="\t_S(${PREFIX}_${U}_N),"
		M+="\t\"${A[$N]}\","
		D+="\t${PREFIX}_${U}_D,"
		H+="#define ${PREFIX}_${U}_D \"\"\n"
		H+="#define ${PREFIX}_${U}_T \"counter\"\n"
		H+="#define ${PREFIX}_${U}_N ${MPREFIX}_${N%64}\n\n"
		(( I++ ))
		(( I == 3 )) && E+='\n' && S+='\n' && D+='\n' && M+='\n' && I=0	
	done
	E+="\t${IPREFIX}_MAX\n} ${ITYPE}_t;\n\n"
	S+='\tNULL\n};\n'
	D+='\tNULL\n};\n'
	M+='\tNULL\n};\n'
	print "$H\n\n$E$M\n$S\n$D"
}

function doMain {
	[[ -z $1 || ! -s $1 ]] && { showUsage; return 1; }
	[[ ${TARGET} == 'net' ]] && doNet "$1"
}

unset VERB TARGET; integer VERB=0
USAGE="[-?${VERSION}"' ]
[-copyright?Copyright (c) 2025 Jens Elkner. All rights reserved.]
[-license?CDDL 1.0]
[+NAME?'"${PROG}"' - Simple kstat output to solmex C defs converter.]
[+DESCRIPTION?Converts the output of \bkstat \b\alinkname\a\b::link\b and \bkstat link::\b\alinkname\a in this order to solmex defines and C helpers.]
[h:help?Print this help and exit.]
[F:functions?Print a list of all functions available.]
[T:trace]:[functionList?A comma separated list of functions of this script to trace (convinience for troubleshooting).] 
[+?]
[t:target]:[name?Generate for the \aname\ad metrics class. Currently supported: \bnet\b, \bzfs\b.]
[v:verbose?Print kstat data names to metric name mapping as discovered on stderr.]
\n\n
\akstat_output\a
'

X="${ print ${USAGE} ; }"
while getopts "${X}" OPT ; do
	case ${OPT} in
		h) showUsage ; exit 0 ;;
		T)	if [[ ${OPTARG} == 'ALL' ]]; then
				typeset -ft ${ typeset +f ; }
			else
				typeset -ft ${OPTARG//,/ }
			fi
			;;
		F) typeset +f && exit 0 ;;
		v) VERB=1 ;;
		t) TARGET="${OPTARG}" ;;
		*) showUsage 1 ; exit 1 ;;
	esac
done

X=$((OPTIND-1))
shift $X && OPTIND=1
unset X

doMain "$@"
