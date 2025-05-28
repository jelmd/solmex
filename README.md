# solmex

solmex is a *m*etrics *ex*porter for the Solaris OS. It collects OS/kernel-specific data and can optionally expose this data via HTTP in the [Prometheuse exposition format](https://prometheus.io/docs/instrumenting/exposition_formats/). This allows metric collector and time series database tools like VictoriaMetrics and Prometheus to gather these metrics (e.g., via http://_hostname:9100_/metrics). These databases can hen be utilized to further process, reuse, or visualize the metrics using tools such as [Grafana](https://grafana.com/).

*solmex* s capable of retrieving and exposing comprehensive data from the Solaris OS/kernel, utilizing its kstats facilities among other resources. However, it limits the data to those typically required for everyday monitoring. It functions as a true daemon, written in C, which caches as much data as possible. Therefore, it is approximately 100 to 500 times faster than node-exporter, depending on the specific data being exported, and it provides significantly more useful metrics than node-exporter or similar tools.

## KISS

Efficiency, size, and simplicity are key objectives of solmex. Consequently, aside from using [libprom](https://github.com/jelmd/libprom) for handling certain Prometheus (PROM) functionality and [libmicrohttpd](https://github.com/Karlson2k/libmicrohttpd) for providing HTTP access, no additional third-party libraries or tools are utilized. The core idea is to operate *solmex* as a local service on the machine being monitored, while leveraging existing OS tools and services, such as firewalls, HTTP proxies, and VictoriaMetrics vmagent to control access to the exposed data. This approach complements the exporters, which are primarily specialized in collecting and simply exporting metrics, by providing the desired level of security and access control."

## Examples

- [all stats emitted by *solmex* incl. comments](https://github.com/jelmd/solmex/blob/main/etc/example-full.out)
- [all stats emitted by *solmex* excl. comments](https://github.com/jelmd/solmex/blob/main/etc/example-full-noComments.out)
- [a day-by-day example we use](https://github.com/jelmd/solmex/blob/main/etc/example-day-by-day.out)

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

*solmex* follows the basic principles of semantic versioning with practical considerations. Official releases always consist of three numbers (A.B.C), strictly neither more nor less. For nightly, alpha, beta, or release candidate (RC) builds, etc., a '.0' and potentially additional dot-separated digits are appended. This allows for the possibility of overwriting a pre-release by utilizing a fourth digit greater than 0.


## License

[CDDL 1.1](https://spdx.org/licenses/CDDL-1.1.html)


## Packages

Solaris packages for libprom and solmex can be found via https://pkg.cs.ovgu.de/LNF/i386/5.11 (search for LNFlibprom and LNFsolmex). libmicrohttpd gets provided by Solaris itself, so using the vendor package is recommended (library/libmicrohttpd). Related header sources files get installed, if the develop facet is set to true.
