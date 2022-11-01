[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

# Library of PUSCH kernels

This folder contains an implementation targeting MemPool memory structure of the most computationally complex kernels in the 5G physical uplink shared channel (PUSCH) processing chain. The technical paper related to this work is on arXiv at the following link: https://arxiv.org/abs/2210.09196.

## Compilation with GCC

The kernels can be compiled inside this directory:

```bash
make cfft_q16radix4
```

Each kernel is compiled with a set of default "define" directives. The default values can be overwritten at compile time:

```bash
DEFINES='-DTEST_4096 -DPARALLEL' \
make cfft_q16radix4
```

To have a complete picture of the possible alternatives for each kernel please refer to the "help" target of the Makefile:

```bash
app=cfft_q16radix4 make help
```
## RTL simulation

MemPool supports a set of different RTL simulators. You can run RTL simulations from this folder through the "runrtl" target of the Makefile. Please always specify the app to simulate and the simulator to use. In case you want to use Verilator:

```bash
app=cfft_q16radix4 simulator=verilate make runrtl
```

You can switch between MemPool and TeraPool configurations modifying the configuration file in the config folder of MemPool.
