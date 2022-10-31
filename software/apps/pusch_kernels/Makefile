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
								printf "\n \t DEFINES+=-DSINGLE            --> single-core"; \
								printf "\n \t DEFINES+=-DPARALLEL          --> trivial parallelization without folding"; \
								printf "\n \t DEFINES+=-DFOLDED            --> parallel kernel with folding of input data"; \
								printf "\n \t DEFINES+=-DFOLDED_TWIDDLES   --> folding of twiddles"; \
								printf "\n \t DEFINES+=-DBITREVERSAL       --> LUTs are used for bitreversal"; \
								printf "\n \t DEFINES+=-DPRINT_PARALLEL    --> Print result of parallel execution"; \
								printf "\n \t DEFINES+=-DPRINT_FOLDED      --> Print result of parallel-folded execution"; \
								printf "\n \t DEFINES+=-DPRINT_SINGLE      --> Print result of parallel execution\n\n";}'
endif
ifeq ($(app),cfft_q16radix4_schedule)
	@awk 'BEGIN { printf "\n Defines in FFT kernel scheduled over subsets of the cluster:"; \
								printf "\n \t Choose one of the possible test options between TEST_64, TEST_256, TEST_1024 and TEST_4096"; \
								printf "\n \t E.g. to compile an FFT on 64 points DEFINES+=-DTEST_64 make cfft_q16radix4\n"; \
								printf "\n \t DEFINES+=-DSINGLE            --> single-core"; \
								printf "\n \t DEFINES+=-DN_FFTs=X          --> single-core FFT iterates X times"; \
								printf "\n \t DEFINES+=-DPARALLEL          --> parallel kernel with folding of input data"; \
								printf "\n \t DEFINES+=-DN_FFTs_ROW=X      --> X fine-grained parallelized FFTs are instantiated in parallel on the cluster"; \
								printf "\n \t DEFINES+=-DN_FFTs_COL=Y      --> Y fine-grained parallelized FFTs are run serially on a sub-set of cores\n\n";}'
endif
ifeq ($(app),choldec)
	@awk 'BEGIN { printf "\n Defines in Cholesky decomposition kernel:"; \
								printf "\n \t DEFINES+=-DSINGLE            --> single-core"; \
								printf "\n \t DEFINES+=-DPARALLEL          --> parallel kernel without folding of input data"; \
								printf "\n \t DEFINES+=-DFOLDED            --> parallel kernel with folding of input data"; \
								printf "\n \t DEFINES+=-DLINSOLVER         --> Solve the inversion of a full linear system Ax=b"; \
								printf "\n \t DEFINES+=-DN=X               --> Dimension of the matrix to be decomposed\n\n";}'
endif
ifeq ($(app),choldec_schedule)
	@awk 'BEGIN { printf "\n Defines in Cholesky decomposition kernel scheduled over subsets of the cluster:"; \
								printf "\n \t DEFINES+=-DSINGLE            --> single-core"; \
								printf "\n \t DEFINES+=-DN_DEC=X           --> single-core FFT iterates X times"; \
								printf "\n \t DEFINES+=-DPARALLEL          --> parallel kernel with folding of input data"; \
								printf "\n \t DEFINES+=-DLINSOLVER         --> Solve the inversion of a full linear system Ax=b"; \
								printf "\n \t DEFINES+=-DN_ROW=X           --> X fine-grained parallelized FFTs are instantiated in parallel on the cluster"; \
								printf "\n \t DEFINES+=-DN_COL=Y           --> Y fine-grained parallelized FFTs are run serially on a sub-set of cores"; \
								printf "\n \t DEFINES+=-DN=X               --> Dimension of the matrix to be decomposed\n\n";}'
endif
ifeq ($(app),matmul_i32_conflict_opt)
	@awk 'BEGIN { printf "\n Defines in matrix multiplication kernel:"; \
								printf "\n \t DEFINES+=-DSINGLE               --> single-core"; \
								printf "\n \t DEFINES+=-DPARALLEL             --> parallel kernel with folding of input data"; \
								printf "\n \t DEFINES+=-DPARALLEL_CONCURRENT  --> parallel kernel with folding of input data\n\n";}'
endif

.INTERMEDIATE: $(addsuffix /main.c.o,$(APPS))

