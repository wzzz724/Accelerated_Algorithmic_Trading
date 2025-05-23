#!/usr/bin/env bash

#
# Copyright 2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Sets up environment for running the python side of the AAT hw_emu

if [ -z "$XILINX_VIVADO" ]; then
    echo "XILINX_VIVADO variable needs to be set"
    return 1
fi
CUR_DIR=`pwd`/$(dirname "$BASH_SOURCE")
export PYTHONPATH=$XILINX_VIVADO/data/emulation/hw_em/lib/python:$XILINX_VIVADO/data/emulation/ip_utils/xtlm_ipc/xtlm_ipc_v1_0/python:$CUR_DIR

echo "PYTHONPATH=$PYTHONPATH"
