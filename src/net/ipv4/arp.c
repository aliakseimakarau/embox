/**
 * @file arp.c
 *
 * @brief This module implements the Address Resolution Protocol ARP (RFC 826),
 * which is used to convert IP addresses into a low-level hardware address.
 * @date 11.03.2009
 * @author Anton Bondarev
 */
#include <string.h>
#include <common.h>
#include <in.h>
#include <kernel/module.h>
#include <net/skbuff.h>
#include <net/netdevice.h>
#include <net/net.h>
#include <net/inetdevice.h>
#include <net/etherdevice.h>
#include <net/net_pack_manager.h>
#include <net/arp.h>
#include <net/ip.h>

arp_table_t arp_tables[ARP_CACHE_SIZE];

#define ARP_TABLE_SIZE array_len(arp_tables)

#define ARP_TIMER_ID 12
#define ARP_GAP      500

static struct packet_type arp_packet_type = {
        .type = ETH_P_ARP,
        .func = arp_rcv,
};

void arp_update() {
	int i;
	for (i = 0; i < ARP_CACHE_SIZE; ++i) {
		arp_tables[i].ctime++;
		if( arp_tables[i].state == 1 &&
		    arp_tables[i].ctime >= ARP_ALIVE) {
			arp_tables[i].state = 0;
		}
	}
}

void __init arp_init() {
        dev_add_pack(&arp_packet_type);
        set_timer(ARP_TIMER_ID, ARP_GAP, arp_update);
}

static inline int find_entity(in_device_t *in_dev, in_addr_t dst_addr) {
	int i;
	for (i = 0; i < ARP_TABLE_SIZE; i++) {
		if((arp_tables[i].state == 1) &&
		   (arp_tables[i].pw_addr == dst_addr) &&
		   (in_dev == arp_tables[i].if_handler)) {
			return i;
		}
	}
	return -1;
}

/**
 * this function add entry in arp table if can
 * @param iт_dev (handler of inet_dev struct) which identificate network interface where address can resolve
 * @param ip addr
 * @param hardware addr
 * @return number of entry in table if success else -1
 */
int arp_add_entity(in_device_t *in_dev, in_addr_t ipaddr, unsigned char *macaddr) {
	int i;
	if (-1 != (i = find_entity(in_dev, ipaddr))) {
		return i;
	}
	for (i = 0; i < ARP_TABLE_SIZE; i++) {
		if(arp_tables[i].state == 0) {
			arp_tables[i].if_handler = in_dev;
			arp_tables[i].pw_addr = ipaddr;
			arp_tables[i].ctime = 0;
			arp_tables[i].state = 1;
			memcpy(arp_tables[i].hw_addr, macaddr, ETH_ALEN);
			return i;
		}
	}
	return -1;
}

/**
 * this function delete entry from arp table if can
 * @param in_dev (handler of inet_dev struct) which identificate network interface where address can resolve
 * @param ip addr
 * @param hardware addr
 * @return number of entry in table if success else -1
 */
int arp_delete_entity(in_device_t *in_dev, in_addr_t ipaddr, unsigned char *macaddr) {
	int i;
	for (i = 0; i < ARP_TABLE_SIZE; i++) {
		if( arp_tables[i].pw_addr == ipaddr ||
        	    0 == memcmp(arp_tables[i].hw_addr, macaddr, ETH_ALEN) ||
        	    in_dev == arp_tables[i].if_handler) {
			arp_tables[i].state = 0;
    		}
	}
	return -1;
}

sk_buff_t *arp_create(int type, int ptype, in_addr_t dest_ip,
                           net_device_t *dev, in_addr_t src_ip,
			const unsigned char *dest_hw, const unsigned char *src_hw,
			const unsigned char *target_hw) {
	sk_buff_t *pack;
	struct arphdr *arp;
	if (NULL == dev ||
	    NULL == (pack = alloc_skb(ETH_HEADER_SIZE + ARP_HEADER_SIZE, 0))) {
		return NULL;
	}

	pack->mac.raw = pack->data;
	pack->nh.raw = pack->mac.raw + ETH_HEADER_SIZE;
	arp = pack->nh.arph;

	pack->dev  = dev;
	pack->protocol = htons(ptype);
	if (src_hw == NULL) {
	        src_hw = dev->dev_addr;
	}
	if (dest_hw == NULL) {
	        dest_hw = dev->broadcast;
	}

	/*
	 * Fill the device header for the ARP frame
	 */
	if (dev_hard_header(pack, dev, ptype, (void*)dest_hw, (void*)src_hw, pack->len) < 0) {
		kfree_skb(pack);
		return NULL;
	}

	/*
	 * Fill out the arp protocol part.
	 */
	arp->ar_hrd = htons(ARPHRD_ETHER);
	arp->ar_pro = htons(ETH_P_IP);
	arp->ar_hln = dev->addr_len;
	arp->ar_pln = IPV4_ADDR_LENGTH;
	arp->ar_op = htons(type);
	memcpy (arp->ar_sha, src_hw, ETH_ALEN);
	memcpy (arp->ar_tha, dest_hw, ETH_ALEN);
	arp->ar_sip = src_ip;
	arp->ar_tip = dest_ip;

	return pack;
}

void arp_send(int type, int ptype, in_addr_t dest_ip,
                    struct net_device *dev, in_addr_t src_ip,
                    const unsigned char *dest_hw,
                    const unsigned char *src_hw, const unsigned char *th) {
	struct sk_buff *pack;

	/*
	 * No arp on this interface.
	 */
	if (dev->flags & IFF_NOARP) {
		return;
	}
	pack = arp_create(type, ptype, dest_ip, dev, src_ip,
	                        dest_hw, src_hw, th);
	if (pack == NULL) {
	        return;
	}
	arp_xmit(pack);
}

int arp_find(unsigned char *haddr, sk_buff_t *pack) {
	net_device_t *dev = pack->dev;
	iphdr_t *ip = pack->nh.iph;
	pack->mac.raw = pack->data;
	int i;
	if (ip->daddr == INADDR_BROADCAST) {
		return -1;
	}
	if(-1 != (i = find_entity(in_dev_get(dev), ip->daddr))) {
		memcpy (pack->mac.ethh->h_dest, arp_tables[i].hw_addr, ETH_ALEN);
		return 0;
	}
	arp_send(ARPOP_REQUEST, ETH_P_ARP, ip->daddr, dev,
			    ip->saddr, NULL, dev->dev_addr, NULL);

	//TODO delete this after processes will be added to monitor
	usleep(500);

	if (-1 != (i = find_entity(in_dev_get(dev), ip->daddr))) {
		memcpy (pack->mac.ethh->h_dest, arp_tables[i].hw_addr, ETH_ALEN);
		return 0;
	}
	return -1;
}

/**
 * receive ARP response, update ARP table
 */
static int received_resp(sk_buff_t *pack) {
	arphdr_t *arp = pack->nh.arph;

	//TODO need add function for getting ip addr
#if 0
	if (inet_dev_get_ipaddr(pack->ifdev) != arp->ar_tip) {
		return -1;
	}
#endif
	return 0;
}

/**
 * receive ARP request, send ARP response
 */
static int received_req(sk_buff_t *pack) {
	arphdr_t *arp = pack->nh.arph;
	arp_send(ARPOP_REPLY, ETH_P_ARP, arp->ar_sip, pack->dev,
	                        arp->ar_tip, pack->mac.ethh->h_source, pack->dev->dev_addr, NULL);
	return 0;
}

/**
 * Process an arp request.
 */
static int arp_process(sk_buff_t *pack) {
	int ret;
	struct net_device *dev = pack->dev;
	struct in_device *in_dev = in_dev_get(dev);
	arphdr_t *arp = pack->nh.arph;

	if (ipv4_is_loopback(arp->ar_tip) ||
	    ipv4_is_multicast(arp->ar_tip) ||
	    arp->ar_tip != in_dev_get(pack->dev)->ifa_address) {
		return 0;
	}

	switch(arp->ar_op) {
	case htons(ARPOP_REPLY):
		ret = received_resp(pack);
		break;
	case htons(ARPOP_REQUEST):
		ret = received_req(pack);
		break;
        default:
                ret = 0;
        }
        /* add record into arp_tables */
        arp_add_entity(in_dev, arp->ar_sip, arp->ar_sha);
        return ret;
}

int arp_rcv(sk_buff_t *pack, net_device_t *dev,
                   packet_type_t *pt, net_device_t *orig_dev) {
	arphdr_t *arp = pack->nh.arph;
	if (arp->ar_hln != dev->addr_len ||
	    dev->flags & IFF_NOARP ||
	    pack->pkt_type == PACKET_OTHERHOST ||
	    pack->pkt_type == PACKET_LOOPBACK ||
	    arp->ar_pln != 4) {
	        kfree_skb(pack);
	        return NET_RX_SUCCESS;
	}

	return arp_process(pack);
}

void arp_xmit(sk_buff_t *skb) {
	dev_queue_xmit(skb);
}
