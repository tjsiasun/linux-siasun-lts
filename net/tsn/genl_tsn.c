// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/* Copyright 2017-2019 NXP */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/if_vlan.h>
#include <net/genetlink.h>
#include <net/netlink.h>
#include <linux/version.h>
#include <net/tsn.h>

#define NLA_PARSE_NESTED(a, b, c, d) \
	nla_parse_nested(a, b, c, d, NULL)
#define NLA_PUT_U64(a, b, c) nla_put_u64_64bit(a, b, c, NLA_U64)

static struct genl_family tsn_family;

LIST_HEAD(port_list);

static const struct nla_policy tsn_cmd_policy[TSN_CMD_ATTR_MAX + 1] = {
	[TSN_CMD_ATTR_MESG]		= { .type = NLA_STRING },
	[TSN_CMD_ATTR_DATA]		= { .type = NLA_S32 },
	[TSN_ATTR_IFNAME]		= { .type = NLA_STRING },
	[TSN_ATTR_PORT_NUMBER]		= { .type = NLA_U8 },
	[TSN_ATTR_CAP]			= { .type = NLA_NESTED	},
	[TSN_ATTR_QBV]			= { .type = NLA_NESTED },
	[TSN_ATTR_STREAM_IDENTIFY]	= { .type = NLA_NESTED },
	[TSN_ATTR_QCI_SP]		= { .type = NLA_NESTED },
	[TSN_ATTR_QCI_SFI]		= { .type = NLA_NESTED },
	[TSN_ATTR_QCI_SGI]		= { .type = NLA_NESTED },
	[TSN_ATTR_QCI_FMI]		= { .type = NLA_NESTED },
	[TSN_ATTR_CBS]			= { .type = NLA_NESTED },
	[TSN_ATTR_TSD]			= { .type = NLA_NESTED },
	[TSN_ATTR_QBU]			= { .type = NLA_NESTED },
	[TSN_ATTR_CT]			= { .type = NLA_NESTED },
	[TSN_ATTR_CBGEN]		= { .type = NLA_NESTED },
	[TSN_ATTR_CBREC]		= { .type = NLA_NESTED },
	[TSN_ATTR_CBSTAT]               = { .type = NLA_NESTED },
	[TSN_ATTR_DSCP]                 = { .type = NLA_NESTED },
};

static const struct nla_policy tsn_cap_policy[TSN_CAP_ATTR_MAX + 1] = {
	[TSN_CAP_ATTR_QBV]		= { .type = NLA_FLAG },
	[TSN_CAP_ATTR_QCI]		= { .type = NLA_FLAG },
	[TSN_CAP_ATTR_QBU]		= { .type = NLA_FLAG },
	[TSN_CAP_ATTR_CBS]		= { .type = NLA_FLAG },
	[TSN_CAP_ATTR_CB]		= { .type = NLA_FLAG },
	[TSN_CAP_ATTR_TBS]		= { .type = NLA_FLAG },
	[TSN_CAP_ATTR_CTH]		= { .type = NLA_FLAG },
};

static const struct nla_policy qci_cap_policy[TSN_QCI_STREAM_ATTR_MAX + 1] = {
	[TSN_QCI_STREAM_ATTR_MAX_SFI]	= { .type = NLA_U32 },
	[TSN_QCI_STREAM_ATTR_MAX_SGI]	= { .type = NLA_U32 },
	[TSN_QCI_STREAM_ATTR_MAX_FMI]	= { .type = NLA_U32 },
	[TSN_QCI_STREAM_ATTR_SLM]	= { .type = NLA_U32 },
};

static const struct nla_policy ct_policy[TSN_CT_ATTR_MAX + 1] = {
	[TSN_CT_ATTR_QUEUE_STATE]	= { .type = NLA_U8 }
};

static const struct nla_policy cbgen_policy[TSN_CBGEN_ATTR_MAX + 1] = {
	[TSN_CBGEN_ATTR_INDEX]		= { .type = NLA_U32 },
	[TSN_CBGEN_ATTR_PORT_MASK]	= { .type = NLA_U8 },
	[TSN_CBGEN_ATTR_SPLIT_MASK]	= { .type = NLA_U8 },
	[TSN_CBGEN_ATTR_SEQ_LEN]	= { .type = NLA_U8 },
	[TSN_CBGEN_ATTR_SEQ_NUM]	= { .type = NLA_U32 },
};

static const struct nla_policy cbrec_policy[TSN_CBREC_ATTR_MAX + 1] = {
	[TSN_CBREC_ATTR_INDEX]		= { .type = NLA_U32 },
	[TSN_CBREC_ATTR_SEQ_LEN]	= { .type = NLA_U8 },
	[TSN_CBREC_ATTR_HIS_LEN]	= { .type = NLA_U8 },
	[TSN_CBREC_ATTR_TAG_POP_EN]	= { .type = NLA_FLAG },
};

static const struct nla_policy cbstat_policy[TSN_CBSTAT_ATTR_MAX + 1] = {
	[TSN_CBSTAT_ATTR_INDEX]         = { .type = NLA_U32 },
	[TSN_CBSTAT_ATTR_GEN_REC]       = { .type = NLA_U8 },
	[TSN_CBSTAT_ATTR_ERR]		= { .type = NLA_U8 },
	[TSN_CBSTAT_ATTR_SEQ_NUM]       = { .type = NLA_U32 },
	[TSN_CBSTAT_ATTR_SEQ_LEN]       = { .type = NLA_U8 },
	[TSN_CBSTAT_ATTR_SPLIT_MASK]    = { .type = NLA_U8 },
	[TSN_CBSTAT_ATTR_PORT_MASK]	= { .type = NLA_U8 },
	[TSN_CBSTAT_ATTR_HIS_LEN]       = { .type = NLA_U8 },
	[TSN_CBSTAT_ATTR_SEQ_HIS]       = { .type = NLA_U32 },
};

static const struct nla_policy qbu_policy[TSN_QBU_ATTR_MAX + 1] = {
	[TSN_QBU_ATTR_ADMIN_STATE]	= { .type = NLA_U8 },
	[TSN_QBU_ATTR_HOLD_ADVANCE]	= { .type = NLA_U32},
	[TSN_QBU_ATTR_RELEASE_ADVANCE]	= { .type = NLA_U32},
	[TSN_QBU_ATTR_ACTIVE]		= { .type = NLA_FLAG},
	[TSN_QBU_ATTR_HOLD_REQUEST]	= { .type = NLA_U8},
};

static const struct nla_policy cbs_policy[TSN_CBS_ATTR_MAX + 1] = {
	[TSN_CBS_ATTR_TC_INDEX]		= { .type = NLA_U8},
	[TSN_CBS_ATTR_BW]		= { .type = NLA_U8},
};

static const struct nla_policy tsd_policy[TSN_TSD_ATTR_MAX + 1] = {
	[TSN_TSD_ATTR_ENABLE]			= { .type = NLA_FLAG},
	[TSN_TSD_ATTR_DISABLE]			= { .type = NLA_FLAG},
	[TSN_TSD_ATTR_PERIOD]			= { .type = NLA_U32},
	[TSN_TSD_ATTR_MAX_FRM_NUM]		= { .type = NLA_U32},
	[TSN_TSD_ATTR_CYCLE_NUM]		= { .type = NLA_U32},
	[TSN_TSD_ATTR_LOSS_STEPS]		= { .type = NLA_U32},
	[TSN_TSD_ATTR_SYN_IMME]			= { .type = NLA_FLAG},
};

static const struct nla_policy qbv_policy[TSN_QBV_ATTR_MAX + 1] = {
	[TSN_QBV_ATTR_ADMINENTRY]	= {	.type = NLA_NESTED},
	[TSN_QBV_ATTR_OPERENTRY]	= { .type = NLA_NESTED},
	[TSN_QBV_ATTR_ENABLE]		= { .type = NLA_FLAG},
	[TSN_QBV_ATTR_DISABLE]		= { .type = NLA_FLAG},
	[TSN_QBV_ATTR_CONFIGCHANGE]	= { .type = NLA_FLAG},
	[TSN_QBV_ATTR_CONFIGCHANGETIME]	= { .type = NLA_U64},
	[TSN_QBV_ATTR_MAXSDU]		= { .type = NLA_U32},
	[TSN_QBV_ATTR_GRANULARITY]	= { .type = NLA_U32},
	[TSN_QBV_ATTR_CURRENTTIME]	= { .type = NLA_U64},
	[TSN_QBV_ATTR_CONFIGPENDING]	= {.type = NLA_FLAG},
	[TSN_QBV_ATTR_CONFIGCHANGEERROR]	= { .type = NLA_U64},
	[TSN_QBV_ATTR_LISTMAX]		= { .type = NLA_U32},
};

static const struct nla_policy qbv_ctrl_policy[TSN_QBV_ATTR_CTRL_MAX + 1] = {
	[TSN_QBV_ATTR_CTRL_LISTCOUNT]		= { .type = NLA_U32},
	[TSN_QBV_ATTR_CTRL_GATESTATE]		= { .type = NLA_U8},
	[TSN_QBV_ATTR_CTRL_CYCLETIME]		= { .type = NLA_U32},
	[TSN_QBV_ATTR_CTRL_CYCLETIMEEXT]	= { .type = NLA_U32},
	[TSN_QBV_ATTR_CTRL_BASETIME]		= { .type = NLA_U64},
	[TSN_QBV_ATTR_CTRL_LISTENTRY]		= { .type = NLA_NESTED},
};

static const struct nla_policy qbv_entry_policy[TSN_QBV_ATTR_ENTRY_MAX + 1] = {
	[TSN_QBV_ATTR_ENTRY_ID]	= { .type = NLA_U32},
	[TSN_QBV_ATTR_ENTRY_GC]	= { .type = NLA_U8},
	[TSN_QBV_ATTR_ENTRY_TM]	= { .type = NLA_U32},
};

static const struct nla_policy cb_streamid_policy[TSN_STREAMID_ATTR_MAX + 1] = {
	[TSN_STREAMID_ATTR_INDEX]	= { .type = NLA_U32},
	[TSN_STREAMID_ATTR_ENABLE]	= { .type = NLA_FLAG},
	[TSN_STREAMID_ATTR_DISABLE]	= { .type = NLA_FLAG},
	[TSN_STREAMID_ATTR_STREAM_HANDLE]	= { .type = NLA_S32},
	[TSN_STREAMID_ATTR_IFOP]	= { .type = NLA_U32},
	[TSN_STREAMID_ATTR_OFOP]	= { .type = NLA_U32},
	[TSN_STREAMID_ATTR_IFIP]	= { .type = NLA_U32},
	[TSN_STREAMID_ATTR_OFIP]	= { .type = NLA_U32},
	[TSN_STREAMID_ATTR_TYPE]	= { .type = NLA_U8},
	[TSN_STREAMID_ATTR_NDMAC]	= { .type = NLA_U64},
	[TSN_STREAMID_ATTR_NTAGGED]	= { .type = NLA_U8},
	[TSN_STREAMID_ATTR_NVID]	= { .type = NLA_U16},
	[TSN_STREAMID_ATTR_SMAC]	= { .type = NLA_U64},
	[TSN_STREAMID_ATTR_STAGGED]	= { .type = NLA_U8},
	[TSN_STREAMID_ATTR_SVID]	= { .type = NLA_U16},
	[TSN_STREAMID_ATTR_COUNTERS_PSI] = { .type = NLA_U64},
	[TSN_STREAMID_ATTR_COUNTERS_PSO] = { .type = NLA_U64},
	[TSN_STREAMID_ATTR_COUNTERS_PSPPI] = { .type = NLA_U64},
	[TSN_STREAMID_ATTR_COUNTERS_PSPPO] = { .type = NLA_U64},
};

static const struct nla_policy qci_sfi_policy[TSN_QCI_SFI_ATTR_MAX + 1] = {
	[TSN_QCI_SFI_ATTR_INDEX]		= { .type = NLA_U32},
	[TSN_QCI_SFI_ATTR_ENABLE]		= { .type = NLA_FLAG},
	[TSN_QCI_SFI_ATTR_DISABLE]		= { .type = NLA_FLAG},
	[TSN_QCI_SFI_ATTR_STREAM_HANDLE]	= { .type = NLA_S32},
	[TSN_QCI_SFI_ATTR_PRIO_SPEC]		= { .type = NLA_S8},
	[TSN_QCI_SFI_ATTR_GATE_ID]		= { .type = NLA_U32},
	[TSN_QCI_SFI_ATTR_FILTER_TYPE]		= { .type = NLA_U8},
	[TSN_QCI_SFI_ATTR_FLOW_ID]		= { .type = NLA_S32},
	[TSN_QCI_SFI_ATTR_MAXSDU]		= { .type = NLA_U16},
	[TSN_QCI_SFI_ATTR_COUNTERS]		= {
		.len = sizeof(struct tsn_qci_psfp_sfi_counters)},
	[TSN_QCI_SFI_ATTR_OVERSIZE_ENABLE]	= { .type = NLA_FLAG},
	[TSN_QCI_SFI_ATTR_OVERSIZE]		= { .type = NLA_FLAG},
};

static const struct nla_policy qci_sgi_policy[] = {
	[TSN_QCI_SGI_ATTR_INDEX]		= { .type = NLA_U32},
	[TSN_QCI_SGI_ATTR_ENABLE]		= { .type = NLA_FLAG},
	[TSN_QCI_SGI_ATTR_DISABLE]		= { .type = NLA_FLAG},
	[TSN_QCI_SGI_ATTR_CONFCHANGE]		= { .type = NLA_FLAG},
	[TSN_QCI_SGI_ATTR_IRXEN]		= { .type = NLA_FLAG},
	[TSN_QCI_SGI_ATTR_IRX]			= { .type = NLA_FLAG},
	[TSN_QCI_SGI_ATTR_OEXEN]		= { .type = NLA_FLAG},
	[TSN_QCI_SGI_ATTR_OEX]			= { .type = NLA_FLAG},
	[TSN_QCI_SGI_ATTR_ADMINENTRY]		= { .type = NLA_NESTED},
	[TSN_QCI_SGI_ATTR_OPERENTRY]		= { .type = NLA_NESTED},
	[TSN_QCI_SGI_ATTR_CCTIME]		= { .type = NLA_U64},
	[TSN_QCI_SGI_ATTR_TICKG]		= { .type = NLA_U32},
	[TSN_QCI_SGI_ATTR_CUTIME]		= { .type = NLA_U64},
	[TSN_QCI_SGI_ATTR_CPENDING]		= { .type = NLA_FLAG},
	[TSN_QCI_SGI_ATTR_CCERROR]		= { .type = NLA_U64},
};

static const struct nla_policy qci_sgi_ctrl_policy[] = {
	[TSN_SGI_ATTR_CTRL_INITSTATE]		= { .type = NLA_FLAG},
	[TSN_SGI_ATTR_CTRL_LEN]			= { .type = NLA_U8},
	[TSN_SGI_ATTR_CTRL_CYTIME]		= { .type = NLA_U32},
	[TSN_SGI_ATTR_CTRL_CYTIMEEX]		= { .type = NLA_U32},
	[TSN_SGI_ATTR_CTRL_BTIME]		= { .type = NLA_U64},
	[TSN_SGI_ATTR_CTRL_INITIPV]		= { .type = NLA_S8},
	[TSN_SGI_ATTR_CTRL_GCLENTRY]		= { .type = NLA_NESTED},
};

static const struct nla_policy qci_sgi_gcl_policy[] = {
	[TSN_SGI_ATTR_GCL_GATESTATE]		= { .type = NLA_FLAG},
	[TSN_SGI_ATTR_GCL_IPV]			= { .type = NLA_S8},
	[TSN_SGI_ATTR_GCL_INTERVAL]		= { .type = NLA_U32},
	[TSN_SGI_ATTR_GCL_OCTMAX]		= { .type = NLA_U32},
};

static const struct nla_policy qci_fmi_policy[] = {
	[TSN_QCI_FMI_ATTR_INDEX]	= { .type = NLA_U32},
	[TSN_QCI_FMI_ATTR_ENABLE]	= { .type = NLA_FLAG},
	[TSN_QCI_FMI_ATTR_DISABLE]	= { .type = NLA_FLAG},
	[TSN_QCI_FMI_ATTR_CIR]		= { .type = NLA_U32},
	[TSN_QCI_FMI_ATTR_CBS]		= { .type = NLA_U32},
	[TSN_QCI_FMI_ATTR_EIR]		= { .type = NLA_U32},
	[TSN_QCI_FMI_ATTR_EBS]		= { .type = NLA_U32},
	[TSN_QCI_FMI_ATTR_CF]		= { .type = NLA_FLAG},
	[TSN_QCI_FMI_ATTR_CM]		= { .type = NLA_FLAG},
	[TSN_QCI_FMI_ATTR_DROPYL]	= { .type = NLA_FLAG},
	[TSN_QCI_FMI_ATTR_MAREDEN]	= { .type = NLA_FLAG},
	[TSN_QCI_FMI_ATTR_MARED]	= { .type = NLA_FLAG},
	[TSN_QCI_FMI_ATTR_COUNTERS]	= {
		.len = sizeof(struct tsn_qci_psfp_fmi_counters)},
};

static const struct nla_policy dscp_policy[] = {
	[TSN_DSCP_ATTR_INDEX]		= { .type = NLA_U32},
	[TSN_DSCP_ATTR_DISABLE]		= { .type = NLA_FLAG},
	[TSN_DSCP_ATTR_COS]		= { .type = NLA_U8},
	[TSN_DSCP_ATTR_DPL]		= { .type = NLA_U8},
};

static ATOMIC_NOTIFIER_HEAD(tsn_notif_chain);

/**
 *	register_tsn_notifier - Register notifier
 *	@nb: notifier_block
 *
 *	Register switch device notifier.
 */
int register_tsn_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&tsn_notif_chain, nb);
}
EXPORT_SYMBOL_GPL(register_tsn_notifier);

/**
 *	unregister_tsn_notifier - Unregister notifier
 *	@nb: notifier_block
 *
 *	Unregister switch device notifier.
 */
int unregister_tsn_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(&tsn_notif_chain, nb);
}
EXPORT_SYMBOL_GPL(unregister_tsn_notifier);

/**
 *	call_tsn_notifiers - Call notifiers
 *	@val: value passed unmodified to notifier function
 *	@dev: port device
 *	@info: notifier information data
 *
 *	Call all network notifier blocks.
 */
int call_tsn_notifiers(unsigned long val, struct net_device *dev,
		       struct tsn_notifier_info *info)
{
	info->dev = dev;
	return atomic_notifier_call_chain(&tsn_notif_chain, val, info);
}
EXPORT_SYMBOL_GPL(call_tsn_notifiers);

struct tsn_port *tsn_get_port(struct net_device *ndev)
{
	struct tsn_port *port;
	bool tsn_found = false;

	list_for_each_entry(port, &port_list, list) {
		if (port->netdev == ndev) {
			tsn_found = true;
			break;
		}
	}

	if (!tsn_found)
		return NULL;

	return port;
}
EXPORT_SYMBOL_GPL(tsn_get_port);

static int tsn_prepare_reply(struct genl_info *info, u8 cmd,
			     struct sk_buff **skbp, size_t size)
{
	struct sk_buff *skb;
	void *reply;

	/* If new attributes are added, please revisit this allocation
	 */
	skb = genlmsg_new(size, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	if (!info) {
		nlmsg_free(skb);
		return -EINVAL;
	}

	reply = genlmsg_put_reply(skb, info, &tsn_family, 0, cmd);
	if (!reply) {
		nlmsg_free(skb);
		return -EINVAL;
	}

	*skbp = skb;
	return 0;
}

static int tsn_mk_reply(struct sk_buff *skb, int aggr, void *data, int len)
{
    /* add a netlink attribute to a socket buffer */
	return nla_put(skb, aggr, len, data);
}

static int tsn_send_reply(struct sk_buff *skb, struct genl_info *info)
{
	struct genlmsghdr *genlhdr = nlmsg_data(nlmsg_hdr(skb));
	void *reply = genlmsg_data(genlhdr);

	genlmsg_end(skb, reply);

	return genlmsg_reply(skb, info);
}

static int cmd_attr_echo_message(struct genl_info *info)
{
	struct nlattr *na;
	char *msg;
	struct sk_buff *rep_skb;
	size_t size;
	int ret;

	na = info->attrs[TSN_CMD_ATTR_MESG];
	if (!na)
		return -EINVAL;

	msg = (char *)nla_data(na);
	pr_info("tsn generic netlink receive echo mesg %s\n", msg);

	size = nla_total_size(strlen(msg) + 1);

	ret = tsn_prepare_reply(info, TSN_CMD_REPLY, &rep_skb,
				size + NLMSG_ALIGN(MAX_USER_SIZE));
	if (ret < 0)
		return ret;

	ret = tsn_mk_reply(rep_skb, TSN_CMD_ATTR_MESG, msg, size);
	if (ret < 0)
		goto err;

	return tsn_send_reply(rep_skb, info);

err:
	nlmsg_free(rep_skb);
	return ret;
}

static int cmd_attr_echo_data(struct genl_info *info)
{
	struct nlattr *na;
	s32	data;
	struct sk_buff *rep_skb;
	size_t size;
	int ret;

	/*read data */
	na = info->attrs[TSN_CMD_ATTR_DATA];
	if (!na)
		return -EINVAL;

	data = nla_get_s32(info->attrs[TSN_CMD_ATTR_DATA]);
	pr_info("tsn generic netlink receive echo data %d\n", data);

	/* send back */
	size = nla_total_size(sizeof(s32));

	ret = tsn_prepare_reply(info, TSN_CMD_REPLY, &rep_skb,
				size + NLMSG_ALIGN(MAX_USER_SIZE));
	if (ret < 0)
		return ret;

	/* netlink lib func */
	ret = nla_put_s32(rep_skb, TSN_CMD_ATTR_DATA, data);
	if (ret < 0)
		goto err;

	return tsn_send_reply(rep_skb, info);

err:
	nlmsg_free(rep_skb);
	return ret;
}

static int tsn_echo_cmd(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_CMD_ATTR_MESG])
		return cmd_attr_echo_message(info);
	else if (info->attrs[TSN_CMD_ATTR_DATA])
		return cmd_attr_echo_data(info);

	return -EINVAL;
}

static int tsn_simple_reply(struct genl_info *info, u32 cmd,
			    char *portname, s32 retvalue)
{
	struct sk_buff *rep_skb;
	size_t size;
	int ret;

	/* send back */
	size = nla_total_size(strlen(portname) + 1);
	size += nla_total_size(sizeof(s32));

	ret = tsn_prepare_reply(info, cmd,
				&rep_skb, size + NLMSG_ALIGN(MAX_USER_SIZE));
	if (ret < 0)
		return ret;

	/* netlink lib func */
	ret = nla_put_string(rep_skb, TSN_ATTR_IFNAME, portname);
	if (ret < 0)
		return ret;

	ret = nla_put_s32(rep_skb, TSN_CMD_ATTR_DATA, retvalue);
	if (ret < 0)
		return ret;

	return tsn_send_reply(rep_skb, info);
}

struct tsn_port *tsn_init_check(struct genl_info *info,
				struct net_device **ndev)
{
	struct nlattr *na;
	char *portname;
	struct net_device *netdev;
	struct tsn_port *port;
	bool tsn_found = false;

	if (!info->attrs[TSN_ATTR_IFNAME]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 "no portname", -EINVAL);
		return NULL;
	}

	na = info->attrs[TSN_ATTR_IFNAME];

	portname = (char *)nla_data(na);

	netdev = __dev_get_by_name(genl_info_net(info), portname);
	if (!netdev) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 "error device", -ENODEV);
		return NULL;
	}

	list_for_each_entry(port, &port_list, list) {
		if (port->netdev == netdev) {
			tsn_found = true;
			break;
		}
	}

	if (!tsn_found) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -ENODEV);
		return NULL;
	}

	*ndev = netdev;

	return port;
}

static int tsn_cap_get(struct sk_buff *skb, struct genl_info *info)
{
	struct sk_buff *rep_skb;
	struct nlattr *tsn_cap_attr;
	int ret;
	u32 cap = 0;
	struct net_device *netdev;
	struct genlmsghdr *genlhdr;
	const struct tsn_ops *tsnops;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port) {
		ret = -ENODEV;
		goto out;
	}

	tsnops = port->tsnops;
	genlhdr = info->genlhdr;
	if (!tsnops->get_capability) {
		ret = -EOPNOTSUPP;
		goto out;
	}

	cap = tsnops->get_capability(netdev);
	if (cap < 0) {
		ret = cap;
		goto out;
	}

	/* Pad netlink reply data */
	ret = tsn_prepare_reply(info, genlhdr->cmd,
				&rep_skb, NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		goto out;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name)) {
		ret = -EMSGSIZE;
		goto err;
	}

	tsn_cap_attr = nla_nest_start(rep_skb, TSN_ATTR_CAP);
	if (!tsn_cap_attr) {
		ret = -EMSGSIZE;
		goto err;
	}

	if (cap & TSN_CAP_QBV) {
		if (nla_put_flag(rep_skb, TSN_CAP_ATTR_QBV))
			goto err;
	}

	if (cap & TSN_CAP_QCI) {
		if (nla_put_flag(rep_skb, TSN_CAP_ATTR_QCI))
			goto err;
	}

	if (cap & TSN_CAP_QBU) {
		if (nla_put_flag(rep_skb, TSN_CAP_ATTR_QBU))
			goto err;
	}

	if (cap & TSN_CAP_CBS) {
		if (nla_put_flag(rep_skb, TSN_CAP_ATTR_CBS))
			goto err;
	}

	if (cap & TSN_CAP_CB) {
		if (nla_put_flag(rep_skb, TSN_CAP_ATTR_CB))
			goto err;
	}

	if (cap & TSN_CAP_TBS) {
		if (nla_put_flag(rep_skb, TSN_CAP_ATTR_TBS))
			goto err;
	}

	if (cap & TSN_CAP_CTH) {
		if (nla_put_flag(rep_skb, TSN_CAP_ATTR_CTH))
			goto err;
	}

	nla_nest_end(rep_skb, tsn_cap_attr);

	tsn_send_reply(rep_skb, info);
	return 0;
err:
	nlmsg_free(rep_skb);
out:
	if (ret < 0)
		tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, ret);
	return ret;
}

static int cmd_cb_streamid_set(struct genl_info *info)
{
	struct nlattr *na, *sid[TSN_STREAMID_ATTR_MAX + 1];
	u32 sid_index;
	u8 iden_type = 1;
	bool enable;
	int ret;
	struct net_device *netdev;
	struct tsn_cb_streamid sidconf;
	const struct tsn_ops *tsnops;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	memset(&sidconf, 0, sizeof(struct tsn_cb_streamid));

	if (!info->attrs[TSN_ATTR_STREAM_IDENTIFY])
		return -EINVAL;

	na = info->attrs[TSN_ATTR_STREAM_IDENTIFY];

	ret = NLA_PARSE_NESTED(sid, TSN_STREAMID_ATTR_MAX,
			       na, cb_streamid_policy);
	if (ret)
		return -EINVAL;

	if (!sid[TSN_STREAMID_ATTR_INDEX])
		return -EINVAL;

	sid_index = nla_get_u32(sid[TSN_STREAMID_ATTR_INDEX]);

	if (sid[TSN_STREAMID_ATTR_ENABLE])
		enable = true;
	else if (sid[TSN_STREAMID_ATTR_DISABLE])
		enable = false;
	else
		return -EINVAL;

	if (!enable)
		goto loaddev;

	if (sid[TSN_STREAMID_ATTR_TYPE])
		iden_type = nla_get_u8(sid[TSN_STREAMID_ATTR_TYPE]);
	else
		return -EINVAL;

	sidconf.type = iden_type;
	switch (iden_type) {
	case STREAMID_NULL:
		if (!sid[TSN_STREAMID_ATTR_NDMAC] ||
		    !sid[TSN_STREAMID_ATTR_NTAGGED] ||
		    !sid[TSN_STREAMID_ATTR_NVID]) {
			return -EINVAL;
		}

		sidconf.para.nid.dmac =
			nla_get_u64(sid[TSN_STREAMID_ATTR_NDMAC]);
		sidconf.para.nid.tagged =
			nla_get_u8(sid[TSN_STREAMID_ATTR_NTAGGED]);
		sidconf.para.nid.vid =
			nla_get_u16(sid[TSN_STREAMID_ATTR_NVID]);
		break;
	case STREAMID_SMAC_VLAN:
		/* TODO: not supportted yet */
		if (!sid[TSN_STREAMID_ATTR_SMAC] ||
		    !sid[TSN_STREAMID_ATTR_STAGGED] ||
		    !sid[TSN_STREAMID_ATTR_SVID]) {
			return -EINVAL;
		}

		sidconf.para.sid.smac =
			nla_get_u64(sid[TSN_STREAMID_ATTR_SMAC]);
		sidconf.para.sid.tagged =
			nla_get_u8(sid[TSN_STREAMID_ATTR_STAGGED]);
		sidconf.para.sid.vid =
			nla_get_u16(sid[TSN_STREAMID_ATTR_SVID]);
		break;
	case STREAMID_DMAC_VLAN:

	case STREAMID_IP:

	default:
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	if (sid[TSN_STREAMID_ATTR_STREAM_HANDLE])
		sidconf.handle =
			nla_get_s32(sid[TSN_STREAMID_ATTR_STREAM_HANDLE]);

	if (sid[TSN_STREAMID_ATTR_IFOP])
		sidconf.ifac_oport = nla_get_u32(sid[TSN_STREAMID_ATTR_IFOP]);
	if (sid[TSN_STREAMID_ATTR_OFOP])
		sidconf.ofac_oport = nla_get_u32(sid[TSN_STREAMID_ATTR_OFOP]);
	if (sid[TSN_STREAMID_ATTR_IFIP])
		sidconf.ifac_iport = nla_get_u32(sid[TSN_STREAMID_ATTR_IFIP]);
	if (sid[TSN_STREAMID_ATTR_OFIP])
		sidconf.ofac_iport = nla_get_u32(sid[TSN_STREAMID_ATTR_OFIP]);

loaddev:
	if (!tsnops->cb_streamid_set) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -EOPNOTSUPP;
	}

	ret = tsnops->cb_streamid_set(netdev, sid_index, enable, &sidconf);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, ret);
		return ret;
	}

	/* simple reply here. To be continue */
	if (tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, 0))
		return -1;

	return 0;
}

static int tsn_cb_streamid_set(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME]) {
		cmd_cb_streamid_set(info);
		return 0;
	}

	return -1;
}

static int cmd_cb_streamid_get(struct genl_info *info)
{
	struct nlattr *na, *sidattr, *sid[TSN_STREAMID_ATTR_MAX + 1];
	u32 sid_index;
	struct genlmsghdr *genlhdr;
	struct sk_buff *rep_skb;
	int ret, i;
	int valid;
	struct net_device *netdev;
	struct tsn_cb_streamid sidconf;
	struct tsn_cb_streamid_counters sidcounts;
	const struct tsn_ops *tsnops;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	memset(&sidconf, 0, sizeof(struct tsn_cb_streamid));
	memset(&sidcounts, 0, sizeof(struct tsn_cb_streamid_counters));

	if (!info->attrs[TSN_ATTR_STREAM_IDENTIFY])
		return -EINVAL;

	na = info->attrs[TSN_ATTR_STREAM_IDENTIFY];

	ret = NLA_PARSE_NESTED(sid, TSN_STREAMID_ATTR_MAX,
			       na, cb_streamid_policy);
	if (ret)
		return -EINVAL;

	if (!sid[TSN_STREAMID_ATTR_INDEX])
		return -EINVAL;

	sid_index = nla_get_u32(sid[TSN_STREAMID_ATTR_INDEX]);

	if (!tsnops->cb_streamid_get) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		ret = -EINVAL;
		goto exit;
	} else {
		valid = tsnops->cb_streamid_get(netdev, sid_index, &sidconf);
		if (valid < 0) {
			tsn_simple_reply(info, TSN_CMD_REPLY,
					 netdev->name, valid);
			return valid;
		}
	}

	/* send back */
	genlhdr = info->genlhdr;
	ret = tsn_prepare_reply(info, genlhdr->cmd, &rep_skb,
				NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		return ret;

	/* input netlink the parameters */
	sidattr = nla_nest_start(rep_skb, TSN_ATTR_STREAM_IDENTIFY);
	if (!sidattr) {
		ret = -EINVAL;
		goto err;
	}

	if (nla_put_u32(rep_skb, TSN_STREAMID_ATTR_INDEX, sid_index))
		return -EMSGSIZE;

	if (valid == 1) {
		nla_put_flag(rep_skb, TSN_STREAMID_ATTR_ENABLE);
	} else if (valid == 0) {
		nla_put_flag(rep_skb, TSN_STREAMID_ATTR_DISABLE);
	} else {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		goto err;
	}

	if (nla_put_s32(rep_skb,
			TSN_STREAMID_ATTR_STREAM_HANDLE, sidconf.handle) ||
	    nla_put_u32(rep_skb, TSN_STREAMID_ATTR_IFOP, sidconf.ifac_oport) ||
	    nla_put_u32(rep_skb, TSN_STREAMID_ATTR_OFOP, sidconf.ofac_oport) ||
	    nla_put_u32(rep_skb, TSN_STREAMID_ATTR_IFIP, sidconf.ifac_iport) ||
	    nla_put_u32(rep_skb, TSN_STREAMID_ATTR_OFIP, sidconf.ofac_iport) ||
	    nla_put_u8(rep_skb, TSN_STREAMID_ATTR_TYPE, sidconf.type))
		return -EMSGSIZE;

	switch (sidconf.type) {
	case STREAMID_NULL:
		if (NLA_PUT_U64(rep_skb, TSN_STREAMID_ATTR_NDMAC,
				sidconf.para.nid.dmac) ||
		    nla_put_u16(rep_skb, TSN_STREAMID_ATTR_NVID,
				sidconf.para.nid.vid) ||
		    nla_put_u8(rep_skb, TSN_STREAMID_ATTR_NTAGGED,
			       sidconf.para.nid.tagged))
			return -EMSGSIZE;
		break;
	case STREAMID_SMAC_VLAN:
		if (NLA_PUT_U64(rep_skb, TSN_STREAMID_ATTR_SMAC,
				sidconf.para.sid.smac) ||
		    nla_put_u16(rep_skb, TSN_STREAMID_ATTR_SVID,
				sidconf.para.sid.vid) ||
		    nla_put_u8(rep_skb, TSN_STREAMID_ATTR_STAGGED,
			       sidconf.para.sid.tagged))
			return -EMSGSIZE;
		break;
	case STREAMID_DMAC_VLAN:
	case STREAMID_IP:
	default:
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		goto err;
	}

	if (!tsnops->cb_streamid_counters_get) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		goto err;
	} else {
		ret = tsnops->cb_streamid_counters_get(netdev,
							sid_index,
							&sidcounts);
		if (ret < 0) {
			tsn_simple_reply(info, TSN_CMD_REPLY,
					 netdev->name, ret);
			goto err;
		}
	}

	if (NLA_PUT_U64(rep_skb, TSN_STREAMID_ATTR_COUNTERS_PSI,
			sidcounts.per_stream.input) ||
	    NLA_PUT_U64(rep_skb, TSN_STREAMID_ATTR_COUNTERS_PSO,
			sidcounts.per_stream.output))
		return -EMSGSIZE;

	for (i = 0; i < 32; i++) {
		if (NLA_PUT_U64(rep_skb, TSN_STREAMID_ATTR_COUNTERS_PSPPI,
				sidcounts.per_streamport[i].input) ||
		    NLA_PUT_U64(rep_skb, TSN_STREAMID_ATTR_COUNTERS_PSPPO,
				sidcounts.per_streamport[i].output))
			return -EMSGSIZE;
	}

	nla_nest_end(rep_skb, sidattr);
	/* end netlink input the parameters */

	/* netlink lib func */
	ret = nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name);
	if (ret < 0)
		goto err;

	ret = nla_put_s32(rep_skb, TSN_CMD_ATTR_DATA, 0);
	if (ret < 0)
		goto err;

	return tsn_send_reply(rep_skb, info);

err:
	nlmsg_free(rep_skb);
exit:
	return ret;
}

static int tsn_cb_streamid_get(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME]) {
		cmd_cb_streamid_get(info);
		return 0;
	}

	return -1;
}

static int cmb_cb_streamid_counters_get(struct genl_info *info)
{
	return 0;
}

static int tsn_cb_streamid_counters_get(struct sk_buff *skb,
					struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME]) {
		cmb_cb_streamid_counters_get(info);
		return 0;
	}

	return -1;
}

static int tsn_qci_cap_get(struct sk_buff *skb, struct genl_info *info)
{
	struct nlattr *qci_cap;
	struct sk_buff *rep_skb;
	int ret;
	struct net_device *netdev;
	struct genlmsghdr *genlhdr;
	struct tsn_qci_psfp_stream_param qci_cap_status;
	const struct tsn_ops *tsnops;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port) {
		ret = -EINVAL;
		goto out;
	}

	tsnops = port->tsnops;

	genlhdr = info->genlhdr;

	memset(&qci_cap_status, 0, sizeof(qci_cap_status));

	if (!tsnops->qci_get_maxcap) {
		ret = -EOPNOTSUPP;
		goto out;
	}

	ret = tsnops->qci_get_maxcap(netdev, &qci_cap_status);
	if (ret < 0)
		goto out;

	/* Pad netlink reply data */
	ret = tsn_prepare_reply(info, genlhdr->cmd,
				&rep_skb, NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		goto out;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name)) {
		ret = -EMSGSIZE;
		goto err;
	}

	qci_cap = nla_nest_start(rep_skb, TSN_ATTR_QCI_SP);
	if (!qci_cap) {
		ret = -EMSGSIZE;
		goto err;
	}

	if (nla_put_u32(rep_skb, TSN_QCI_STREAM_ATTR_MAX_SFI,
			qci_cap_status.max_sf_instance) ||
		nla_put_u32(rep_skb, TSN_QCI_STREAM_ATTR_MAX_SGI,
			    qci_cap_status.max_sg_instance) ||
		nla_put_u32(rep_skb, TSN_QCI_STREAM_ATTR_MAX_FMI,
			    qci_cap_status.max_fm_instance) ||
		nla_put_u32(rep_skb, TSN_QCI_STREAM_ATTR_SLM,
			    qci_cap_status.supported_list_max)) {
		ret = -EMSGSIZE;
		goto err;
	}

	nla_nest_end(rep_skb, qci_cap);

	tsn_send_reply(rep_skb, info);

	return 0;
err:
	nlmsg_free(rep_skb);
out:
	if (ret < 0)
		tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, ret);

	return ret;
}

static int cmd_qci_sfi_set(struct genl_info *info)
{
	struct nlattr *na, *sfi[TSN_QCI_SFI_ATTR_MAX + 1];
	u32 sfi_handle;
	bool enable;
	int ret;
	struct net_device *netdev;
	struct tsn_qci_psfp_sfi_conf sficonf;
	const struct tsn_ops *tsnops;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	memset(&sficonf, 0, sizeof(struct tsn_qci_psfp_sfi_conf));

	if (!info->attrs[TSN_ATTR_QCI_SFI])
		return -EINVAL;

	na = info->attrs[TSN_ATTR_QCI_SFI];

	ret = NLA_PARSE_NESTED(sfi, TSN_QCI_SFI_ATTR_MAX, na, qci_sfi_policy);
	if (ret) {
		pr_info("tsn: parse value TSN_QCI_SFI_ATTR_MAX  error.");
		return -EINVAL;
	}

	if (!sfi[TSN_QCI_SFI_ATTR_INDEX])
		return -EINVAL;

	sfi_handle = nla_get_u32(sfi[TSN_QCI_SFI_ATTR_INDEX]);

	if (sfi[TSN_QCI_SFI_ATTR_ENABLE]) {
		enable = true;
	} else if (sfi[TSN_QCI_SFI_ATTR_DISABLE]) {
		enable = false;
		goto loaddrive;
	} else {
		pr_err("tsn: must provde ENABLE or DISABLE attribute.\n");
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	if (!sfi[TSN_QCI_SFI_ATTR_GATE_ID]) {
		pr_err("tsn: must provide stream gate index\n");
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	if (!sfi[TSN_QCI_SFI_ATTR_STREAM_HANDLE])
		sficonf.stream_handle_spec = -1;
	else
		sficonf.stream_handle_spec =
			nla_get_s32(sfi[TSN_QCI_SFI_ATTR_STREAM_HANDLE]);

	if (!sfi[TSN_QCI_SFI_ATTR_PRIO_SPEC])
		sficonf.priority_spec = -1;
	else
		sficonf.priority_spec =
			nla_get_s8(sfi[TSN_QCI_SFI_ATTR_PRIO_SPEC]);

	sficonf.stream_gate_instance_id =
			nla_get_u32(sfi[TSN_QCI_SFI_ATTR_GATE_ID]);

	if (sfi[TSN_QCI_SFI_ATTR_MAXSDU])
		sficonf.stream_filter.maximum_sdu_size =
			nla_get_u16(sfi[TSN_QCI_SFI_ATTR_MAXSDU]);
	else
		sficonf.stream_filter.maximum_sdu_size = 0;

	if (sfi[TSN_QCI_SFI_ATTR_FLOW_ID])
		sficonf.stream_filter.flow_meter_instance_id =
			nla_get_s32(sfi[TSN_QCI_SFI_ATTR_FLOW_ID]);
	else
		sficonf.stream_filter.flow_meter_instance_id = -1;

	if (sfi[TSN_QCI_SFI_ATTR_OVERSIZE_ENABLE])
		sficonf.block_oversize_enable = true;

	if (sfi[TSN_QCI_SFI_ATTR_OVERSIZE])
		sficonf.block_oversize = true;

loaddrive:
	if (!tsnops->qci_sfi_set) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -EINVAL;
	}

	ret = tsnops->qci_sfi_set(netdev, sfi_handle, enable, &sficonf);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, ret);
		return ret;
	}

	ret = tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, 0);

	if (ret)
		return ret;
	return 0;
}

static int tsn_qci_sfi_set(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME]) {
		cmd_qci_sfi_set(info);
		return 0;
	}

	return -1;
}

static int cmd_qci_sfi_get(struct genl_info *info)
{
	struct nlattr *na, *sfiattr;
	struct nlattr *sfi[TSN_QCI_SFI_ATTR_MAX + 1];
	u32 sfi_handle;
	struct sk_buff *rep_skb;
	int ret, valid = 0;
	struct net_device *netdev;
	struct genlmsghdr *genlhdr;
	struct tsn_qci_psfp_sfi_conf sficonf;
	struct tsn_qci_psfp_sfi_counters sficount;
	const struct tsn_ops *tsnops;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	genlhdr = info->genlhdr;

	if (!info->attrs[TSN_ATTR_QCI_SFI])
		return -EINVAL;

	na = info->attrs[TSN_ATTR_QCI_SFI];

	ret = NLA_PARSE_NESTED(sfi, TSN_QCI_SFI_ATTR_MAX,
			       na, qci_sfi_policy);
	if (ret)
		return -EINVAL;

	if (!sfi[TSN_QCI_SFI_ATTR_INDEX])
		return -EINVAL;

	sfi_handle = nla_get_u32(sfi[TSN_QCI_SFI_ATTR_INDEX]);

	memset(&sficonf, 0, sizeof(struct tsn_qci_psfp_sfi_conf));
	memset(&sficount, 0, sizeof(struct tsn_qci_psfp_sfi_counters));

	if (!tsnops->qci_sfi_get || !tsnops->qci_sfi_counters_get) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		ret = -EINVAL;
		goto exit;
	} else {
		valid = tsnops->qci_sfi_get(netdev, sfi_handle, &sficonf);
		if (valid < 0) {
			tsn_simple_reply(info, TSN_CMD_REPLY,
					 netdev->name, valid);
			return valid;
		}

		valid = tsnops->qci_sfi_counters_get(netdev, sfi_handle,
						     &sficount);
		if (valid < 0) {
			tsn_simple_reply(info, TSN_CMD_REPLY,
					 netdev->name, valid);
			return valid;
		}
	}

	ret = tsn_prepare_reply(info, genlhdr->cmd,
				&rep_skb, NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		return ret;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name))
		goto err;

	sfiattr = nla_nest_start(rep_skb, TSN_ATTR_QCI_SFI);
	if (!sfiattr) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		ret = -EINVAL;
		goto err;
	}

	if (nla_put_u32(rep_skb, TSN_QCI_SFI_ATTR_INDEX, sfi_handle))
		return -EMSGSIZE;

	if (valid) {
		if (nla_put_flag(rep_skb, TSN_QCI_SFI_ATTR_ENABLE))
			return -EMSGSIZE;
	} else {
		if (nla_put_flag(rep_skb, TSN_QCI_SFI_ATTR_DISABLE))
			return -EMSGSIZE;
	}

	if (nla_put_s32(rep_skb, TSN_QCI_SFI_ATTR_STREAM_HANDLE,
			sficonf.stream_handle_spec) ||
	    nla_put_s8(rep_skb, TSN_QCI_SFI_ATTR_PRIO_SPEC,
		       sficonf.priority_spec) ||
	    nla_put_u32(rep_skb, TSN_QCI_SFI_ATTR_GATE_ID,
			sficonf.stream_gate_instance_id))
		return -EMSGSIZE;

	if (sficonf.stream_filter.maximum_sdu_size)
		if (nla_put_u16(rep_skb, TSN_QCI_SFI_ATTR_MAXSDU,
				sficonf.stream_filter.maximum_sdu_size))
			return -EMSGSIZE;

	if (sficonf.stream_filter.flow_meter_instance_id >= 0)
		if (nla_put_s32(rep_skb, TSN_QCI_SFI_ATTR_FLOW_ID,
				sficonf.stream_filter.flow_meter_instance_id))
			return -EMSGSIZE;

	if (sficonf.block_oversize_enable)
		if (nla_put_flag(rep_skb, TSN_QCI_SFI_ATTR_OVERSIZE_ENABLE))
			return -EMSGSIZE;
	if (sficonf.block_oversize)
		if (nla_put_flag(rep_skb, TSN_QCI_SFI_ATTR_OVERSIZE))
			return -EMSGSIZE;

	if (nla_put(rep_skb, TSN_QCI_SFI_ATTR_COUNTERS,
		    sizeof(struct tsn_qci_psfp_sfi_counters), &sficount))
		return -EMSGSIZE;

	nla_nest_end(rep_skb, sfiattr);

	return tsn_send_reply(rep_skb, info);
err:
	nlmsg_free(rep_skb);
	tsn_simple_reply(info, TSN_CMD_REPLY,
			 netdev->name, -EINVAL);
exit:
	return ret;
}

static int tsn_qci_sfi_get(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME]) {
		cmd_qci_sfi_get(info);
		return 0;
	}

	return -1;
}

static int cmd_qci_sfi_counters_get(struct genl_info *info)
{
	struct nlattr *na, *sfiattr;
	struct nlattr *sfi[TSN_QCI_SFI_ATTR_MAX + 1];
	u32 sfi_handle;
	struct sk_buff *rep_skb;
	int ret;
	struct net_device *netdev;
	struct genlmsghdr *genlhdr;
	struct tsn_qci_psfp_sfi_counters sficount;
	const struct tsn_ops *tsnops;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	genlhdr = info->genlhdr;

	if (!info->attrs[TSN_ATTR_QCI_SFI])
		return -EINVAL;

	na = info->attrs[TSN_ATTR_QCI_SFI];

	ret = NLA_PARSE_NESTED(sfi, TSN_QCI_SFI_ATTR_MAX,
			       na, qci_sfi_policy);
	if (ret)
		return -EINVAL;

	if (!sfi[TSN_QCI_SFI_ATTR_INDEX])
		return -EINVAL;

	sfi_handle = nla_get_u32(sfi[TSN_QCI_SFI_ATTR_INDEX]);

	memset(&sficount, 0, sizeof(struct tsn_qci_psfp_sfi_counters));
	if (!tsnops->qci_sfi_counters_get) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	ret = tsnops->qci_sfi_counters_get(netdev, sfi_handle, &sficount);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	ret = tsn_prepare_reply(info, genlhdr->cmd, &rep_skb,
				NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		return ret;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name))
		goto err;

	sfiattr = nla_nest_start(rep_skb, TSN_ATTR_QCI_SFI);
	if (!sfiattr) {
		ret = -EINVAL;
		goto err;
	}

	if (nla_put_u32(rep_skb, TSN_QCI_SFI_ATTR_INDEX, sfi_handle))
		return -EMSGSIZE;

	ret = tsnops->qci_sfi_counters_get(netdev, sfi_handle, &sficount);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, ret);
		return ret;
	}

	if (nla_put(rep_skb, TSN_QCI_SFI_ATTR_COUNTERS,
		    sizeof(struct tsn_qci_psfp_sfi_counters), &sficount))
		return -EMSGSIZE;

	nla_nest_end(rep_skb, sfiattr);

	return tsn_send_reply(rep_skb, info);
err:
	nlmsg_free(rep_skb);
	tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, -EINVAL);
	return ret;
}

static int tsn_qci_sfi_counters_get(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME]) {
		cmd_qci_sfi_counters_get(info);
		return 0;
	}

	return -1;
}

static int cmd_qci_sgi_set(struct genl_info *info)
{
	struct nlattr *na;
	struct nlattr *sgia[TSN_QCI_SGI_ATTR_MAX + 1];
	struct nlattr *admin[TSN_SGI_ATTR_CTRL_MAX + 1];
	int ret = 0;
	struct net_device *netdev;
	const struct tsn_ops *tsnops;
	struct tsn_qci_psfp_sgi_conf sgi;
	struct tsn_qci_psfp_gcl *gcl = NULL;
	u16 sgi_handle = 0;
	u16 listcount = 0;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	memset(&sgi, 0, sizeof(struct tsn_qci_psfp_sgi_conf));

	if (!info->attrs[TSN_ATTR_QCI_SGI]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	na = info->attrs[TSN_ATTR_QCI_SGI];

	ret = NLA_PARSE_NESTED(sgia, TSN_QCI_SGI_ATTR_MAX,
			       na, qci_sgi_policy);
	if (ret) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	if (sgia[TSN_QCI_SGI_ATTR_ENABLE] && sgia[TSN_QCI_SGI_ATTR_DISABLE]) {
		pr_err("tsn: enable or disable?\n");
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -1;
	}

	if (sgia[TSN_QCI_SGI_ATTR_INDEX])
		sgi_handle = nla_get_u32(sgia[TSN_QCI_SGI_ATTR_INDEX]);

	if (sgia[TSN_QCI_SGI_ATTR_DISABLE]) {
		sgi.gate_enabled = 0;
		goto loaddev;
	} else {
		/* set default to be enable*/
		sgi.gate_enabled = 1;
	}

	if (sgia[TSN_QCI_SGI_ATTR_CONFCHANGE])
		sgi.config_change = 1;

	if (sgia[TSN_QCI_SGI_ATTR_IRXEN])
		sgi.block_invalid_rx_enable = 1;

	if (sgia[TSN_QCI_SGI_ATTR_IRX])
		sgi.block_invalid_rx = 1;

	if (sgia[TSN_QCI_SGI_ATTR_OEXEN])
		sgi.block_octets_exceeded_enable = 1;

	if (sgia[TSN_QCI_SGI_ATTR_OEX])
		sgi.block_octets_exceeded = 1;

	if (sgia[TSN_QCI_SGI_ATTR_ADMINENTRY]) {
		struct nlattr *entry;
		int rem;
		int count = 0;

		na = sgia[TSN_QCI_SGI_ATTR_ADMINENTRY];
		ret = NLA_PARSE_NESTED(admin, TSN_SGI_ATTR_CTRL_MAX,
				       na, qci_sgi_ctrl_policy);

		/* Other parameters in admin control */
		if (admin[TSN_SGI_ATTR_CTRL_INITSTATE])
			sgi.admin.gate_states = 1;

		if (admin[TSN_SGI_ATTR_CTRL_CYTIME])
			sgi.admin.cycle_time =
				nla_get_u32(admin[TSN_SGI_ATTR_CTRL_CYTIME]);

		if (admin[TSN_SGI_ATTR_CTRL_CYTIMEEX])
			sgi.admin.cycle_time_extension =
				nla_get_u32(admin[TSN_SGI_ATTR_CTRL_CYTIMEEX]);

		if (admin[TSN_SGI_ATTR_CTRL_BTIME])
			sgi.admin.base_time =
				nla_get_u64(admin[TSN_SGI_ATTR_CTRL_BTIME]);

		if (admin[TSN_SGI_ATTR_CTRL_INITIPV])
			sgi.admin.init_ipv =
				nla_get_s8(admin[TSN_SGI_ATTR_CTRL_INITIPV]);
		else
			sgi.admin.init_ipv = -1;

		if (admin[TSN_SGI_ATTR_CTRL_LEN]) {
			sgi.admin.control_list_length =
				nla_get_u8(admin[TSN_SGI_ATTR_CTRL_LEN]);
			listcount = sgi.admin.control_list_length;
		}

		if (!listcount)
			goto loaddev;

		gcl = kmalloc_array(listcount, sizeof(*gcl), GFP_KERNEL);

		memset(gcl, 0, listcount * sizeof(struct tsn_qci_psfp_gcl));

		/* Check the whole admin attrs,
		 * checkout the TSN_SGI_ATTR_CTRL_GCLENTRY attributes
		 */
		nla_for_each_nested(entry, na, rem) {
			struct nlattr *gcl_entry[TSN_SGI_ATTR_GCL_MAX + 1];
			struct nlattr *ti, *om;

			if (nla_type(entry) != TSN_SGI_ATTR_CTRL_GCLENTRY)
				continue;

			/* parse each TSN_SGI_ATTR_CTRL_GCLENTRY */
			ret = NLA_PARSE_NESTED(gcl_entry, TSN_SGI_ATTR_GCL_MAX,
					       entry, qci_sgi_gcl_policy);
			/* Parse gate control list */
			if (gcl_entry[TSN_SGI_ATTR_GCL_GATESTATE])
				(gcl + count)->gate_state = 1;

			if (gcl_entry[TSN_SGI_ATTR_GCL_IPV])
				(gcl + count)->ipv =
				 nla_get_s8(gcl_entry[TSN_SGI_ATTR_GCL_IPV]);

			if (gcl_entry[TSN_SGI_ATTR_GCL_INTERVAL]) {
				ti = gcl_entry[TSN_SGI_ATTR_GCL_INTERVAL];
				(gcl + count)->time_interval = nla_get_u32(ti);
			}

			if (gcl_entry[TSN_SGI_ATTR_GCL_OCTMAX]) {
				om = gcl_entry[TSN_SGI_ATTR_GCL_OCTMAX];
				(gcl + count)->octet_max = nla_get_u32(om);
			}

			count++;

			if (count >= listcount)
				break;
		}

		if (count < listcount) {
			tsn_simple_reply(info, TSN_CMD_REPLY,
					 netdev->name, -EINVAL);
			pr_err("tsn: count less than TSN_SGI_ATTR_CTRL_LEN\n");
			kfree(gcl);
			return -EINVAL;
		}

	} else {
		pr_info("tsn: no admin list parameters setting\n");
	}

loaddev:
	if (!tsnops->qci_sgi_set) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		kfree(gcl);
		return -EINVAL;
	}

	sgi.admin.gcl = gcl;

	ret = tsnops->qci_sgi_set(netdev, sgi_handle, &sgi);
	kfree(gcl);
	if (!ret)
		return tsn_simple_reply(info, TSN_CMD_REPLY,
					netdev->name, 0);

	tsn_simple_reply(info, TSN_CMD_REPLY,
			 netdev->name, ret);
	return ret;
}

static int tsn_qci_sgi_set(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME]) {
		cmd_qci_sgi_set(info);
		return 0;
	}

	return -1;
}

static int cmd_qci_sgi_get(struct genl_info *info)
{
	struct nlattr *na, *sgiattr, *adminattr, *sglattr;
	struct nlattr *sgi[TSN_QCI_SGI_ATTR_MAX + 1];
	struct sk_buff *rep_skb;
	int ret;
	struct net_device *netdev;
	struct genlmsghdr *genlhdr;
	struct tsn_qci_psfp_sgi_conf sgiadmin;
	struct tsn_qci_psfp_gcl *gcl = NULL;
	const struct tsn_ops *tsnops;
	u16 sgi_handle;
	u8 listcount, i;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	if (!info->attrs[TSN_ATTR_QCI_SGI]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		pr_err("tsn: no sgi handle input\n");
		return -EINVAL;
	}

	na = info->attrs[TSN_ATTR_QCI_SGI];

	ret = NLA_PARSE_NESTED(sgi, TSN_QCI_SGI_ATTR_MAX,
			       na, qci_sgi_policy);
	if (ret)
		return -EINVAL;

	if (!sgi[TSN_QCI_SGI_ATTR_INDEX]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		pr_err("tsn: no sgi handle input\n");
		return -EINVAL;
	}

	sgi_handle = nla_get_u32(sgi[TSN_QCI_SGI_ATTR_INDEX]);

	/* Get config data from device */
	genlhdr = info->genlhdr;

	memset(&sgiadmin, 0, sizeof(struct tsn_qci_psfp_sgi_conf));

	if (!tsnops->qci_sgi_get) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	ret = tsnops->qci_sgi_get(netdev, sgi_handle, &sgiadmin);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	/* Form netlink reply data */
	ret = tsn_prepare_reply(info, genlhdr->cmd,
				&rep_skb, NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		return ret;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name))
		return -EMSGSIZE;

	sgiattr = nla_nest_start(rep_skb, TSN_ATTR_QCI_SGI);
	if (!sgiattr)
		return -EMSGSIZE;

	if (nla_put_u32(rep_skb, TSN_QCI_SGI_ATTR_INDEX, sgi_handle))
		return -EMSGSIZE;

	/* Gate enable? sgiadmin.gate_enabled */
	if (sgiadmin.gate_enabled) {
		if (nla_put_flag(rep_skb, TSN_QCI_SGI_ATTR_ENABLE))
			return -EMSGSIZE;
	} else {
		if (nla_put_flag(rep_skb, TSN_QCI_SGI_ATTR_DISABLE))
			return -EMSGSIZE;
	}

	if (sgiadmin.config_change)
		if (nla_put_flag(rep_skb, TSN_QCI_SGI_ATTR_CONFCHANGE))
			return -EMSGSIZE;

	if (sgiadmin.block_invalid_rx_enable)
		if (nla_put_flag(rep_skb, TSN_QCI_SGI_ATTR_IRXEN))
			return -EMSGSIZE;

	if (sgiadmin.block_invalid_rx)
		if (nla_put_flag(rep_skb, TSN_QCI_SGI_ATTR_IRX))
			return -EMSGSIZE;

	if (sgiadmin.block_octets_exceeded_enable)
		if (nla_put_flag(rep_skb, TSN_QCI_SGI_ATTR_OEXEN))
			return -EMSGSIZE;

	if (sgiadmin.block_octets_exceeded)
		if (nla_put_flag(rep_skb, TSN_QCI_SGI_ATTR_OEX))
			return -EMSGSIZE;

	/* Administration */
	adminattr = nla_nest_start(rep_skb, TSN_QCI_SGI_ATTR_ADMINENTRY);
	if (!adminattr)
		return -EMSGSIZE;

	if (sgiadmin.admin.gate_states)
		if (nla_put_flag(rep_skb, TSN_SGI_ATTR_CTRL_INITSTATE))
			return -EMSGSIZE;

	if (nla_put_u32(rep_skb, TSN_SGI_ATTR_CTRL_CYTIME,
			sgiadmin.admin.cycle_time) ||
	    nla_put_u32(rep_skb, TSN_SGI_ATTR_CTRL_CYTIMEEX,
			sgiadmin.admin.cycle_time_extension) ||
	    NLA_PUT_U64(rep_skb, TSN_SGI_ATTR_CTRL_BTIME,
			sgiadmin.admin.base_time) ||
	    nla_put_u8(rep_skb, TSN_SGI_ATTR_CTRL_INITIPV,
		       sgiadmin.admin.init_ipv))
		return -EMSGSIZE;

	listcount = sgiadmin.admin.control_list_length;
	if (!listcount)
		goto out1;

	if (!sgiadmin.admin.gcl) {
		pr_err("error: no gate control list\n");
		ret = -EINVAL;
		goto err;
	}

	gcl = sgiadmin.admin.gcl;

	/* loop list */
	for (i = 0; i < listcount; i++) {
		s8 ipv;
		u32 ti, omax;

		if (!(gcl + i)) {
			pr_err("error: list count too big\n");
			ret = -EINVAL;
			kfree(sgiadmin.admin.gcl);
			goto err;
		}

		/* Adminastration entry */
		sglattr = nla_nest_start(rep_skb,
						TSN_SGI_ATTR_CTRL_GCLENTRY);
		if (!sglattr)
			return -EMSGSIZE;
		ipv = (gcl + i)->ipv;
		ti = (gcl + i)->time_interval;
		omax = (gcl + i)->octet_max;

		if ((gcl + i)->gate_state)
			if (nla_put_flag(rep_skb, TSN_SGI_ATTR_GCL_GATESTATE))
				return -EMSGSIZE;

		if (nla_put_s8(rep_skb, TSN_SGI_ATTR_GCL_IPV, ipv) ||
		    nla_put_u32(rep_skb, TSN_SGI_ATTR_GCL_INTERVAL, ti) ||
		    nla_put_u32(rep_skb, TSN_SGI_ATTR_GCL_OCTMAX, omax))
			return -EMSGSIZE;

		/* End administration entry */
		nla_nest_end(rep_skb, sglattr);
	}

	kfree(sgiadmin.admin.gcl);
	if (nla_put_u8(rep_skb, TSN_SGI_ATTR_CTRL_LEN, listcount))
		return -EMSGSIZE;

out1:
	/* End adminastration */
	nla_nest_end(rep_skb, adminattr);

	nla_nest_end(rep_skb, sgiattr);

	return tsn_send_reply(rep_skb, info);
err:
	nlmsg_free(rep_skb);
	tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, ret);
	return ret;
}

static int tsn_qci_sgi_get(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME]) {
		cmd_qci_sgi_get(info);
		return 0;
	}

	return -1;
}

static int cmd_qci_sgi_status_get(struct genl_info *info)
{
	struct nlattr *na, *sgiattr, *operattr, *sglattr;
	struct nlattr *sgi[TSN_QCI_SGI_ATTR_MAX + 1];
	struct sk_buff *rep_skb;
	int ret;
	struct net_device *netdev;
	struct genlmsghdr *genlhdr;
	struct tsn_psfp_sgi_status sgistat;
	struct tsn_qci_psfp_gcl *gcl = NULL;
	const struct tsn_ops *tsnops;
	u16 sgi_handle;
	u8 listcount;
	int valid, i;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	if (!info->attrs[TSN_ATTR_QCI_SGI]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		pr_err("tsn: no sgi handle input\n");
		return -EINVAL;
	}

	na = info->attrs[TSN_ATTR_QCI_SGI];

	ret = NLA_PARSE_NESTED(sgi, TSN_QCI_SGI_ATTR_MAX,
			       na, qci_sgi_policy);
	if (ret)
		return -EINVAL;

	if (!sgi[TSN_QCI_SGI_ATTR_INDEX]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		pr_err("tsn: no sgi handle input\n");
		return -EINVAL;
	}

	sgi_handle = nla_get_u32(sgi[TSN_QCI_SGI_ATTR_INDEX]);

	/* Get status data from device */
	genlhdr = info->genlhdr;

	memset(&sgistat, 0, sizeof(struct tsn_psfp_sgi_status));

	if (!tsnops->qci_sgi_status_get) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	valid = tsnops->qci_sgi_status_get(netdev, sgi_handle, &sgistat);
	if (valid < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, valid);
		return valid;
	}

	/* Form netlink reply data */
	ret = tsn_prepare_reply(info, genlhdr->cmd,
				&rep_skb, NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		return ret;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name))
		return -EMSGSIZE;

	/* Down one netlink attribute level */
	sgiattr = nla_nest_start(rep_skb, TSN_ATTR_QCI_SGI);
	if (!sgiattr)
		return -EMSGSIZE;

	if (nla_put_u32(rep_skb, TSN_QCI_SGI_ATTR_INDEX, sgi_handle))
		return -EMSGSIZE;

	/* Gate enable */
	if (valid == 1) {
		if (nla_put_flag(rep_skb, TSN_QCI_SGI_ATTR_ENABLE))
			return -EMSGSIZE;
	} else {
		if (nla_put_flag(rep_skb, TSN_QCI_SGI_ATTR_DISABLE))
			return -EMSGSIZE;
	}

	if (nla_put_u32(rep_skb, TSN_QCI_SGI_ATTR_TICKG,
			sgistat.tick_granularity) ||
	    NLA_PUT_U64(rep_skb, TSN_QCI_SGI_ATTR_CCTIME,
			sgistat.config_change_time) ||
	    NLA_PUT_U64(rep_skb, TSN_QCI_SGI_ATTR_CUTIME,
			sgistat.current_time) ||
	    NLA_PUT_U64(rep_skb, TSN_QCI_SGI_ATTR_CCERROR,
			sgistat.config_change_error))
		return -EMSGSIZE;

	if (sgistat.config_pending)
		if (nla_put_flag(rep_skb, TSN_QCI_SGI_ATTR_CPENDING))
			return -EMSGSIZE;

	/* operation data */
	operattr = nla_nest_start(rep_skb, TSN_QCI_SGI_ATTR_OPERENTRY);
	if (!operattr)
		return -EMSGSIZE;

	if (sgistat.oper.gate_states)
		if (nla_put_flag(rep_skb, TSN_SGI_ATTR_CTRL_INITSTATE))
			return -EMSGSIZE;

	if (nla_put_u32(rep_skb, TSN_SGI_ATTR_CTRL_CYTIME,
			sgistat.oper.cycle_time) ||
	    nla_put_u32(rep_skb, TSN_SGI_ATTR_CTRL_CYTIMEEX,
			sgistat.oper.cycle_time_extension) ||
	    NLA_PUT_U64(rep_skb, TSN_SGI_ATTR_CTRL_BTIME,
			sgistat.oper.base_time) ||
	    nla_put_u8(rep_skb, TSN_SGI_ATTR_CTRL_INITIPV,
		       sgistat.oper.init_ipv))
		return -EMSGSIZE;

	/* Loop list */
	listcount = sgistat.oper.control_list_length;
	if (!listcount)
		goto out1;

	if (!sgistat.oper.gcl) {
		pr_err("error: list lenghth is not zero!\n");
		ret = -EINVAL;
		goto err;
	}

	gcl = sgistat.oper.gcl;

	/* loop list */
	for (i = 0; i < listcount; i++) {
		s8 ipv;
		u32 ti, omax;

		if (!(gcl + i)) {
			pr_err("error: list count too big\n");
			ret = -EINVAL;
			kfree(sgistat.oper.gcl);
			goto err;
		}

		/* Operation entry */
		sglattr = nla_nest_start(rep_skb,
						TSN_SGI_ATTR_CTRL_GCLENTRY);
		if (!sglattr)
			return -EMSGSIZE;
		ipv = (gcl + i)->ipv;
		ti = (gcl + i)->time_interval;
		omax = (gcl + i)->octet_max;

		if ((gcl + i)->gate_state)
			if (nla_put_flag(rep_skb, TSN_SGI_ATTR_GCL_GATESTATE))
				return -EMSGSIZE;

		if (nla_put_s8(rep_skb, TSN_SGI_ATTR_GCL_IPV, ipv) ||
		    nla_put_u32(rep_skb, TSN_SGI_ATTR_GCL_INTERVAL, ti) ||
		    nla_put_u32(rep_skb, TSN_SGI_ATTR_GCL_OCTMAX, omax))
			return -EMSGSIZE;

		/* End operation entry */
		nla_nest_end(rep_skb, sglattr);
	}

	kfree(sgistat.oper.gcl);
	if (nla_put_u8(rep_skb, TSN_SGI_ATTR_CTRL_LEN, listcount))
		return -EMSGSIZE;
out1:
	/* End operation */
	nla_nest_end(rep_skb, operattr);

	nla_nest_end(rep_skb, sgiattr);

	return tsn_send_reply(rep_skb, info);
err:
	nlmsg_free(rep_skb);
	tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, ret);
	return ret;
}

static int tsn_qci_sgi_status_get(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME]) {
		cmd_qci_sgi_status_get(info);
		return 0;
	}

	return -1;
}

static int cmd_qci_fmi_set(struct genl_info *info)
{
	struct nlattr *na, *fmi[TSN_QCI_FMI_ATTR_MAX + 1];
	u32 index;
	int ret;
	struct net_device *netdev;
	struct tsn_qci_psfp_fmi fmiconf;
	const struct tsn_ops *tsnops;
	bool enable = 0;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	memset(&fmiconf, 0, sizeof(struct tsn_qci_psfp_fmi));

	if (!info->attrs[TSN_ATTR_QCI_FMI])
		return -EINVAL;

	na = info->attrs[TSN_ATTR_QCI_FMI];

	ret = NLA_PARSE_NESTED(fmi, TSN_QCI_FMI_ATTR_MAX, na, qci_fmi_policy);
	if (ret) {
		pr_info("tsn: parse value TSN_QCI_FMI_ATTR_MAX  error.");
		return -EINVAL;
	}

	if (!fmi[TSN_QCI_FMI_ATTR_INDEX])
		return -EINVAL;

	index = nla_get_u32(fmi[TSN_QCI_FMI_ATTR_INDEX]);

	if (fmi[TSN_QCI_FMI_ATTR_DISABLE])
		goto loaddev;

	enable = 1;

	if (fmi[TSN_QCI_FMI_ATTR_CIR])
		fmiconf.cir = nla_get_u32(fmi[TSN_QCI_FMI_ATTR_CIR]);

	if (fmi[TSN_QCI_FMI_ATTR_CBS])
		fmiconf.cbs = nla_get_u32(fmi[TSN_QCI_FMI_ATTR_CBS]);

	if (fmi[TSN_QCI_FMI_ATTR_EIR])
		fmiconf.eir = nla_get_u32(fmi[TSN_QCI_FMI_ATTR_EIR]);

	if (fmi[TSN_QCI_FMI_ATTR_EBS])
		fmiconf.ebs = nla_get_u32(fmi[TSN_QCI_FMI_ATTR_EBS]);

	if (fmi[TSN_QCI_FMI_ATTR_CF])
		fmiconf.cf = 1;

	if (fmi[TSN_QCI_FMI_ATTR_CM])
		fmiconf.cm = 1;

	if (fmi[TSN_QCI_FMI_ATTR_DROPYL])
		fmiconf.drop_on_yellow = 1;

	if (fmi[TSN_QCI_FMI_ATTR_MAREDEN])
		fmiconf.mark_red_enable = 1;

	if (fmi[TSN_QCI_FMI_ATTR_MARED])
		fmiconf.mark_red = 1;

loaddev:

	if (!tsnops->qci_fmi_set) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -EINVAL;
	}

	ret = tsnops->qci_fmi_set(netdev, index, enable, &fmiconf);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, ret);
		return ret;
	}

	ret = tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, 0);

	if (ret)
		return ret;
	return 0;
}

static int tsn_qci_fmi_set(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME]) {
		cmd_qci_fmi_set(info);
		return 0;
	}

	return -1;
}

static int cmd_qci_fmi_get(struct genl_info *info)
{
	struct nlattr *na, *fmi[TSN_QCI_FMI_ATTR_MAX + 1], *fmiattr;
	u32 index;
	struct sk_buff *rep_skb;
	int ret;
	struct net_device *netdev;
	struct tsn_qci_psfp_fmi fmiconf;
	struct tsn_qci_psfp_fmi_counters counters;
	const struct tsn_ops *tsnops;
	struct genlmsghdr *genlhdr;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	if (!info->attrs[TSN_ATTR_QCI_FMI])
		return -EINVAL;

	na = info->attrs[TSN_ATTR_QCI_FMI];

	ret = NLA_PARSE_NESTED(fmi, TSN_QCI_FMI_ATTR_MAX,
			       na, qci_fmi_policy);
	if (ret) {
		pr_info("tsn: parse value TSN_QCI_FMI_ATTR_MAX  error.");
		return -EINVAL;
	}

	if (!fmi[TSN_QCI_FMI_ATTR_INDEX])
		return -EINVAL;

	index = nla_get_u32(fmi[TSN_QCI_FMI_ATTR_INDEX]);

	/* Get data from device */
	memset(&fmiconf, 0, sizeof(struct tsn_qci_psfp_fmi));
	memset(&counters, 0, sizeof(struct tsn_qci_psfp_fmi_counters));

	if (!tsnops->qci_fmi_get) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -EINVAL;
	}

	ret = tsnops->qci_fmi_get(netdev, index, &fmiconf, &counters);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY, netdev->name, ret);
		return ret;
	}

	genlhdr = info->genlhdr;

	/* Form netlink reply data */
	ret = tsn_prepare_reply(info, genlhdr->cmd,
				&rep_skb, NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		return ret;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name))
		return -EMSGSIZE;

	fmiattr = nla_nest_start(rep_skb, TSN_ATTR_QCI_FMI);
	if (!fmiattr)
		return -EMSGSIZE;

	if (nla_put_u32(rep_skb, TSN_QCI_FMI_ATTR_INDEX, index) ||
	    nla_put_u32(rep_skb, TSN_QCI_FMI_ATTR_CIR, fmiconf.cir) ||
	    nla_put_u32(rep_skb, TSN_QCI_FMI_ATTR_CBS, fmiconf.cbs) ||
	    nla_put_u32(rep_skb, TSN_QCI_FMI_ATTR_EIR, fmiconf.eir) ||
	    nla_put_u32(rep_skb, TSN_QCI_FMI_ATTR_EBS, fmiconf.ebs))
		return -EMSGSIZE;

	if (fmiconf.cf)
		if (nla_put_flag(rep_skb, TSN_QCI_FMI_ATTR_CF))
			return -EMSGSIZE;

	if (fmiconf.cm)
		if (nla_put_flag(rep_skb, TSN_QCI_FMI_ATTR_CM))
			return -EMSGSIZE;

	if (fmiconf.drop_on_yellow)
		if (nla_put_flag(rep_skb, TSN_QCI_FMI_ATTR_DROPYL))
			return -EMSGSIZE;

	if (fmiconf.mark_red_enable)
		if (nla_put_flag(rep_skb, TSN_QCI_FMI_ATTR_MAREDEN))
			return -EMSGSIZE;

	if (fmiconf.mark_red)
		if (nla_put_flag(rep_skb, TSN_QCI_FMI_ATTR_MAREDEN))
			return -EMSGSIZE;

	if (nla_put(rep_skb, TSN_QCI_FMI_ATTR_COUNTERS,
		    sizeof(struct tsn_qci_psfp_fmi_counters), &counters))
		return -EMSGSIZE;

	nla_nest_end(rep_skb, fmiattr);

	tsn_send_reply(rep_skb, info);

	return 0;
}

static int tsn_qci_fmi_get(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME]) {
		cmd_qci_fmi_get(info);
		return 0;
	}

	return -1;
}

static int cmd_qbv_set(struct genl_info *info)
{
	struct nlattr *na, *na1;
	struct nlattr *qbv_table;
	struct nlattr *qbv[TSN_QBV_ATTR_MAX + 1];
	struct nlattr *qbvctrl[TSN_QBV_ATTR_CTRL_MAX + 1];
	int rem;
	int ret = 0;
	struct net_device *netdev;
	struct tsn_qbv_conf qbvconfig;
	const struct tsn_ops *tsnops;
	struct tsn_qbv_entry *gatelist = NULL;
	int count = 0;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	memset(&qbvconfig, 0, sizeof(struct tsn_qbv_conf));

	if (!info->attrs[TSN_ATTR_QBV])
		return -EINVAL;

	na = info->attrs[TSN_ATTR_QBV];

	ret = NLA_PARSE_NESTED(qbv, TSN_QBV_ATTR_MAX, na, qbv_policy);
	if (ret)
		return -EINVAL;

	if (qbv[TSN_QBV_ATTR_ENABLE])
		qbvconfig.gate_enabled = 1;
	else
		goto setdrive;

	if (qbv[TSN_QBV_ATTR_CONFIGCHANGE])
		qbvconfig.config_change = 1;

	if (!qbv[TSN_QBV_ATTR_ADMINENTRY]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -1;
	}

	na1 = qbv[TSN_QBV_ATTR_ADMINENTRY];
	NLA_PARSE_NESTED(qbvctrl, TSN_QBV_ATTR_CTRL_MAX,
			 na1, qbv_ctrl_policy);

	if (qbvctrl[TSN_QBV_ATTR_CTRL_CYCLETIME]) {
		qbvconfig.admin.cycle_time =
			nla_get_u32(qbvctrl[TSN_QBV_ATTR_CTRL_CYCLETIME]);
	}

	if (qbvctrl[TSN_QBV_ATTR_CTRL_CYCLETIMEEXT]) {
		qbvconfig.admin.cycle_time_extension =
			nla_get_u32(qbvctrl[TSN_QBV_ATTR_CTRL_CYCLETIMEEXT]);
	}

	if (qbvctrl[TSN_QBV_ATTR_CTRL_BASETIME]) {
		qbvconfig.admin.base_time =
			nla_get_u64(qbvctrl[TSN_QBV_ATTR_CTRL_BASETIME]);
	}

	if (qbvctrl[TSN_QBV_ATTR_CTRL_GATESTATE]) {
		qbvconfig.admin.gate_states =
			nla_get_u8(qbvctrl[TSN_QBV_ATTR_CTRL_GATESTATE]);
	}

	if (qbvctrl[TSN_QBV_ATTR_CTRL_LISTCOUNT]) {
		int listcount;

		listcount = nla_get_u32(qbvctrl[TSN_QBV_ATTR_CTRL_LISTCOUNT]);

		qbvconfig.admin.control_list_length = listcount;

		gatelist = kmalloc_array(listcount,
					 sizeof(*gatelist),
					 GFP_KERNEL);

		nla_for_each_nested(qbv_table, na1, rem) {
			struct nlattr *qbv_entry[TSN_QBV_ATTR_ENTRY_MAX + 1];

			if (nla_type(qbv_table) != TSN_QBV_ATTR_CTRL_LISTENTRY)
				continue;

			ret = NLA_PARSE_NESTED(qbv_entry,
					       TSN_QBV_ATTR_ENTRY_MAX,
					       qbv_table, qbv_entry_policy);
			if (ret)
				return -EINVAL;

			(gatelist + count)->gate_state =
				nla_get_u8(qbv_entry[TSN_QBV_ATTR_ENTRY_GC]);
			(gatelist + count)->time_interval =
				nla_get_u32(qbv_entry[TSN_QBV_ATTR_ENTRY_TM]);
			count++;
			if (count > listcount)
				break;
		}
	}

	if (gatelist)
		qbvconfig.admin.control_list = gatelist;

setdrive:
	if (!tsnops->qbv_set) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		goto err;
	}

	ret = tsnops->qbv_set(netdev, &qbvconfig);

	/* send back */
	if (ret < 0)
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
	else
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, 0);

err:
	kfree(gatelist);
	return ret;
}

static int tsn_qbv_set(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME]) {
		cmd_qbv_set(info);
		return 0;
	}

	return -1;
}

static int cmd_qbv_get(struct genl_info *info)
{
	struct nlattr *qbv, *qbvadminattr;
	struct sk_buff *rep_skb;
	int ret;
	int len = 0, i = 0;
	struct net_device *netdev;
	struct genlmsghdr *genlhdr;
	struct tsn_qbv_conf qbvconf;
	const struct tsn_ops *tsnops;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	genlhdr = info->genlhdr;

	memset(&qbvconf, 0, sizeof(struct tsn_qbv_conf));

	if (!tsnops->qbv_get) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	ret = tsnops->qbv_get(netdev, &qbvconf);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	ret = tsn_prepare_reply(info, genlhdr->cmd,
				&rep_skb, NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		return ret;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name))
		return -EMSGSIZE;

	qbv = nla_nest_start(rep_skb, TSN_ATTR_QBV);
	if (!qbv)
		return -EMSGSIZE;

	qbvadminattr = nla_nest_start(rep_skb, TSN_QBV_ATTR_ADMINENTRY);
	if (!qbvadminattr)
		return -EMSGSIZE;

	if (qbvconf.admin.control_list) {
		len = qbvconf.admin.control_list_length;
		if (nla_put_u32(rep_skb, TSN_QBV_ATTR_CTRL_LISTCOUNT, len))
			return -EMSGSIZE;

		for (i = 0; i < len; i++) {
			struct nlattr *qbv_table;
			u8 gs;
			u32 tp;
			int glisttype = TSN_QBV_ATTR_CTRL_LISTENTRY;

			gs = (qbvconf.admin.control_list + i)->gate_state;
			tp = (qbvconf.admin.control_list + i)->time_interval;

			qbv_table =
				nla_nest_start(rep_skb, glisttype);
			if (!qbv_table)
				return -EMSGSIZE;

			if (nla_put_u32(rep_skb, TSN_QBV_ATTR_ENTRY_ID, i) ||
			    nla_put_u8(rep_skb, TSN_QBV_ATTR_ENTRY_GC, gs) ||
			    nla_put_u32(rep_skb, TSN_QBV_ATTR_ENTRY_TM, tp))
				return -EMSGSIZE;
			nla_nest_end(rep_skb, qbv_table);
		}

		if (qbvconf.admin.gate_states)
			if (nla_put_u8(rep_skb, TSN_QBV_ATTR_CTRL_GATESTATE,
				       qbvconf.admin.gate_states))
				return -EMSGSIZE;

		if (qbvconf.admin.cycle_time)
			if (nla_put_u32(rep_skb, TSN_QBV_ATTR_CTRL_CYCLETIME,
					qbvconf.admin.cycle_time))
				return -EMSGSIZE;

		if (qbvconf.admin.cycle_time_extension)
			if (nla_put_u32(rep_skb, TSN_QBV_ATTR_CTRL_CYCLETIMEEXT,
					qbvconf.admin.cycle_time_extension))
				return -EMSGSIZE;

		if (qbvconf.admin.base_time)
			if (NLA_PUT_U64(rep_skb, TSN_QBV_ATTR_CTRL_BASETIME,
					qbvconf.admin.base_time))
				return -EMSGSIZE;

		kfree(qbvconf.admin.control_list);

	} else {
		pr_info("tsn: error get administrator data.");
	}

	nla_nest_end(rep_skb, qbvadminattr);

	if (qbvconf.gate_enabled) {
		if (nla_put_flag(rep_skb, TSN_QBV_ATTR_ENABLE))
			return -EMSGSIZE;
	} else {
		if (nla_put_flag(rep_skb, TSN_QBV_ATTR_DISABLE))
			return -EMSGSIZE;
	}

	if (qbvconf.maxsdu)
		if (nla_put_u32(rep_skb, TSN_QBV_ATTR_MAXSDU, qbvconf.maxsdu))
			return -EMSGSIZE;

	if (qbvconf.config_change)
		if (nla_put_flag(rep_skb, TSN_QBV_ATTR_CONFIGCHANGE))
			return -EMSGSIZE;

	nla_nest_end(rep_skb, qbv);

	tsn_send_reply(rep_skb, info);

	return ret;
}

static int cmd_qbv_status_get(struct genl_info *info)
{
	struct nlattr *qbv, *qbvoperattr;
	struct sk_buff *rep_skb;
	int ret;
	int len = 0, i = 0;
	struct net_device *netdev;
	struct genlmsghdr *genlhdr;
	struct tsn_qbv_status qbvstatus;
	const struct tsn_ops *tsnops;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	genlhdr = info->genlhdr;

	memset(&qbvstatus, 0, sizeof(struct tsn_qbv_status));

	if (!tsnops->qbv_get_status) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	ret = tsnops->qbv_get_status(netdev, &qbvstatus);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	ret = tsn_prepare_reply(info, genlhdr->cmd,
				&rep_skb, NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		return ret;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name))
		return -EMSGSIZE;

	qbv = nla_nest_start(rep_skb, TSN_ATTR_QBV);
	if (!qbv)
		return -EMSGSIZE;

	qbvoperattr = nla_nest_start(rep_skb, TSN_QBV_ATTR_OPERENTRY);
	if (!qbvoperattr)
		return -EMSGSIZE;

	if (qbvstatus.oper.control_list) {
		len = qbvstatus.oper.control_list_length;
		if (nla_put_u32(rep_skb, TSN_QBV_ATTR_CTRL_LISTCOUNT, len)) {
			nla_nest_cancel(rep_skb, qbvoperattr);
			return -EMSGSIZE;
		}

		for (i = 0; i < len; i++) {
			struct nlattr *qbv_table;
			u8 gs;
			u32 tp;
			int glisttype = TSN_QBV_ATTR_CTRL_LISTENTRY;

			gs = (qbvstatus.oper.control_list + i)->gate_state;
			tp = (qbvstatus.oper.control_list + i)->time_interval;

			qbv_table = nla_nest_start(rep_skb, glisttype);
			if (!qbv_table)
				return -EMSGSIZE;

			if (nla_put_u32(rep_skb, TSN_QBV_ATTR_ENTRY_ID, i) ||
			    nla_put_u8(rep_skb, TSN_QBV_ATTR_ENTRY_GC, gs) ||
			    nla_put_u32(rep_skb, TSN_QBV_ATTR_ENTRY_TM, tp)) {
				nla_nest_cancel(rep_skb, qbv_table);
				return -EMSGSIZE;
			}

			nla_nest_end(rep_skb, qbv_table);
		}

		if (qbvstatus.oper.gate_states) {
			if (nla_put_u8(rep_skb, TSN_QBV_ATTR_CTRL_GATESTATE,
				       qbvstatus.oper.gate_states))
				return -EMSGSIZE;
		}

		if (qbvstatus.oper.cycle_time) {
			if (nla_put_u32(rep_skb, TSN_QBV_ATTR_CTRL_CYCLETIME,
					qbvstatus.oper.cycle_time))
				return -EMSGSIZE;
		}

		if (qbvstatus.oper.cycle_time_extension) {
			if (nla_put_u32(rep_skb, TSN_QBV_ATTR_CTRL_CYCLETIMEEXT,
					qbvstatus.oper.cycle_time_extension))
				return -EMSGSIZE;
		}

		if (qbvstatus.oper.base_time) {
			if (NLA_PUT_U64(rep_skb, TSN_QBV_ATTR_CTRL_BASETIME,
					qbvstatus.oper.base_time))
				return -EMSGSIZE;
		}

		kfree(qbvstatus.oper.control_list);
	} else {
		pr_info("tsn: error get operation list data.");
	}

	nla_nest_end(rep_skb, qbvoperattr);

	if (qbvstatus.config_change_time) {
		if (NLA_PUT_U64(rep_skb, TSN_QBV_ATTR_CONFIGCHANGETIME,
				qbvstatus.config_change_time))
			return -EMSGSIZE;
	}

	if (qbvstatus.tick_granularity) {
		if (nla_put_u32(rep_skb, TSN_QBV_ATTR_GRANULARITY,
				qbvstatus.tick_granularity))
			return -EMSGSIZE;
	}

	if (qbvstatus.current_time) {
		if (NLA_PUT_U64(rep_skb, TSN_QBV_ATTR_CURRENTTIME,
				qbvstatus.current_time))
			return -EMSGSIZE;
	}

	if (qbvstatus.config_pending) {
		if (nla_put_flag(rep_skb, TSN_QBV_ATTR_CONFIGPENDING))
			return -EMSGSIZE;
	}

	if (qbvstatus.config_change_error) {
		if (NLA_PUT_U64(rep_skb, TSN_QBV_ATTR_CONFIGCHANGEERROR,
				qbvstatus.config_change_error))
			return -EMSGSIZE;
	}

	if (qbvstatus.supported_list_max) {
		if (nla_put_u32(rep_skb, TSN_QBV_ATTR_LISTMAX,
				qbvstatus.supported_list_max))
			return -EMSGSIZE;
	}

	nla_nest_end(rep_skb, qbv);

	tsn_send_reply(rep_skb, info);

	return ret;
}

static int tsn_qbv_status_get(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME])
		cmd_qbv_status_get(info);

	return 0;
}

static int tsn_qbv_get(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME])
		cmd_qbv_get(info);

	return 0;
}

static int tsn_cbs_set(struct sk_buff *skb, struct genl_info *info)
{
	struct nlattr *na;
	struct nlattr *cbsa[TSN_CBS_ATTR_MAX + 1];
	struct net_device *netdev;
	const struct tsn_ops *tsnops;
	int ret;
	u8 tc, bw;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	if (!info->attrs[TSN_ATTR_CBS]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	na = info->attrs[TSN_ATTR_CBS];

	if (!tsnops->cbs_set) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	ret = NLA_PARSE_NESTED(cbsa, TSN_CBS_ATTR_MAX, na, cbs_policy);
	if (ret) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	if (!cbsa[TSN_CBS_ATTR_TC_INDEX]) {
		pr_err("tsn: no TSN_CBS_ATTR_TC_INDEX input\n");
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}
	tc = nla_get_u8(cbsa[TSN_CBS_ATTR_TC_INDEX]);

	if (!cbsa[TSN_CBS_ATTR_BW]) {
		pr_err("tsn: no TSN_CBS_ATTR_BW input\n");
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	bw = nla_get_u8(cbsa[TSN_CBS_ATTR_BW]);
	if (bw > 100) {
		pr_err("tsn: TSN_CBS_ATTR_BW isn't in the range of 0~100\n");
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	ret = tsnops->cbs_set(netdev, tc, bw);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	tsn_simple_reply(info, TSN_CMD_REPLY,
			 netdev->name, 0);
	return 0;
}

static int tsn_cbs_get(struct sk_buff *skb, struct genl_info *info)
{
	struct nlattr *na, *cbsattr;
	struct nlattr *cbsa[TSN_CBS_ATTR_MAX + 1];
	struct net_device *netdev;
	const struct tsn_ops *tsnops;
	struct sk_buff *rep_skb;
	int ret;
	struct genlmsghdr *genlhdr;
	u8 tc;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	if (!info->attrs[TSN_ATTR_CBS]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	if (!tsnops->cbs_get) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	na = info->attrs[TSN_ATTR_CBS];
	ret = NLA_PARSE_NESTED(cbsa, TSN_CBS_ATTR_MAX, na, cbs_policy);
	if (ret) {
		pr_err("tsn: parse value TSN_CBS_ATTR_MAX error.");
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	/* Get status data from device */
	genlhdr = info->genlhdr;

	/* Form netlink reply data */
	ret = tsn_prepare_reply(info, genlhdr->cmd, &rep_skb,
				NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		return ret;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name))
		return -EMSGSIZE;

	cbsattr = nla_nest_start(rep_skb, TSN_ATTR_CBS);
	if (!cbsattr)
		return -EMSGSIZE;

	if (!cbsa[TSN_CBS_ATTR_TC_INDEX]) {
		pr_err("tsn: must to specify the TSN_CBS_ATTR_TC_INDEX\n");
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}
	tc = nla_get_u8(cbsa[TSN_CBS_ATTR_TC_INDEX]);

	ret = tsnops->cbs_get(netdev, tc);
	if (ret < 0) {
		pr_err("tsn: cbs_get return error\n");
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	if (nla_put_u8(rep_skb, TSN_CBS_ATTR_BW, ret & 0XF))
		return -EMSGSIZE;

	nla_nest_end(rep_skb, cbsattr);
	return tsn_send_reply(rep_skb, info);
}

static int cmd_qbu_set(struct genl_info *info)
{
	struct nlattr *na;
	struct nlattr *qbua[TSN_QBU_ATTR_MAX + 1];
	struct net_device *netdev;
	const struct tsn_ops *tsnops;
	int ret;
	u8 preemptible = 0;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	if (!info->attrs[TSN_ATTR_QBU]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	na = info->attrs[TSN_ATTR_QBU];

	ret = NLA_PARSE_NESTED(qbua, TSN_QBU_ATTR_MAX, na, qbu_policy);
	if (ret) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	if (qbua[TSN_QBU_ATTR_ADMIN_STATE])
		preemptible = nla_get_u8(qbua[TSN_QBU_ATTR_ADMIN_STATE]);
	else
		pr_info("No preemptible TSN_QBU_ATTR_ADMIN_STATE config!\n");

	if (!tsnops->qbu_set) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -EINVAL;
	}

	ret = tsnops->qbu_set(netdev, preemptible);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	tsn_simple_reply(info, TSN_CMD_REPLY,
			 netdev->name, 0);
	return 0;
}

static int tsn_qbu_set(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME])
		return cmd_qbu_set(info);

	return -1;
}

static int cmd_qbu_get_status(struct genl_info *info)
{
	struct nlattr *qbuattr;
	struct net_device *netdev;
	const struct tsn_ops *tsnops;
	struct sk_buff *rep_skb;
	int ret;
	struct genlmsghdr *genlhdr;
	struct tsn_preempt_status pps;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	/* Get status data from device */
	genlhdr = info->genlhdr;

	memset(&pps, 0, sizeof(struct tsn_preempt_status));

	if (!tsnops->qbu_get) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	ret = tsnops->qbu_get(netdev, &pps);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	/* Form netlink reply data */
	ret = tsn_prepare_reply(info, genlhdr->cmd,
				&rep_skb, NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		return ret;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name))
		return -EMSGSIZE;

	qbuattr = nla_nest_start(rep_skb, TSN_ATTR_QBU);
	if (!qbuattr)
		return -EMSGSIZE;

	if (nla_put_u8(rep_skb, TSN_QBU_ATTR_ADMIN_STATE, pps.admin_state) ||
	    nla_put_u32(rep_skb,
			TSN_QBU_ATTR_HOLD_ADVANCE, pps.hold_advance) ||
	    nla_put_u32(rep_skb,
			TSN_QBU_ATTR_RELEASE_ADVANCE, pps.release_advance))
		return -EMSGSIZE;

	if (pps.preemption_active) {
		if (nla_put_flag(rep_skb, TSN_QBU_ATTR_ACTIVE))
			return -EMSGSIZE;
	}

	if (nla_put_u8(rep_skb, TSN_QBU_ATTR_HOLD_REQUEST, pps.hold_request))
		return -EMSGSIZE;

	nla_nest_end(rep_skb, qbuattr);

	return tsn_send_reply(rep_skb, info);
}

static int tsn_qbu_get_status(struct sk_buff *skb, struct genl_info *info)
{
	if (info->attrs[TSN_ATTR_IFNAME])
		return cmd_qbu_get_status(info);

	return -1;
}

static int tsn_tsd_set(struct sk_buff *skb, struct genl_info *info)
{
	struct nlattr *na;
	struct nlattr *ntsd[TSN_TSD_ATTR_MAX + 1];
	struct net_device *netdev;
	const struct tsn_ops *tsnops;
	struct tsn_tsd tsd;
	int ret;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	memset(&tsd, 0, sizeof(struct tsn_tsd));

	if (!info->attrs[TSN_ATTR_TSD]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	na = info->attrs[TSN_ATTR_TSD];

	ret = NLA_PARSE_NESTED(ntsd, TSN_TSD_ATTR_MAX, na, tsd_policy);
	if (ret) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	if (!tsnops->tsd_set) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -EINVAL;
	}

	if (nla_get_flag(ntsd[TSN_TSD_ATTR_DISABLE])) {
		tsd.enable = false;
	} else {
		if (ntsd[TSN_TSD_ATTR_PERIOD])
			tsd.period = nla_get_u32(ntsd[TSN_TSD_ATTR_PERIOD]);

		if (!tsd.period) {
			tsn_simple_reply(info, TSN_CMD_REPLY,
					 netdev->name, -EINVAL);
			return -EINVAL;
		}

		if (ntsd[TSN_TSD_ATTR_MAX_FRM_NUM])
			tsd.maxFrameNum =
				nla_get_u32(ntsd[TSN_TSD_ATTR_MAX_FRM_NUM]);

		if (ntsd[TSN_TSD_ATTR_SYN_IMME])
			tsd.syn_flag = 2;
		else
			tsd.syn_flag = 1;

		tsd.enable = true;
	}

	ret = tsnops->tsd_set(netdev, &tsd);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	tsn_simple_reply(info, TSN_CMD_REPLY,
			 netdev->name, 0);
	return 0;
}

static int tsn_tsd_get(struct sk_buff *skb, struct genl_info *info)
{
	struct nlattr *na, *tsdattr;
	struct nlattr *tsda[TSN_TSD_ATTR_MAX + 1];
	struct net_device *netdev;
	const struct tsn_ops *tsnops;
	struct sk_buff *rep_skb;
	int ret;
	struct genlmsghdr *genlhdr;
	struct tsn_tsd_status tts;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	if (!info->attrs[TSN_ATTR_TSD]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	if (!tsnops->tsd_get) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	ret = tsnops->tsd_get(netdev, &tts);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	na = info->attrs[TSN_ATTR_TSD];

	ret = NLA_PARSE_NESTED(tsda, TSN_TSD_ATTR_MAX,
			       na, tsd_policy);
	if (ret) {
		pr_err("tsn: parse value TSN_TSD_ATTR_MAX error.");
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	/* Get status data from device */
	genlhdr = info->genlhdr;

	/* Form netlink reply data */
	ret = tsn_prepare_reply(info, genlhdr->cmd, &rep_skb,
				NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		return ret;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name))
		return -EMSGSIZE;

	tsdattr = nla_nest_start(rep_skb, TSN_ATTR_TSD);
	if (!tsdattr)
		return -EMSGSIZE;

	if (nla_put_u32(rep_skb, TSN_TSD_ATTR_PERIOD, tts.period) ||
	    nla_put_u32(rep_skb, TSN_TSD_ATTR_MAX_FRM_NUM, tts.maxFrameNum) ||
	    nla_put_u32(rep_skb, TSN_TSD_ATTR_CYCLE_NUM, tts.cycleNum) ||
	    nla_put_u32(rep_skb, TSN_TSD_ATTR_LOSS_STEPS, tts.loss_steps) ||
	    nla_put_u32(rep_skb, TSN_TSD_ATTR_MAX_FRM_NUM, tts.maxFrameNum))
		return -EMSGSIZE;

	if (!tts.enable) {
		if (nla_put_flag(rep_skb, TSN_TSD_ATTR_DISABLE))
			return -EMSGSIZE;
	} else {
		if (nla_put_flag(rep_skb, TSN_TSD_ATTR_ENABLE))
			return -EMSGSIZE;
	}

	if (tts.flag == 2)
		if (nla_put_flag(rep_skb, TSN_TSD_ATTR_SYN_IMME))
			return -EMSGSIZE;

	nla_nest_end(rep_skb, tsdattr);
	return tsn_send_reply(rep_skb, info);
}

static int tsn_ct_set(struct sk_buff *skb, struct genl_info *info)
{
	struct nlattr *na;
	struct nlattr *cta[TSN_CT_ATTR_MAX + 1];
	struct net_device *netdev;
	const struct tsn_ops *tsnops;
	int ret;
	u8 queue_stat;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	if (!info->attrs[TSN_ATTR_CT]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	na = info->attrs[TSN_ATTR_CT];

	if (!tsnops->ct_set) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	ret = NLA_PARSE_NESTED(cta, TSN_CT_ATTR_MAX,
			       na, ct_policy);
	if (ret) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	queue_stat = nla_get_u8(cta[TSN_CT_ATTR_QUEUE_STATE]);

	ret = tsnops->ct_set(netdev, queue_stat);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	tsn_simple_reply(info, TSN_CMD_REPLY,
			 netdev->name, 0);
	return 0;
}

static int tsn_cbgen_set(struct sk_buff *skb, struct genl_info *info)
{
	struct nlattr *na;
	struct nlattr *cbgena[TSN_CBGEN_ATTR_MAX + 1];
	struct net_device *netdev;
	const struct tsn_ops *tsnops;
	int ret;
	u32 index;
	struct tsn_seq_gen_conf sg_conf;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	if (!info->attrs[TSN_ATTR_CBGEN]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	na = info->attrs[TSN_ATTR_CBGEN];

	if (!tsnops->cbgen_set) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	ret = NLA_PARSE_NESTED(cbgena, TSN_CBGEN_ATTR_MAX,
			       na, cbgen_policy);
	if (ret) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	index = nla_get_u32(cbgena[TSN_CBGEN_ATTR_INDEX]);

	memset(&sg_conf, 0, sizeof(struct tsn_seq_gen_conf));
	sg_conf.iport_mask = nla_get_u8(cbgena[TSN_CBGEN_ATTR_PORT_MASK]);
	sg_conf.split_mask = nla_get_u8(cbgena[TSN_CBGEN_ATTR_SPLIT_MASK]);
	sg_conf.seq_len = nla_get_u8(cbgena[TSN_CBGEN_ATTR_SEQ_LEN]);
	sg_conf.seq_num = nla_get_u32(cbgena[TSN_CBGEN_ATTR_SEQ_NUM]);

	ret = tsnops->cbgen_set(netdev, index, &sg_conf);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	tsn_simple_reply(info, TSN_CMD_REPLY,
			 netdev->name, 0);
	return 0;
}

static int tsn_cbrec_set(struct sk_buff *skb, struct genl_info *info)
{
	struct nlattr *na;
	struct nlattr *cbreca[TSN_CBREC_ATTR_MAX + 1];
	struct net_device *netdev;
	const struct tsn_ops *tsnops;
	int ret;
	u32 index;
	struct tsn_seq_rec_conf sr_conf;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	if (!info->attrs[TSN_ATTR_CBREC]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	na = info->attrs[TSN_ATTR_CBREC];

	if (!tsnops->cbrec_set) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	ret = NLA_PARSE_NESTED(cbreca, TSN_CBREC_ATTR_MAX,
			       na, cbrec_policy);
	if (ret) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	index = nla_get_u32(cbreca[TSN_CBREC_ATTR_INDEX]);

	memset(&sr_conf, 0, sizeof(struct tsn_seq_rec_conf));
	sr_conf.seq_len = nla_get_u8(cbreca[TSN_CBREC_ATTR_SEQ_LEN]);
	sr_conf.his_len = nla_get_u8(cbreca[TSN_CBREC_ATTR_HIS_LEN]);
	sr_conf.rtag_pop_en = nla_get_flag(cbreca[TSN_CBREC_ATTR_TAG_POP_EN]);

	ret = tsnops->cbrec_set(netdev, index, &sr_conf);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	tsn_simple_reply(info, TSN_CMD_REPLY,
			 netdev->name, 0);
	return 0;
}

static int tsn_cbstatus_get(struct sk_buff *skb, struct genl_info *info)
{
	struct nlattr *na;
	struct nlattr *cba[TSN_CBSTAT_ATTR_MAX + 1];
	struct nlattr *cbattr;
	struct net_device *netdev;
	const struct tsn_ops *tsnops;
	struct sk_buff *rep_skb;
	int ret;
	unsigned int index;
	struct genlmsghdr *genlhdr;
	struct tsn_cb_status cbstat;
	struct tsn_port *port;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	/* Get status data from device */
	genlhdr = info->genlhdr;

	memset(&cbstat, 0, sizeof(struct tsn_cb_status));

	if (!tsnops->cb_get) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	na = info->attrs[TSN_ATTR_CBSTAT];
	ret = NLA_PARSE_NESTED(cba, TSN_CBSTAT_ATTR_MAX,
			       na, cbstat_policy);
	if (ret) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	index = nla_get_u32(cba[TSN_CBSTAT_ATTR_INDEX]);

	ret = tsnops->cb_get(netdev, index, &cbstat);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	/* Form netlink reply data */
	ret = tsn_prepare_reply(info, genlhdr->cmd,
				&rep_skb, NLMSG_ALIGN(MAX_ATTR_SIZE));
	if (ret < 0)
		return ret;

	if (nla_put_string(rep_skb, TSN_ATTR_IFNAME, netdev->name))
		return -EMSGSIZE;

	cbattr = nla_nest_start(rep_skb, TSN_ATTR_CBSTAT);
	if (!cbattr)
		return -EMSGSIZE;

	if (nla_put_u8(rep_skb, TSN_CBSTAT_ATTR_GEN_REC, cbstat.gen_rec) ||
	    nla_put_u8(rep_skb, TSN_CBSTAT_ATTR_ERR, cbstat.err) ||
	    nla_put_u32(rep_skb, TSN_CBSTAT_ATTR_SEQ_NUM,
			cbstat.seq_num) ||
	    nla_put_u8(rep_skb, TSN_CBSTAT_ATTR_SEQ_LEN, cbstat.seq_len) ||
	    nla_put_u8(rep_skb, TSN_CBSTAT_ATTR_SPLIT_MASK,
		       cbstat.split_mask) ||
	    nla_put_u8(rep_skb, TSN_CBSTAT_ATTR_PORT_MASK,
		       cbstat.iport_mask) ||
	    nla_put_u8(rep_skb, TSN_CBSTAT_ATTR_HIS_LEN, cbstat.his_len) ||
	    nla_put_u32(rep_skb, TSN_CBSTAT_ATTR_SEQ_HIS,
			cbstat.seq_his))
		return -EMSGSIZE;

	nla_nest_end(rep_skb, cbattr);

	return tsn_send_reply(rep_skb, info);
}

static int tsn_dscp_set(struct sk_buff *skb, struct genl_info *info)
{
	struct nlattr *na;
	struct nlattr *dscpa[TSN_DSCP_ATTR_MAX + 1];
	struct net_device *netdev;
	const struct tsn_ops *tsnops;
	int ret;
	bool enable = 0;
	struct tsn_port *port;
	int dscp_ix;
	struct tsn_qos_switch_dscp_conf dscp_conf;

	port = tsn_init_check(info, &netdev);
	if (!port)
		return -ENODEV;

	tsnops = port->tsnops;

	if (!info->attrs[TSN_ATTR_DSCP]) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	na = info->attrs[TSN_ATTR_DSCP];

	if (!tsnops->dscp_set) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EPERM);
		return -1;
	}

	ret = NLA_PARSE_NESTED(dscpa, TSN_DSCP_ATTR_MAX,
			       na, dscp_policy);
	if (ret) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, -EINVAL);
		return -EINVAL;
	}

	enable = 1;
	if (dscpa[TSN_DSCP_ATTR_DISABLE])
		enable = 0;
	dscp_ix = nla_get_u32(dscpa[TSN_DSCP_ATTR_INDEX]);
	dscp_conf.cos = nla_get_u32(dscpa[TSN_DSCP_ATTR_COS]);
	dscp_conf.dpl = nla_get_u32(dscpa[TSN_DSCP_ATTR_DPL]);
	ret = tsnops->dscp_set(netdev, enable, dscp_ix, &dscp_conf);
	if (ret < 0) {
		tsn_simple_reply(info, TSN_CMD_REPLY,
				 netdev->name, ret);
		return ret;
	}

	tsn_simple_reply(info, TSN_CMD_REPLY,
			 netdev->name, 0);

	return 0;
}

static const struct genl_ops tsnnl_ops[] = {
	{
		.cmd		= TSN_CMD_ECHO,
		.doit		= tsn_echo_cmd,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_CAP_GET,
		.doit		= tsn_cap_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QBV_SET,
		.doit		= tsn_qbv_set,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QBV_GET,
		.doit		= tsn_qbv_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QBV_GET_STATUS,
		.doit		= tsn_qbv_status_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_CB_STREAMID_SET,
		.doit		= tsn_cb_streamid_set,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_CB_STREAMID_GET,
		.doit		= tsn_cb_streamid_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_CB_STREAMID_GET_COUNTS,
		.doit		= tsn_cb_streamid_counters_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QCI_CAP_GET,
		.doit		= tsn_qci_cap_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QCI_SFI_SET,
		.doit		= tsn_qci_sfi_set,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QCI_SFI_GET,
		.doit		= tsn_qci_sfi_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QCI_SFI_GET_COUNTS,
		.doit		= tsn_qci_sfi_counters_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QCI_SGI_SET,
		.doit		= tsn_qci_sgi_set,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QCI_SGI_GET,
		.doit		= tsn_qci_sgi_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QCI_SGI_GET_STATUS,
		.doit		= tsn_qci_sgi_status_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QCI_FMI_SET,
		.doit		= tsn_qci_fmi_set,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QCI_FMI_GET,
		.doit		= tsn_qci_fmi_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_CBS_SET,
		.doit		= tsn_cbs_set,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_CBS_GET,
		.doit		= tsn_cbs_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QBU_SET,
		.doit		= tsn_qbu_set,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_QBU_GET_STATUS,
		.doit		= tsn_qbu_get_status,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_TSD_SET,
		.doit		= tsn_tsd_set,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_TSD_GET,
		.doit		= tsn_tsd_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_CT_SET,
		.doit		= tsn_ct_set,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_CBGEN_SET,
		.doit		= tsn_cbgen_set,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_CBREC_SET,
		.doit		= tsn_cbrec_set,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_CBSTAT_GET,
		.doit		= tsn_cbstatus_get,
		.flags		= GENL_ADMIN_PERM,
	},
	{
		.cmd		= TSN_CMD_DSCP_SET,
		.doit		= tsn_dscp_set,
		.flags		= GENL_ADMIN_PERM,
	},
};

static const struct genl_multicast_group tsn_mcgrps[] = {
	[TSN_MCGRP_QBV] = { .name = TSN_MULTICAST_GROUP_QBV},
	[TSN_MCGRP_QCI] = { .name = TSN_MULTICAST_GROUP_QCI},
};

static struct genl_family tsn_family = {
	.name		= TSN_GENL_NAME,
	.version	= TSN_GENL_VERSION,
	.maxattr	= TSN_CMD_ATTR_MAX,
	.module		= THIS_MODULE,
	.netnsok	= true,
	.ops		= tsnnl_ops,
	.n_ops		= ARRAY_SIZE(tsnnl_ops),
	.mcgrps		= tsn_mcgrps,
	.n_mcgrps	= ARRAY_SIZE(tsn_mcgrps),
};

int tsn_port_register(struct net_device *netdev,
		      struct tsn_ops *tsnops, u16 groupid)
{
	struct tsn_port *port;

	if (list_empty(&port_list)) {
		INIT_LIST_HEAD(&port_list);
	} else {
		list_for_each_entry(port, &port_list, list) {
			if (port->netdev == netdev) {
				pr_info("TSN device already registered!\n");
				return -1;
			}
		}
	}

	port = kzalloc(sizeof(*port), GFP_KERNEL);
	if (!port)
		return -1;

	port->netdev = netdev;
	port->groupid = groupid;
	port->tsnops = tsnops;
	port->nd.dev = netdev;

	if (groupid < GROUP_OFFSET_SWITCH)
		port->type = TSN_ENDPOINT;
	else
		port->type = TSN_SWITCH;

	list_add_tail(&port->list, &port_list);

	if (tsnops && tsnops->device_init)
		port->tsnops->device_init(netdev);

	return 0;
}
EXPORT_SYMBOL(tsn_port_register);

void tsn_port_unregister(struct net_device *netdev)
{
	struct tsn_port *p;

	list_for_each_entry(p, &port_list, list) {
		if (!p || !p->netdev)
			continue;
		if (p->netdev == netdev) {
			if (p->tsnops->device_deinit)
				p->tsnops->device_deinit(netdev);
			list_del(&p->list);
			kfree(p);
			break;
		}
	}
}
EXPORT_SYMBOL(tsn_port_unregister);

static int tsn_multicast_to_user(unsigned long event,
				 struct tsn_notifier_info *tsn_info)
{
	struct sk_buff *skb;
	struct genlmsghdr *nlh;
	int res;
	struct tsn_qbv_conf *qbvdata;

	/* If new attributes are added, please revisit this allocation */
	skb = genlmsg_new(sizeof(*tsn_info), GFP_KERNEL);
	if (!skb) {
		pr_err("Allocation failure.\n");
		return -ENOMEM;
	}

	switch (event) {
	case TSN_QBV_CONFIGCHANGETIME_ARRIVE:
		nlh = genlmsg_put(skb, 0, 1, &tsn_family,
				  GFP_KERNEL, TSN_CMD_QBV_SET);
		qbvdata = &tsn_info->ntdata.qbv_notify;
		res = NLA_PUT_U64(skb, TSN_QBV_ATTR_CTRL_BASETIME,
				  qbvdata->admin.base_time);

		if (res) {
			pr_err("put data failure!\n");
			goto done;
		}

		res = nla_put_u32(skb, TSN_QBV_ATTR_CTRL_CYCLETIME,
				  qbvdata->admin.cycle_time);
		if (res) {
			pr_err("put data failure!\n");
			goto done;
		}

		if (qbvdata->gate_enabled)
			res = nla_put_flag(skb, TSN_QBV_ATTR_ENABLE +
					   TSN_QBV_ATTR_CTRL_MAX);
		else
			res = nla_put_flag(skb, TSN_QBV_ATTR_DISABLE +
					   TSN_QBV_ATTR_CTRL_MAX);
		if (res) {
			pr_err("put data failure!\n");
			goto done;
		}

		res = nla_put_u32(skb, TSN_QBV_ATTR_CTRL_UNSPEC,
				  tsn_info->dev->ifindex);
		if (res) {
			pr_err("put data failure!\n");
			goto done;
		}

		break;
	default:
		pr_info("event not supportted!\n");
		break;
	}

	(void)genlmsg_end(skb, nlh);

	res = genlmsg_multicast_allns(&tsn_family, skb, 0,
				      TSN_MCGRP_QBV, GFP_KERNEL);
	skb = NULL;
	if (res && res != -ESRCH) {
		pr_err("genlmsg_multicast_allns error: %d\n", res);
		goto done;
	}

	if (res == -ESRCH)
		res = 0;

done:
	if (skb) {
		nlmsg_free(skb);
		skb = NULL;
	}

	return res;
}

/* called with RTNL or RCU */
static int tsn_event(struct notifier_block *unused,
		     unsigned long event, void *ptr)
{
	struct tsn_notifier_info *tsn_info;
	int err = NOTIFY_DONE;

	switch (event) {
	case TSN_QBV_CONFIGCHANGETIME_ARRIVE:
		tsn_info = ptr;
		err = tsn_multicast_to_user(event, tsn_info);
		if (err) {
			err = notifier_from_errno(err);
			break;
		}
		break;
	default:
		pr_info("event not supportted!\n");
		break;
	}

	return err;
}

static struct notifier_block tsn_notifier = {
	.notifier_call = tsn_event,
};

static int __init tsn_genetlink_init(void)
{
	int ret;

	pr_info("tsn generic netlink module v%d init...\n", TSN_GENL_VERSION);

	ret = genl_register_family(&tsn_family);

	if (ret != 0) {
		pr_info("failed to init tsn generic netlink example module\n");
		return ret;
	}

	register_tsn_notifier(&tsn_notifier);

	return 0;
}

static void __exit tsn_genetlink_exit(void)
{
	int ret;

	ret = genl_unregister_family(&tsn_family);
	if (ret != 0)
		pr_info("failed to unregister family:%i\n", ret);

	unregister_tsn_notifier(&tsn_notifier);
}

module_init(tsn_genetlink_init);
module_exit(tsn_genetlink_exit);
MODULE_LICENSE("GPL");
