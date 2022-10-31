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
all: $(ALL)

$(APPS): % : $(BIN_DIR)/% $(PUSCH_DIR)/Makefile $(shell find $(RUNTIME_DIR)/**.{S,c,h,ld} -type f)

.PHONY: $(BINARIES)
$(BINARIES): $(BIN_DIR)/%: %/main.c.o $(RUNTIME) $(LINKER_SCRIPT) $(DATA) update_opcodes
	mkdir -p $(dir $@)
	$(RISCV_CC) -Iinclude $(RISCV_LDFLAGS) -o $@ $< $(RUNTIME) -T$(RUNTIME_DIR)/link.ld
	$(RISCV_OBJDUMP) $(RISCV_OBJDUMP_FLAGS) -D $@ > $@.dump

# Helper targets
update_opcodes:
	make -C $(MEMPOOL_DIR) update_opcodes

.PHONY: clean, runrtl

runrtl:
	make -C $(HARDWARE_DIR) $(simulator)
	make -C $(HARDWARE_DIR) trace

clean:
	rm -vf $(BINARIES)
	rm -vf $(addsuffix .dump,$(BINARIES))
	rm -vf $(addsuffix /main.c.o,$(APPS))
	rm -vf $(RUNTIME)
	rm -vf $(LINKER_SCRIPT)

.INTERMEDIATE: $(addsuffix /main.c.o,$(APPS))

