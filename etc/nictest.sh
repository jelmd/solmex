#!/bin/ksh93

SVC_URL='http://localhost:9100/metrics'
TEST_VNIC='test0'
PARENT_LINK='a0'
SUDO='+'

integer N=31 M X=0
for I in {1..$N}; do
    M=0
	curl -s "${SVC_URL}" | while read  K V ; do
		(( M++ ))
		[[ $K == 'solmex_scrape_duration_seconds{collector="node"}' ]] && \
			print $V
	done
	(( M == 0 )) && break
	if (( I % 2 == 1 )); then
		if (( X )); then
			${SUDO} dladm delete-vnic ${TEST_VNIC}
			X=0
			print "VNIC deleted"
		else
			${SUDO} dladm create-vnic -l ${PARENT_LINK} ${TEST_VNIC}
			X=1
			print "VNIC created"
		fi
	fi
	sleep 1
done
