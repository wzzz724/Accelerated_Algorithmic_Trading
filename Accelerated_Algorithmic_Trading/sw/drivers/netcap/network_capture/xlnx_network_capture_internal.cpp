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


#include "xlnx_network_capture.h"
#include "xlnx_network_capture_internal.h"
using namespace XLNX;






uint32_t NetworkCapture::CheckIsInitialised(void)
{
    uint32_t retval = XLNX_OK;

    if (m_initialisedMagicNumber != XLNX_NETWORK_CAPTURE_INITIALISED_MAGIC_NUMBER)
    {
        retval = XLNX_NETWORK_CAPTURE_ERROR_NOT_INITIALISED;
    }

    return retval;
}




uint32_t NetworkCapture::CheckIsRunning(void)
{
    uint32_t retval = XLNX_OK;
    bool bIsRunning = false;

    retval = IsRunning(&bIsRunning);

    if (retval == XLNX_OK)
    {
        if (bIsRunning == false)
        {
            retval = XLNX_NETWORK_CAPTURE_ERROR_NOT_RUNNING;
        }
    }

    return retval;
}






uint32_t NetworkCapture::CheckIsNotRunning(void)
{
    uint32_t retval = XLNX_OK;
    bool bIsRunning = false;

    retval = IsRunning(&bIsRunning);

    if (retval == XLNX_OK)
    {
        if (bIsRunning)
        {
            retval = XLNX_NETWORK_CAPTURE_ERROR_ALREADY_RUNNING;
        }
    }

    return retval;
}
