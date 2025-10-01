/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//DEFAULT INCLUSIONS
#include <stdio.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
//END DEFAULT INCLUSIONS

#include <stdlib.h> //needed for atoi
#include <string.h> //needed for parsing strings
#include <drivers/ipc_rpmsg.h> //needed for shared memory
#include <C:/ti/mmwave_mcuplus_sdk_04_07_01_04/mmwave_mcuplus_sdk_04_07_01_04/ti/utils/cli/cli.h> //needed for CLI wrapper
#include "FreeRTOS.h" //needed for task management
#include "task.h" //needed for task management

//Endpoints for each core's mailbox
#define R5F1_ENDPOINT   11
#define DSP_ENDPOINT    22

//RPMessage objects
static RPMessage_Object gMsgObj;
static uint32_t gMyEndPt;

//command structure
typedef struct {
    char op[4]; //opcode
    int x; //first number
    int y; //second number
} MathCmd;

/* This function sends commands to the correct cores to offload tasks
 */
static void send_to_core(uint16_t RemoteCoreID, uint16_t RemoteEndPt, MathCmd* cmd)
{
    RPMessage_send( (void*)cmd, sizeof(MathCmd),
                    RemoteCoreID, RemoteEndPt,
                    gMyEndPt, 60000);
}

/* This function handles addition
 * It is run locally
 */
static int32_t cmd_add(int32_t argc, char* argv[])
{
    if(argc == 3) //make sure there are 3 parts of the argument
    {
        int x = atoi(argv[1]); //set second part (first number) as x
        int y = atoi(argv[2]); //set third part (second number) as y
        int result = x + y; //add numbers

        //give result to the CLI
        DebugP_log("ADD result = %d\n", result);
    }
    return 0;
}

/*
 * This is adapted from the empty project provided in the ti sdk.
 * This handles the CLI interface and routing tasks.
 */

void empty_main(void *args)
{
    /* Open drivers to open the UART driver for console */
    Drivers_open();
    Board_driversOpen();

    //initiate CLI interface
    CLI_Cfg cliCfg = {0};
    cliCfg.cliUartHandle = gUartHandle[CONFIG_UART0]; //UART handle from sysconfig
    cliCfg.cliPrompt = "R5F0> ";
    cliCfg.taskPriority = 3;

    //-----SET UP COMMANDS-----
    //addition
    cliCfg.tableEntry[0].cmd = "ADD";
    cliCfg.tableEntry[0].helpString = "Add two integers";
    cliCfg.tableEntry[0].cmdHandlerFxn = cmd_add;

    CLI_open(&cliCfg);
    while(1)
    {
        vTaskDelay(500); //keep task alive
    }
    Board_driversClose();
    Drivers_close();
};
