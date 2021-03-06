/* Copyright (C) 2007-2012 Open Information Security Foundation
 *
 * You can copy, redistribute or modify this Program under the terms of
 * the GNU General Public License version 2 as published by the Free
 * Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/** \file
 *
 *  \author Vadym Malakhatko <v.malakhatko@sirinsoftware.com>
 */


#include "suricata-common.h"
#include "suricata-plugin.h"

#include "flow.h"
#include "tm-threads.h"
#include "conf.h"
#include "runmodes.h"

#include "tm-threads.h"
#include "runmodes.h"
#include "runmode-testimony.h"
#include "output.h"
#include "source-testimony.h"

#include "util-debug.h"
#include "util-time.h"
#include "util-cpu.h"
#include "util-affinity.h"
#include "util-runmodes.h"
#include "util-device.h"
#include "util-ioctl.h"
#include "util-byte.h"

const char *RunModeTestimonyGetDefaultMode(void)
{
    return "workers";
}

void RunModeIdsTestimonyRegister(int slot)
{
    RunModeRegisterNewRunMode(slot, "single",
                              "Single threaded testimony mode",
                              RunModeIdsTestimonySingle);
    RunModeRegisterNewRunMode(slot, "workers",
                              "Workers testimony mode, each thread does all "
                              " tasks from acquisition to logging",
                              RunModeIdsTestimonyWorkers);
}

/**
 * \brief Each socket can have one thread per each fanout index
 */
static int TestimonyConfigGetThreadsCount(void *conf)
{
    TestimonySocketConfig *tconfig = (TestimonySocketConfig *)conf;
    return tconfig->fanout_size;
}

static void TestimonyDerefConfig(void *conf)
{
    TestimonySocketConfig *pfp = (TestimonySocketConfig *)conf;
    /* Testimony config is used only once but cost of this low. */
    if (SC_ATOMIC_SUB(pfp->ref, 1) == 1) {
        SCFree(pfp);
    }
}

static void *ParseTestimonyConfig(const char *socket)
{
    TestimonySocketConfig *tconf = SCMalloc(sizeof(*tconf));
    ConfNode *testimony_packet_node;
    ConfNode *sock_root;
    long fanout_size;

    if (unlikely(tconf == NULL)) {
        return NULL;
    }

    strlcpy(tconf->socket, socket, sizeof(tconf->socket));
    tconf->fanout_size = 1;

    SC_ATOMIC_INIT(tconf->current_fanout_index);
    tconf->DerefFunc = TestimonyDerefConfig;

    testimony_packet_node = ConfGetNode("testimony");
    if (testimony_packet_node == NULL) {
        SCLogInfo("unable to find testimony config using default values");
        goto finalize;
    }

    sock_root = ConfGetChildWithDefault(testimony_packet_node, NULL, socket);

    if (sock_root == NULL) {
        SCLogInfo("unable to find testimony config for "
                  "socket \"%s\"", socket);
        goto finalize;
    }

    if (ConfGetChildValueInt(sock_root, "fanout-size", &fanout_size) == 1) {
        tconf->fanout_size = fanout_size;
        SCLogInfo("Testimony fanout size is set to \"%ld\"", fanout_size); 
    }
finalize:
    SC_ATOMIC_RESET(tconf->ref);
    (void) SC_ATOMIC_ADD(tconf->ref, tconf->fanout_size);

    return tconf;
}

/**
 * \brief Single thread version of the Testimony live processing.
 */
int RunModeIdsTestimonySingle(void)
{
    SCEnter();
    int ret;
    const char *socket = NULL;

    RunModeInitialize();
    TimeModeSetLive();
    
    if (ConfGet("testimony.socket-path", &socket) != 1) {
        SCLogError(SC_ERR_INVALID_ARGUMENT, "No testimony socket path is set");
        SCReturnInt(TM_ECODE_FAILED);
    }

    ret = RunModeSetLiveCaptureSingle(ParseTestimonyConfig,
                                      TestimonyConfigGetThreadsCount,
                                      "ReceiveTestimony",
                                      "DecodeTestimony",
                                      thread_name_single,
                                      socket);
    if (ret != 0) {
        SCLogError(SC_ERR_RUNMODE, "Runmode start failed");
        exit(EXIT_FAILURE);
    }

    SCLogInfo("RunModeIdsTestimonySingle initialised");

    SCReturnInt(0);
}



/**
 * \brief Workers version of the PCAP LIVE processing.
 *
 * Start N threads with each thread doing all the work.
 *
 */
int RunModeIdsTestimonyWorkers(void)
{
    SCEnter();

    int ret;
    const char *live_dev = NULL;
    SCEnter();

    RunModeInitialize();
    TimeModeSetLive();

    if (ConfGet("testimony.socket-path", &live_dev) != 1) {
        SCLogError(SC_ERR_INVALID_ARGUMENT, "No testimony socket path is set");
        SCReturnInt(TM_ECODE_FAILED);
    }

    ret = RunModeSetLiveCaptureWorkers(ParseTestimonyConfig,
                                    TestimonyConfigGetThreadsCount,
                                    "ReceiveTestimony",
                                    "DecodeTestimony",
                                    thread_name_workers,
                                    live_dev);
    if (ret != 0) {
        SCLogError(SC_ERR_RUNMODE, "Unable to start runmode");
        exit(EXIT_FAILURE);
    }

    SCLogInfo("RunModeIdsTestimonyWorkers initialised");

    SCReturnInt(0);
}
