/*
 * Intel MIC Platform Software Stack (MPSS)
 *
 * Copyright(c) 2014 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * Intel SCIF driver.
 *
 */
#ifndef SCIF_EPD_H
#define SCIF_EPD_H

#include <linux/delay.h>
#include <linux/scif.h>
#include <linux/scif_ioctl.h>

#define SCIF_EPLOCK_HELD true

enum scif_epd_state {
	SCIFEP_UNBOUND,
	SCIFEP_BOUND,
	SCIFEP_LISTENING,
	SCIFEP_CONNECTED,
	SCIFEP_CONNECTING,
	SCIFEP_MAPPING,
	SCIFEP_CLOSING,
	SCIFEP_CLLISTEN,
	SCIFEP_DISCONNECTED,
	SCIFEP_ZOMBIE
};

/*
 * struct scif_conreq - Data structure added to the connection list.
 *
 * @msg: connection request message received
 * @list: link to list of connection requests
 */
struct scif_conreq {
	struct scifmsg msg;
	struct list_head list;
};

/* Size of the RB for the Endpoint QP */
#define SCIF_ENDPT_QP_SIZE 0x1000

/*
 * scif_endpt_qp_info - SCIF endpoint queue pair
 *
 * @qp - Qpair for this endpoint
 * @qp_offset - DMA address of the QP
 * @gnt_pld - Payload in a SCIF_CNCT_GNT message containing the
 * physical address of the remote_qp.
 */
struct scif_endpt_qp_info {
	struct scif_qp *qp;
	dma_addr_t qp_offset;
	dma_addr_t gnt_pld;
};

/*
 * struct scif_endpt - The SCIF endpoint data structure
 *
 * @state: end point state
 * @lock: lock synchronizing access to endpoint fields like state etc
 * @port: self port information
 * @peer: peer port information
 * @backlog: maximum pending connection requests
 * @qp_info: Endpoint QP information for SCIF messaging
 * @remote_dev: scifdev used by this endpt to communicate with remote node.
 * @remote_ep: remote endpoint
 * @conreqcnt: Keep track of number of connection requests.
 * @files: Open file information used to match the id passed in with
 *         the flush routine.
 * @conlist: list of connection requests
 * @conwq: waitqueue for connection processing
 * @discon: completion used during disconnection
 * @sendwq: waitqueue used during sending messages
 * @recvwq: waitqueue used during message receipt
 * @sendlock: Synchronize ordering of messages sent
 * @recvlock: Synchronize ordering of messages received
 * @list: link to list of various endpoints like connected, listening etc
 * @li_accept: pending ACCEPTREG
 * @acceptcnt: pending ACCEPTREG cnt
 * @liacceptlist: link to listen accept
 * @miacceptlist: link to uaccept
 * @listenep: associated listen ep
 * @conn_work: Non blocking connect work
 * @conn_port: Connection port
 * @conn_err: Errors during connection
 * @conn_async_state: Async connection
 * @conn_list: List of async connection requests
 */
struct scif_endpt {
	enum scif_epd_state state;
	spinlock_t lock;
	struct scif_port_id port;
	struct scif_port_id peer;
	int backlog;
	struct scif_endpt_qp_info qp_info;
	struct scif_dev *remote_dev;
	u64 remote_ep;
	int conreqcnt;
	struct files_struct *files;
	struct list_head conlist;
	wait_queue_head_t conwq;
	struct completion discon;
	wait_queue_head_t sendwq;
	wait_queue_head_t recvwq;
	struct mutex sendlock;
	struct mutex recvlock;
	struct list_head list;
	struct list_head li_accept;
	int acceptcnt;
	struct list_head liacceptlist;
	struct list_head miacceptlist;
	struct scif_endpt *listenep;
	struct scif_port_id conn_port;
	int conn_err;
	int conn_async_state;
	struct list_head conn_list;
};

static inline int scifdev_alive(struct scif_endpt *ep)
{
	return _scifdev_alive(ep->remote_dev);
}

void scif_cleanup_zombie_epd(void);
void scif_teardown_ep(void *endpt);
void scif_cleanup_ep_qp(struct scif_endpt *ep);
void scif_add_epd_to_zombie_list(struct scif_endpt *ep, bool eplock_held);
void scif_get_node_info(void);
void scif_send_acks(struct scif_dev *dev);
void scif_conn_handler(struct work_struct *work);
int scif_rsrv_port(u16 port);
void scif_get_port(u16 port);
int scif_get_new_port(void);
void scif_put_port(u16 port);
int __scif_flush(scif_epd_t epd);
#endif /* SCIF_EPD_H */
