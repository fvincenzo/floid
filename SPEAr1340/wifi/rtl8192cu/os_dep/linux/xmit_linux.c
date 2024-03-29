/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 
******************************************************************************/
#define _XMIT_OSDEP_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>

#include <if_ether.h>
#include <ip.h>
#include <rtw_byteorder.h>
#include <wifi.h>
#include <mlme_osdep.h>
#include <xmit_osdep.h>
#include <osdep_intf.h>
#include <circ_buf.h>


uint remainder_len(struct pkt_file *pfile)
{
	return (pfile->buf_len - ((SIZE_PTR)(pfile->cur_addr) - (SIZE_PTR)(pfile->buf_start)));
}

void _open_pktfile (_pkt *pktptr, struct pkt_file *pfile)
{
_func_enter_;

	pfile->pkt = pktptr;
	pfile->cur_addr = pfile->buf_start = pktptr->data;
	pfile->pkt_len = pfile->buf_len = pktptr->len;

	pfile->cur_buffer = pfile->buf_start ;
	
_func_exit_;
}

uint _pktfile_read (struct pkt_file *pfile, u8 *rmem, uint rlen)
{	
	uint	len = 0;
	
_func_enter_;

       len =  remainder_len(pfile);
      	len = (rlen > len)? len: rlen;

       if(rmem)
	  skb_copy_bits(pfile->pkt, pfile->buf_len-pfile->pkt_len, rmem, len);

       pfile->cur_addr += len;
       pfile->pkt_len -= len;
	   
_func_exit_;	       		

	return len;	
}

sint endofpktfile(struct pkt_file *pfile)
{
_func_enter_;

	if (pfile->pkt_len == 0) {
_func_exit_;
		return _TRUE;
	}

_func_exit_;

	return _FALSE;
}

void set_tx_chksum_offload(_pkt *pkt, struct pkt_attrib *pattrib)
{

#ifdef CONFIG_RTL8712_TCP_CSUM_OFFLOAD_TX
	struct sk_buff *skb = (struct sk_buff *)pkt;
	pattrib->hw_tcp_csum = 0;
	
	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		if (skb_shinfo(skb)->nr_frags == 0)
		{	
                        const struct iphdr *ip = ip_hdr(skb);
                        if (ip->protocol == IPPROTO_TCP) {
                                // TCP checksum offload by HW
                                printk("CHECKSUM_PARTIAL TCP\n");
                                pattrib->hw_tcp_csum = 1;
                                //skb_checksum_help(skb);
                        } else if (ip->protocol == IPPROTO_UDP) {
                                //printk("CHECKSUM_PARTIAL UDP\n");
#if 1                       
                                skb_checksum_help(skb);
#else
                                // Set UDP checksum = 0 to skip checksum check
                                struct udphdr *udp = skb_transport_header(skb);
                                udp->check = 0;
#endif
                        } else {
				printk("%s-%d TCP CSUM offload Error!!\n", __FUNCTION__, __LINE__);
                                WARN_ON(1);     /* we need a WARN() */
			    }
		}
		else { // IP fragmentation case
			printk("%s-%d nr_frags != 0, using skb_checksum_help(skb);!!\n", __FUNCTION__, __LINE__);
                	skb_checksum_help(skb);
		}		
	}
#endif	
	
}

int os_xmit_resource_alloc(_adapter *padapter, struct xmit_buf *pxmitbuf,u32 alloc_sz)
{
#ifdef CONFIG_USB_HCI
	int i;
	struct dvobj_priv	*pdvobjpriv = &padapter->dvobjpriv;
	struct usb_device	*pusbd = pdvobjpriv->pusbdev;

#ifdef CONFIG_USE_USB_BUFFER_ALLOC

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	pxmitbuf->pallocated_buf = usb_alloc_coherent(pusbd, (size_t)alloc_sz, GFP_ATOMIC, &pxmitbuf->dma_transfer_addr);
#else
	pxmitbuf->pallocated_buf = usb_buffer_alloc(pusbd, (size_t)alloc_sz, GFP_ATOMIC, &pxmitbuf->dma_transfer_addr);
#endif
	pxmitbuf->pbuf = pxmitbuf->pallocated_buf;
	if(pxmitbuf->pallocated_buf == NULL)
		return _FAIL;
#else
	
	pxmitbuf->pallocated_buf = rtw_zmalloc(alloc_sz);
	if (pxmitbuf->pallocated_buf == NULL)
	{
		return _FAIL;
	}

	pxmitbuf->pbuf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitbuf->pallocated_buf), XMITBUF_ALIGN_SZ);
	pxmitbuf->dma_transfer_addr = 0;

#endif

       for(i=0; i<8; i++)
      	{
      		pxmitbuf->pxmit_urb[i] = usb_alloc_urb(0, GFP_KERNEL);
             	if(pxmitbuf->pxmit_urb[i] == NULL) 
             	{
             		printk("pxmitbuf->pxmit_urb[i]==NULL");
	        	return _FAIL;	 
             	}      		  	
	
      	}
#endif
#ifdef CONFIG_PCI_HCI
	pxmitbuf->pallocated_buf = rtw_zmalloc(alloc_sz);
	if (pxmitbuf->pallocated_buf == NULL)
	{
		return _FAIL;
	}

	pxmitbuf->pbuf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitbuf->pallocated_buf), XMITBUF_ALIGN_SZ);
#endif

	return _SUCCESS;	
}

void os_xmit_resource_free(_adapter *padapter, struct xmit_buf *pxmitbuf,u32 free_sz)
{
#ifdef CONFIG_USB_HCI
	int i;
	struct dvobj_priv	*pdvobjpriv = &padapter->dvobjpriv;
	struct usb_device	*pusbd = pdvobjpriv->pusbdev;


	for(i=0; i<8; i++)
	{
		if(pxmitbuf->pxmit_urb[i])
		{
			//usb_kill_urb(pxmitbuf->pxmit_urb[i]);
			usb_free_urb(pxmitbuf->pxmit_urb[i]);
		}
	}

#ifdef CONFIG_USE_USB_BUFFER_ALLOC

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	usb_free_coherent(pusbd, (size_t)free_sz, pxmitbuf->pallocated_buf, pxmitbuf->dma_transfer_addr);
#else
	usb_buffer_free(pusbd, (size_t)free_sz, pxmitbuf->pallocated_buf, pxmitbuf->dma_transfer_addr);
#endif
	pxmitbuf->pallocated_buf =  NULL;
	pxmitbuf->dma_transfer_addr = 0;
	
#else

	if(pxmitbuf->pallocated_buf)
		rtw_mfree(pxmitbuf->pallocated_buf, free_sz);

#endif

#endif
#ifdef CONFIG_PCI_HCI
	if(pxmitbuf->pallocated_buf)
		rtw_mfree(pxmitbuf->pallocated_buf, free_sz);
#endif
}

void os_pkt_complete(_adapter *padapter, _pkt *pkt)
{
	if (netif_queue_stopped(padapter->pnetdev))
		netif_wake_queue(padapter->pnetdev);

	dev_kfree_skb_any(pkt);
}

void os_xmit_complete(_adapter *padapter, struct xmit_frame *pxframe)
{
	if(pxframe->pkt)
	{
		//RT_TRACE(_module_xmit_osdep_c_,_drv_err_,("linux : os_xmit_complete, dev_kfree_skb()\n"));	

		//dev_kfree_skb_any(pxframe->pkt);	
		os_pkt_complete(padapter, pxframe->pkt);
		
	}	

	pxframe->pkt = NULL;
}

void os_xmit_schedule(_adapter *padapter)
{
	_irqL  irqL;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	_enter_critical_bh(&pxmitpriv->lock, &irqL);
	
	if(txframes_pending(padapter))	
	{
		tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
	}

	_exit_critical_bh(&pxmitpriv->lock, &irqL);
}


int xmit_entry(_pkt *pkt, _nic_hdl pnetdev)
{
	_adapter *padapter = (_adapter *)netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	s32 res = 0;
	int ret = 0;

_func_enter_;

	RT_TRACE(_module_rtl871x_mlme_c_, _drv_info_, ("+xmit_enry\n"));

	if (if_up(padapter) == _FALSE) {
		RT_TRACE(_module_xmit_osdep_c_, _drv_err_, ("xmit_entry: if_up fail\n"));
		goto drop_packet;
	}

	res = rtw_xmit(padapter, pkt);
	if (res < 0) goto drop_packet;

	pxmitpriv->tx_pkts++;
	RT_TRACE(_module_xmit_osdep_c_, _drv_info_, ("xmit_entry: tx_pkts=%d\n", (u32)pxmitpriv->tx_pkts));
	goto exit;

drop_packet:
	pxmitpriv->tx_drop++;
	dev_kfree_skb_any(pkt);
	RT_TRACE(_module_xmit_osdep_c_, _drv_notice_, ("xmit_entry: drop, tx_drop=%d\n", (u32)pxmitpriv->tx_drop));

exit:

_func_exit_;

	return 0;
}

