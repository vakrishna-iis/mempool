# Copyright 2021 ETH Zurich and University of Bologna.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

# Author: Marco Bertuletti, ETH Zurich

SHELL = /usr/bin/env bash
ROOT_DIR := $(patsubst %/,%, $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
MEMPOOL_DIR := $(shell git rev-parse --show-toplevel 2>/dev/null || echo $$MEMPOOL_DIR)
SOFTWARE_DIR := $(abspath $(MEMPOOL_DIR)/software)
HARDWARE_DIR := $(abspath $(MEMPOOL_DIR)/hardware)

APPS_DIR := $(abspath $(SOFTWARE_DIR)/apps)
RUNTIME_DIR := $(abspath $(SOFTWARE_DIR)/runtime)
BIN_DIR := $(abspath $(SOFTWARE_DIR)/bin)
PUSCH_DIR := $(ROOT_DIR)

# This will overwrite the ROOT_DIR variable from the included makefile
include $(RUNTIME_DIR)/runtime.mk

APPS := $(patsubst $(PUSCH_DIR)/%/main.c,%,$(shell find $(PUSCH_DIR) -name "main.c"))
DATA := $(patsubst %.args,%.h,$(shell find $(PUSCH_DIR) -name "data.args"))
BINARIES := $(addprefix $(BIN_DIR)/,$(APPS))

# Make all applications
all: ## Make all applications.
	$(ALL)

$(APPS): % : $(BIN_DIR)/% $(PUSCH_DIR)/Makefile $(shell find $(RUNTIME_DIR)/**.{S,c,h,ld} -type f)

.PHONY: $(BINARIES)
$(BINARIES): $(BIN_DIR)/%: %/main.c.o $(RUNTIME) $(LINKER_SCRIPT) $(DATA) update_opcodes
	mkdir -p $(dir $@)
	$(RISCV_CC) -Iinclude $(RISCV_LDFLAGS) -o $@ $< $(RUNTIME) -T$(RUNTIME_DIR)/link.ld
	$(RISCV_OBJDUMP) $(RISCV_OBJDUMP_FLAGS) -D $@ > $@.dump

# Helper targets
update_opcodes: ## Update the RISCV opcodes.
	make -C $(MEMPOOL_DIR) update_opcodes

.PHONY: clean, runrtl

runrtl: ## Run RTL simulation and tracing. Average performance are printed after tracing.
ifeq ($(app),)
	$(warning " WARNING: You need to specify the app that you want to run!")
		app =
endif
ifeq ($(simulator),)
	$(warning " WARNING: You need to specify the simulator that you want to run!")
		simulator =
endif
	make -C $(HARDWARE_DIR) $(simulator)
	make -C $(HARDWARE_DIR) trace

clean: ## Clean the bin folder.
	rm -vf $(BINARIES)
	rm -vf $(addsuffix .dump,$(BINARIES))
	rm -vf $(addsuffix /main.c.o,$(APPS))
	rm -vf $(RUNTIME)
	rm -vf $(LINKER_SCRIPT)

help:
	@awk 'BEGIN { printf "\n TIPS:"; \
								printf "\n -- Apps can be compiled with `make` followed by the name of the app."; \
								printf "\n -- To specify which test has to be run specify the required `#define` directives before compiling the application"; \
								printf "\n -- For each app `app=name_app make help` shows the possible alternatives.\n"; \
								printf "\n Makefile targets: \n";}'
	@egrep -h '\s##\s' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m  %-30s:\033[0m %s\n", $$1, $$2}'
	@awk 'BEGIN { printf "\n";}'
ifeq ($(app),cfft_q16radix4)
	@awk 'BEGIN { printf "\n Defines in FFT kernel:"; \
								printf "\n \t Choose one of the possible test options between TEST_64, TEST_256, TEST_1024 and TEST_4096"; \
								printf "\n \t E.g. to compile an FFT on 64 points DEFINES+=-DTEST_64 make cfft_q16radix4\n"; \
								printf "\n \t DEFINES=\x27-DSINGLE\x27            --> single-core"; \
								printf "\n \t DEFINES=\x27-DPARALLEL\x27          --> trivial parallelization without folding"; \
								printf "\n \t DEFINES=\x27-DFOLDED\x27            --> parallel kernel with folding of input data"; \
								printf "\n \t DEFINES=\x27-DFOLDED_TWIDDLES\x27   --> folding of twiddles"; \
								printf "\n \t DEFINES=\x27-COMPUTE_BITREV\x27     --> LUTs are not used for bitreversal"; \
								printf "\n \t DEFINES=\x27-DPRINT_PARALLEL\x27    --> Print result of parallel execution"; \
								printf "\n \t DEFINES=\x27-DPRINT_FOLDED\x27      --> Print result of parallel-folded execution"; \
								printf "\n \t DEFINES=\x27-DPRINT_SINGLE\x27      --> Print result of parallel execution\n\n";}'
endif
ifeq ($(app),cfft_q16radix4_schedule)
	@awk 'BEGIN { printf "\n Defines in FFT kernel scheduled over subsets of the cluster:"; \
								printf "\n \t Choose one of the possible test options between TEST_64, TEST_256, TEST_1024 and TEST_4096"; \
								printf "\n \t E.g. to compile an FFT on 64 points DEFINES+=-DTEST_64 make cfft_q16radix4\n"; \
								printf "\n \t DEFINES=\x27-DSINGLE\x27            --> single-core"; \
								printf "\n \t DEFINES=\x27-DN_FFTs=X\x27          --> single-core FFT iterates X times"; \
								printf "\n \t DEFINES=\x27-DPARALLEL\x27          --> parallel kernel with folding of input data"; \
								printf "\n \t DEFINES=\x27-COMPUTE_BITREV\x27     --> LUTs are not used for bitreversal"; \
								printf "\n \t DEFINES=\x27-DN_FFTs_ROW=X\x27      --> X fine-grained parallelized FFTs are instantiated in parallel on the cluster"; \
								printf "\n \t DEFINES=\x27-DN_FFTs_COL=Y\x27      --> Y fine-grained parallelized FFTs are run serially on a sub-set of cores\n\n";}'
endif
ifeq ($(app),choldec)
	@awk 'BEGIN { printf "\n Defines in Cholesky decomposition kernel:"; \
								printf "\n \t DEFINES=\x27-DSINGLE\x27            --> single-core"; \
								printf "\n \t DEFINES=\x27-DPARALLEL\x27          --> parallel kernel without folding of input data"; \
								printf "\n \t DEFINES=\x27-DFOLDED\x27            --> parallel kernel with folding of input data"; \
								printf "\n \t DEFINES=\x27-DLINSOLVER\x27         --> Solve the inversion of a full linear system Ax=b"; \
								printf "\n \t DEFINES=\x27-DN=X\x27               --> Dimension of the matrix to be decomposed\n\n";}'
endif
ifeq ($(app),choldec_schedule)
	@awk 'BEGIN { printf "\n Defines in Cholesky decomposition kernel scheduled over subsets of the cluster:"; \
								printf "\n \t DEFINES=\x27-DSINGLE\x27            --> single-core"; \
								printf "\n \t DEFINES=\x27-DN_DEC=X\x27           --> single-core FFT iterates X times"; \
								printf "\n \t DEFINES=\x27-DPARALLEL\x27          --> parallel kernel with folding of input data"; \
								printf "\n \t DEFINES=\x27-DLINSOLVER\x27         --> Solve the inversion of a full linear system Ax=b"; \
								printf "\n \t DEFINES=\x27-DN_ROW=X\x27           --> X fine-grained parallelized FFTs are instantiated in parallel on the cluster"; \
								printf "\n \t DEFINES=\x27-DN_COL=Y\x27           --> Y fine-grained parallelized FFTs are run serially on a sub-set of cores"; \
								printf "\n \t DEFINES=\x27-DN=X\x27               --> Dimension of the matrix to be decomposed\n\n";}'
endif
ifeq ($(app),matmul_i32_conflict_opt)
	@awk 'BEGIN { printf "\n Defines in matrix multiplication kernel:"; \
								printf "\n \t DEFINES=\x27-DSINGLE\x27               --> single-core"; \
								printf "\n \t DEFINES=\x27-DPARALLEL\x27             --> parallel kernel with folding of input data"; \
								printf "\n \t DEFINES=\x27-DPARALLEL_CONCURRENT\x27  --> parallel kernel with folding of input data\n\n";}'
endif

.INTERMEDIATE: $(addsuffix /main.c.o,$(APPS))

