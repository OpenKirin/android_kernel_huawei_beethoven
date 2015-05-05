/****************************************************************************
 * Driver for Solarflare network controllers and boards
 * Copyright 2014-2015 Solarflare Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, incorporated herein by reference.
 */

#ifndef EFX_SRIOV_H
#define EFX_SRIOV_H

#include "net_driver.h"

#ifdef CONFIG_SFC_SRIOV

int efx_sriov_set_vf_mac(struct net_device *net_dev, int vf_i, u8 *mac);
int efx_sriov_set_vf_vlan(struct net_device *net_dev, int vf_i, u16 vlan,
			  u8 qos);
int efx_sriov_set_vf_spoofchk(struct net_device *net_dev, int vf_i,
			      bool spoofchk);
int efx_sriov_get_vf_config(struct net_device *net_dev, int vf_i,
			    struct ifla_vf_info *ivi);

#endif /* CONFIG_SFC_SRIOV */

#endif /* EFX_SRIOV_H */
