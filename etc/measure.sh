#!/bin/ksh93

# Simple script to check service time and calc min/max/average.

# URL to query
SVC_URL='http://localhost:9100/metrics'

# solmex
SEARCH='solmex_scrape_duration_seconds{collector="libprom"}'
# node-exporter
#SEARCH='node_scrape_collector_duration_seconds{collector="overall"}'
integer N=60 M
typeset -F16 SUM MIN=1000 MAX=0 AV
for I in {1..$N}; do
	printf '.'
	M=0
	curl -s "${SVC_URL}" | while read  K V ; do
		(( M++ ))
		[[ $K == "${SEARCH}" ]] || continue
		(( SUM += V )) 
		(( V < MIN )) && MIN=$V
		(( V > MAX )) && MAX=V
	done
	(( M == 0 )) && break
	sleep 1
done
(( AV = SUM / N ))

print "\nAVG: ${AV}   MIN: ${MIN}   MAX: ${MAX}"
