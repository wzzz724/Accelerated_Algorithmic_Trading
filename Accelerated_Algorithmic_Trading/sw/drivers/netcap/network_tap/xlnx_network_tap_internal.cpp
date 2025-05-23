/*
 * Copyright 2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "xlnx_network_tap.h"
#include "xlnx_network_tap_internal.h"
#include "xlnx_network_tap_error_codes.h"
using namespace XLNX;


uint32_t NetworkTap::CheckIsInitialised(void)
{
    uint32_t retval = XLNX_OK;

    if (m_initialisedMagicNumber != XLNX_NETWORK_TAP_INITIALISED_MAGIC_NUMBER)
    {
        retval = XLNX_NETWORK_TAP_ERROR_NOT_INITIALISED;
    }

    return retval;
}
