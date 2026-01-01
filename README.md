

# <img src="doc/export-2024-08-26 121525.svg" style="height:80px; width: auto;" /> DatagramDB

This repository provides a preliminary implementation of the Generalised Semistructured Model and of its associated language,
Generalised Semistructured Query Language, as well.

Please read the Wiki for better documentation: https://github.com/datagram-db/gsm_gsql/wiki

## Dependencies

 * gcc and g++ version 14.2.0 (libstdc++6 14.2.0)
 * clang-20 and clang++-20
 * zsv (submodules/zsv) for CSV parsing. See build.md!

## Authors

* Giacomo Bergami (2023 - )
* Wiktor Zegadło (2023)
* Oliver R. Fox (2024 - 2025)


## MetaModel

A preliminary implementation of ```Script v2.0``` is provided in [this](https://github.com/datagram-db/gsm_gsql/tree/withscript) branch, as well as the implementation of some examples of such language from the paper [TBA]. The original implementation from Giacomo Bergami's PhD dissertation is available on [BitBucket](https://bitbucket.org/unibogb/gsql-script/src/master/).

## Datasets

For reproducing the experiments from the paper "[Towards a Generalised Semistructured data Model and Query Language](https://dl.acm.org/doi/10.1145/3609429.3609433)", please download [this](https://files.de-1.osf.io/v1/resources/pgf9h/providers/osfstorage/?zip=) dataset and unzip it in the main project folder.

> Data: https://osf.io/pgf9h/files

