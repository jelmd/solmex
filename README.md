# solmex

solmex is a *m*etrics *ex*porter for Solaris OS. *solmex* collects OS/kernel specific data and optionally exposes them via HTTP in [Prometheuse exposition format](https://prometheus.io/docs/instrumenting/exposition_formats/) using the endpoint URL http://_hostname:9100_/metrics (port and IP are customizable of course) and thus visualized e.g. using [Grafana](https://grafana.com/), [Netdata](https://www.netdata.cloud/), or [Zabbix](https://www.zabbix.com/).

Basically *solmex* is able to retrieve and expose all data from the Solaris OS/kernel e.g. using its kstats facilities and friends. It is a real daemon written in **C**, which caches as much data as possible.

## KISS

Since efficiency, size and simplicity of the utility is one of its main goals, beside [libprom](https://github.com/jelmd/libprom) to handle some prometheus (PROM) related stuff and [libmicrohttpd](https://github.com/Karlson2k/libmicrohttpd) to provide http access, no additional 3rd party libraries, tools, etc. are used. The basic idea is to run *solmex* as a local service on the machine to monitor and use OS tools and services (firewall, http proxy, VictoriaMetrics vmagent, and the like) to control access to exposed data and to replace/ament the wrt. to metrics very limited Prometheus exporter and the like.

## Requirements

- [libprom](https://github.com/jelmd/libprom)
- [libmicrohttpd](https://github.com/Karlson2k/libmicrohttpd)


## Build

Adjust the **Makefile** if needed, optionally set related environment variables
(e.g. `export USE_CC=gcc`) and run GNU **make**.

## Repo

The official repository for *solmex* is https://github.com/jelmd/solmex .
If you need some new features (or bug fixes), please feel free to create an
issue there using https://github.com/jelmd/solmex/issues .


## Versioning

*solmex* follows the basic idea of semantic versioning, but having the real world
in mind. Therefore official releases have always THREE numbers (A.B.C), not
more and not less! For nightly, alpha, beta, RC builds, etc. a *.0* and
possibly more dot separated digits will be append, so that one is always able
to overwrite this one by using a 4th digit > 0.


## License

[CDDL 1.1](https://spdx.org/licenses/CDDL-1.1.html)


## Packages
Solaris packages for libprom and solmex can be found via https://pkg.cs.ovgu.de/LNF/i386/5.11 (search for LNFlibprom and LNFsolmex). libmicrohttpd gets provided by Solaris itself, so using the vendor package is recommended (library/libmicrohttpd). Related header sources files get installed, if the develop facet is set to true.
