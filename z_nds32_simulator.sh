#!/bin/bash

set -e

toolchain_path=$HOME/toolchain/nds32le-elf-mculib-v3

sid ${toolchain_path}/config/nds32-target-config.conf
