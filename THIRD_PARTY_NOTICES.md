# Third-Party Notices

This repository includes or adapts algorithms and reference material from the
projects listed below. Source-file notices are retained where code is copied or
adapted.

## Random123 / Philox

- Source: `vendor/random123/Random123/philox.h`
- Upstream: D. E. Shaw Research Random123
- Notice: copyright and redistribution terms are retained in the vendored
  source files.

## SFMT

- Source: `vendor/SFMT-src-1.5.1`
- License file: `vendor/SFMT-src-1.5.1/LICENSE.txt`
- Notice: existing vendored license text is preserved unchanged.

## WELL19937a

- Source: https://simul.iro.umontreal.ca/rng/WELL19937a.c
- Authors: Francois Panneton, Pierre L'Ecuyer, Makoto Matsumoto
- Notice from the reference source:

```text
Copyright: Francois Panneton and Pierre L'Ecuyer, Universite de Montreal
Makoto Matsumoto, Hiroshima University

This code can be used freely for personal, academic, or non-commercial
purposes. For commercial purposes, please contact P. L'Ecuyer at:
lecuyer@iro.UMontreal.ca

This code can also be used under the terms of the GNU General Public License
as published by the Free Software Foundation, either version 3 of the License,
or any later version.
```

## PCG64 DXSM

- PCG C++ reference: https://github.com/imneme/pcg-cpp
- License files:
  - `vendor/licenses/pcg-cpp/LICENSE-MIT.txt`
  - `vendor/licenses/pcg-cpp/LICENSE-APACHE.txt`
- Notice from the reference source:

```text
PCG Random Number Generation for C++

Copyright 2014-2022 Melissa O'Neill <oneill@pcg-random.org>,
                    and the PCG Project contributors.

SPDX-License-Identifier: (Apache-2.0 OR MIT)
```

## pcg-dxsm C Cross-Check

- Source: https://github.com/fanf2/pcg-dxsm
- License identifier in source: `SPDX-License-Identifier: 0BSD OR MIT-0`
- Upstream README license notice:

```text
Written by Tony Finch <dot@dotat.at> in Cambridge.

Permission is hereby granted to use, copy, modify, and/or
distribute this software for any purpose with or without fee.

This software is provided 'as is', without warranty of any kind.
In no event shall the authors be liable for any damages arising
from the use of this software.

SPDX-License-Identifier: 0BSD OR MIT-0
```

## RomuTrio

- Source: https://www.romu-random.org/
- Paper: https://www.romu-random.org/romupaper.pdf
- Author: Mark A. Overton
- License file: `vendor/licenses/romu/LICENSE-APACHE-2.0.txt`
- Notice: the Romu paper appendices publish reference source under the Apache
  License, Version 2.0.

## Lemire Fast Bounded Integers

- Source: https://github.com/lemire/fastrange
- Paper: Daniel Lemire, "Fast Random Integer Generation in an Interval"
- License file: `vendor/licenses/fastrange/LICENSE`
- Notice: the bounded-integer helper adapts the unbiased multiply-high
  rejection method published by Daniel Lemire.
