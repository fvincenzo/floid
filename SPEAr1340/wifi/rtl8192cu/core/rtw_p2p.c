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
#define _RTW_P2P_C_

#include <drv_types.h>
#include <rtw_p2p.h>
#include <wifi.h>

#if ( P2P_INCLUDED == 1 )


static u32 go_add_group_info_attr(struct wifidirect_info *pwdinfo, u8 *pbuf)
{
	_list	*phead, *plist;
	u32 len=0;
	u16 attr_len = 0;
	u8 tmplen, *pdata_attr, *pstart, *pcur;
	struct sta_info *psta = NULL;
	_adapter *padapter = pwdinfo->padapter;
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_871X("%s\n", __FUNCTION__);

	pdata_attr = rtw_zmalloc(MAX_P2P_IE_LEN);

	pstart = pdata_attr;
	pcur = pdata_attr;

	phead = &pstapriv->asoc_list;
	plist = get_next(phead);

	//look up sta asoc_queue
	while ((end_of_queue_search(phead, plist)) == _FALSE)	
	{		
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		
		plist = get_next(plist);
		

		if(psta->is_p2p_device)
		{
			tmplen = 0;
			
			pcur++;
		
			//P2P device address
			_memcpy(pcur, psta->dev_addr, ETH_ALEN);
			pcur += ETH_ALEN;

			//P2P interface address
			_memcpy(pcur, psta->hwaddr, ETH_ALEN);
			pcur += ETH_ALEN;

			*pcur = psta->dev_cap;
			pcur++;

			*(u16*)(pcur) = cpu_to_be16(psta->config_methods);
			pcur += 2;

			_memcpy(pcur, psta->primary_dev_type, 8);
			pcur += 8;

			*pcur = psta->num_of_secdev_type;
			pcur++;

			_memcpy(pcur, psta->secdev_types_list, psta->num_of_secdev_type*8);
			pcur += psta->num_of_secdev_type*8;
			
			if(psta->dev_name_len>0)
			{
				*(u16*)(pcur) = cpu_to_be16( WPS_ATTR_DEVICE_NAME );
				pcur += 2;

				*(u16*)(pcur) = cpu_to_be16( psta->dev_name_len );
				pcur += 2;

				_memcpy(pcur, psta->dev_name, psta->dev_name_len);
				pcur += psta->dev_name_len;
			}


			tmplen = (u8)(pcur-pstart);
			
			*pstart = (tmplen-1);

			attr_len += tmplen;

			//pstart += tmplen;
			pstart = pcur;
			
		}
		
		
	}

	if(attr_len>0)
	{
		len = set_p2p_attr_content(pbuf, P2P_ATTR_GROUP_INFO, attr_len, pdata_attr);
	}

	rtw_mfree(pdata_attr, MAX_P2P_IE_LEN);

	return len;

}

static void issue_group_disc_req(struct wifidirect_info *pwdinfo, u8 *da)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	_adapter *padapter = pwdinfo->padapter;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);	
	unsigned char category = WLAN_CATEGORY_P2P;//P2P action frame	
	u32	p2poui = cpu_to_be32(P2POUI);
	u8	oui_subtype = P2P_GO_DISC_REQUEST;
	u8	dialogToken=0;

	DBG_871X("[%s]\n", __FUNCTION__);

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, pwdinfo->interface_addr, ETH_ALEN);
	_memcpy(pwlanhdr->addr3, pwdinfo->interface_addr, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_ACTION);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	//Build P2P action frame header
	pframe = set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));	
	pframe = set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));	
	pframe = set_fixed_ie(pframe, 1, &(dialogToken), &(pattrib->pktlen));		

	//there is no IE in this P2P action frame

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

}

static void issue_p2p_devdisc_resp(struct wifidirect_info *pwdinfo, u8 *da, u8 status, u8 dialogToken)
{	
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	_adapter *padapter = pwdinfo->padapter;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);	
	unsigned char category = WLAN_CATEGORY_PUBLIC;
	u8			action = P2P_PUB_ACTION_ACTION;
	u32			p2poui = cpu_to_be32(P2POUI);
	u8			oui_subtype = P2P_DEVDISC_RESP;
	u8 p2pie[8] = { 0x00 };
	u32 p2pielen = 0;	

	DBG_871X("[%s]\n", __FUNCTION__);
	
	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, pwdinfo->device_addr, ETH_ALEN);
	_memcpy(pwlanhdr->addr3, pwdinfo->device_addr, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_ACTION);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	//Build P2P public action frame header
	pframe = set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));	
	pframe = set_fixed_ie(pframe, 1, &(dialogToken), &(pattrib->pktlen));		


	//Build P2P IE
	//	P2P OUI
	p2pielen = 0;
	p2pie[ p2pielen++ ] = 0x50;
	p2pie[ p2pielen++ ] = 0x6F;
	p2pie[ p2pielen++ ] = 0x9A;
	p2pie[ p2pielen++ ] = 0x09;	//	WFA P2P v1.0

	// P2P_ATTR_STATUS
	p2pielen += set_p2p_attr_content(&p2pie[p2pielen], P2P_ATTR_STATUS, 1, &status);
	
	pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, p2pie, &pattrib->pktlen);	

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

}

static void issue_p2p_provision_resp(struct wifidirect_info *pwdinfo, u8* raddr, u8* frame_body)
{
	_adapter *padapter = pwdinfo->padapter;
	unsigned char category = WLAN_CATEGORY_PUBLIC;
	u8			action = P2P_PUB_ACTION_ACTION;
	u8			dialogToken = frame_body[7];	//	The Dialog Token of provisioning discovery request frame.
	u32			p2poui = cpu_to_be32(P2POUI);
	u8			oui_subtype = P2P_PROVISION_DISC_RESP;
	u8			wpsie[ 100 ] = { 0x00 };
	u8			wpsielen = 0;
	
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);


	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, myid(&(padapter->eeprompriv)), ETH_ALEN);
	_memcpy(pwlanhdr->addr3, myid(&(padapter->eeprompriv)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_ACTION);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	pframe = set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));	
	pframe = set_fixed_ie(pframe, 1, &(dialogToken), &(pattrib->pktlen));		

	wpsielen = 0;
	//	WPS OUI
	*(u32*) ( wpsie ) = cpu_to_be32( WPSOUI );
	wpsielen += 4;

	//	WPS version
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_VER1 );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0001 );
	wpsielen += 2;

	//	Value:
	wpsie[wpsielen++] = WPS_VERSION_1;	//	Version 1.0

	//	Config Method
	//	Type:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_ATTR_CONF_METHOD );
	wpsielen += 2;

	//	Length:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( 0x0002 );
	wpsielen += 2;

	//	Value:
	*(u16*) ( wpsie + wpsielen ) = cpu_to_be16( WPS_CONFIG_METHOD_DISPLAY );
	wpsielen += 2;

	pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, wpsielen, (unsigned char *) wpsie, &pattrib->pktlen );	

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

	return;

}

static void issue_p2p_presence_resp(struct wifidirect_info *pwdinfo, u8 *da, u8 status, u8 dialogToken)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	_adapter *padapter = pwdinfo->padapter;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);	
	unsigned char category = WLAN_CATEGORY_P2P;//P2P action frame	
	u32	p2poui = cpu_to_be32(P2POUI);
	u8	oui_subtype = P2P_PRESENCE_RESPONSE;	
	u8 p2pie[ MAX_P2P_IE_LEN] = { 0x00 };
	u8 noa_attr_content[32] = { 0x00 };
	u32 p2pielen = 0;

	DBG_871X("[%s]\n", __FUNCTION__);

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		return;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	_memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	_memcpy(pwlanhdr->addr2, pwdinfo->interface_addr, ETH_ALEN);
	_memcpy(pwlanhdr->addr3, pwdinfo->interface_addr, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_ACTION);

	pframe += sizeof(struct ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct ieee80211_hdr_3addr);

	//Build P2P action frame header
	pframe = set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));	
	pframe = set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));	
	pframe = set_fixed_ie(pframe, 1, &(dialogToken), &(pattrib->pktlen));	


	//Add P2P IE header
	//	P2P OUI
	p2pielen = 0;
	p2pie[ p2pielen++ ] = 0x50;
	p2pie[ p2pielen++ ] = 0x6F;
	p2pie[ p2pielen++ ] = 0x9A;
	p2pie[ p2pielen++ ] = 0x09;	//	WFA P2P v1.0

	//Add Status attribute in P2P IE 
	p2pielen += set_p2p_attr_content(&p2pie[p2pielen], P2P_ATTR_STATUS, 1, &status);

	//Add NoA attribute in P2P IE
	noa_attr_content[0] = 0x1;//index
	noa_attr_content[1] = 0x0;//CTWindow and OppPS Parameters
	
	//todo: Notice of Absence Descriptor(s)
	
	p2pielen += set_p2p_attr_content(&p2pie[p2pielen], P2P_ATTR_NOA, 2, noa_attr_content);



	pframe = set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, p2pie, &(pattrib->pktlen));

	
	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

}

u32 build_beacon_p2p_ie(struct wifidirect_info *pwdinfo, u8 *pbuf)
{
	u8 p2pie[ MAX_P2P_IE_LEN] = { 0x00 };
	u16 capability=0;
	u32 len=0, p2pielen = 0;
	

	//	P2P OUI
	p2pielen = 0;
	p2pie[ p2pielen++ ] = 0x50;
	p2pie[ p2pielen++ ] = 0x6F;
	p2pie[ p2pielen++ ] = 0x9A;
	p2pie[ p2pielen++ ] = 0x09;	//	WFA P2P v1.0


	//	According to the P2P Specification, the beacon frame should contain 3 P2P attributes
	//	1. P2P Capability
	//	2. P2P Device ID
	//	3. Notice of Absence ( NOA )	

	//	P2P Capability ATTR
	//	Type:
	//	Length:
	//	Value:
	//	Device Capability Bitmap, 1 byte
	//	Be able to participate in additional P2P Groups and
	//	support the P2P Invitation Procedure	
	//	Group Capability Bitmap, 1 byte	
	capability = P2P_DEVCAP_INVITATION_PROC;
	capability |=  ((P2P_GRPCAP_GO | P2P_GRPCAP_INTRABSS) << 8);
	if(pwdinfo->p2p_state == P2P_STATE_PROVISIONING_ING)
		capability |= (P2P_GRPCAP_GROUP_FORMATION<<8);

	capability = cpu_to_le16(capability);

	p2pielen += set_p2p_attr_content(&p2pie[p2pielen], P2P_ATTR_CAPABILITY, 2, (u8*)&capability);

	
	// P2P Device ID ATTR
	p2pielen += set_p2p_attr_content(&p2pie[p2pielen], P2P_ATTR_DEVICE_ID, ETH_ALEN, pwdinfo->device_addr);

	
	// Notice of Absence ATTR
	//	Type: 
	//	Length:
	//	Value:
	
	//go_add_noa_attr(pwdinfo);
	
	
	pbuf = set_ie(pbuf, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &len);
	
	
	return len;
	
}

u32 build_probe_resp_p2p_ie(struct wifidirect_info *pwdinfo, u8 *pbuf)
{
	u8 p2pie[ MAX_P2P_IE_LEN] = { 0x00 };
	u32 len=0, p2pielen = 0;	

	//	P2P OUI
	p2pielen = 0;
	p2pie[ p2pielen++ ] = 0x50;
	p2pie[ p2pielen++ ] = 0x6F;
	p2pie[ p2pielen++ ] = 0x9A;
	p2pie[ p2pielen++ ] = 0x09;	//	WFA P2P v1.0

	//	Commented by Albert 20100907
	//	According to the P2P Specification, the probe response frame should contain 5 P2P attributes
	//	1. P2P Capability
	//	2. Extended Listen Timing
	//	3. Notice of Absence ( NOA )	( Only GO needs this )
	//	4. Device Info
	//	5. Group Info	( Only GO need this )

	//	P2P Capability ATTR
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_CAPABILITY;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0002 );
	p2pielen += 2;

	//	Value:
	//	Device Capability Bitmap, 1 byte
	//	Be able to participate in additional P2P Groups and
	//	support the P2P Invitation Procedure
	p2pie[ p2pielen++ ] = P2P_DEVCAP_INVITATION_PROC;
	
	//	Group Capability Bitmap, 1 byte
	if(pwdinfo->role == P2P_ROLE_GO)
	{
		p2pie[ p2pielen ] = (P2P_GRPCAP_GO | P2P_GRPCAP_INTRABSS);

		if(pwdinfo->p2p_state == P2P_STATE_PROVISIONING_ING)
			p2pie[ p2pielen ] |= P2P_GRPCAP_GROUP_FORMATION;

		p2pielen++;
	}
	else if ( pwdinfo->role == P2P_ROLE_DEVICE )
	{
		//	Group Capability Bitmap, 1 byte
		p2pie[ p2pielen++ ] = 0x00;
	}

	//	Extended Listen Timing ATTR
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_EX_LISTEN_TIMING;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0004 );
	p2pielen += 2;

	//	Value:
	//	Availability Period
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0xFFFF );
	p2pielen += 2;

	//	Availability Interval
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0xFFFF );
	p2pielen += 2;


	// Notice of Absence ATTR
	//	Type: 
	//	Length:
	//	Value:
	if(pwdinfo->role == P2P_ROLE_GO)
	{
		//go_add_noa_attr(pwdinfo);
	}	

	//	Device Info ATTR
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_DEVICE_INFO;

	//	Length:
	//	21 -> P2P Device Address (6bytes) + Config Methods (2bytes) + Primary Device Type (8bytes) 
	//	+ NumofSecondDevType (1byte) + WPS Device Name ID field (2bytes) + WPS Device Name Len field (2bytes)
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 21 + pwdinfo->device_name_len );
	p2pielen += 2;

	//	Value:
	//	P2P Device Address
	_memcpy( p2pie + p2pielen, pwdinfo->device_addr, ETH_ALEN );
	p2pielen += ETH_ALEN;

	//	Config Method
	//	This field should be big endian. Noted by P2P specification.
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_CONFIG_METHOD_DISPLAY );
	p2pielen += 2;

	//	Primary Device Type
	//	Category ID
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_PDT_CID_RTK_WIDI );
	p2pielen += 2;

	//	OUI
	*(u32*) ( p2pie + p2pielen ) = cpu_to_be32( WPSOUI );
	p2pielen += 4;

	//	Sub Category ID
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_PDT_SCID_RTK_DMP );
	p2pielen += 2;

	//	Number of Secondary Device Types
	p2pie[ p2pielen++ ] = 0x00;	//	No Secondary Device Type List

	//	Device Name
	//	Type:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_ATTR_DEVICE_NAME );
	p2pielen += 2;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( pwdinfo->device_name_len );
	p2pielen += 2;

	//	Value:
	_memcpy( p2pie + p2pielen, pwdinfo->device_name, pwdinfo->device_name_len );
	p2pielen += pwdinfo->device_name_len;


	// Group Info ATTR
	//	Type:
	//	Length:
	//	Value:
	if(pwdinfo->role == P2P_ROLE_GO)
	{
		p2pielen += go_add_group_info_attr(pwdinfo, p2pie + p2pielen);
	}

	
	pbuf = set_ie(pbuf, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &len);
	

	return len;
	
}

u32 build_prov_disc_request_p2p_ie(struct wifidirect_info *pwdinfo, u8 *pbuf)
{
	u8 p2pie[ MAX_P2P_IE_LEN] = { 0x00 };
	u32 len=0, p2pielen = 0;	

	//	P2P OUI
	p2pielen = 0;
	p2pie[ p2pielen++ ] = 0x50;
	p2pie[ p2pielen++ ] = 0x6F;
	p2pie[ p2pielen++ ] = 0x9A;
	p2pie[ p2pielen++ ] = 0x09;	//	WFA P2P v1.0

	//	Commented by Albert 20110301
	//	According to the P2P Specification, the provision discovery request frame should contain 3 P2P attributes
	//	1. P2P Capability
	//	2. Device Info
	//	3. Group ID ( When joining an operating P2P Group ), ignore here.

	//	P2P Capability ATTR
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_CAPABILITY;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 0x0002 );
	p2pielen += 2;

	//	Value:
	//	Device Capability Bitmap, 1 byte
	//	Be able to participate in additional P2P Groups and
	//	support the P2P Invitation Procedure
	p2pie[ p2pielen++ ] = P2P_DEVCAP_INVITATION_PROC;
	
	//	Group Capability Bitmap, 1 byte
	p2pie[ p2pielen++ ] = 0x00;


	//	Device Info ATTR
	//	Type:
	p2pie[ p2pielen++ ] = P2P_ATTR_DEVICE_INFO;

	//	Length:
	//	21 -> P2P Device Address (6bytes) + Config Methods (2bytes) + Primary Device Type (8bytes) 
	//	+ NumofSecondDevType (1byte) + WPS Device Name ID field (2bytes) + WPS Device Name Len field (2bytes)
	*(u16*) ( p2pie + p2pielen ) = cpu_to_le16( 21 + pwdinfo->device_name_len );
	p2pielen += 2;

	//	Value:
	//	P2P Device Address
	_memcpy( p2pie + p2pielen, pwdinfo->device_addr, ETH_ALEN );
	p2pielen += ETH_ALEN;

	//	Config Method
	//	This field should be big endian. Noted by P2P specification.
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_CONFIG_METHOD_DISPLAY );
	p2pielen += 2;

	//	Primary Device Type
	//	Category ID
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_PDT_CID_RTK_WIDI );
	p2pielen += 2;

	//	OUI
	*(u32*) ( p2pie + p2pielen ) = cpu_to_be32( WPSOUI );
	p2pielen += 4;

	//	Sub Category ID
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_PDT_SCID_RTK_DMP );
	p2pielen += 2;

	//	Number of Secondary Device Types
	p2pie[ p2pielen++ ] = 0x00;	//	No Secondary Device Type List

	//	Device Name
	//	Type:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( WPS_ATTR_DEVICE_NAME );
	p2pielen += 2;

	//	Length:
	*(u16*) ( p2pie + p2pielen ) = cpu_to_be16( pwdinfo->device_name_len );
	p2pielen += 2;

	//	Value:
	_memcpy( p2pie + p2pielen, pwdinfo->device_name, pwdinfo->device_name_len );
	p2pielen += pwdinfo->device_name_len;

	pbuf = set_ie(pbuf, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &len);
	

	return len;
	
}


u32 build_assoc_resp_p2p_ie(struct wifidirect_info *pwdinfo, u8 *pbuf, u8 status_code)
{
	u8 p2pie[ MAX_P2P_IE_LEN] = { 0x00 };
	u32 len=0, p2pielen = 0;	

	//	P2P OUI
	p2pielen = 0;
	p2pie[ p2pielen++ ] = 0x50;
	p2pie[ p2pielen++ ] = 0x6F;
	p2pie[ p2pielen++ ] = 0x9A;
	p2pie[ p2pielen++ ] = 0x09;	//	WFA P2P v1.0

	// According to the P2P Specification, the Association response frame should contain 2 P2P attributes
	//	1. Status
	//	2. Extended Listen Timing (optional)


	//	Status ATTR
	p2pielen += set_p2p_attr_content(&p2pie[p2pielen], P2P_ATTR_STATUS, 1, &status_code);


	// Extended Listen Timing ATTR
	//	Type:
	//	Length:
	//	Value:


	pbuf = set_ie(pbuf, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &len);
	
	return len;
	
}

u32 build_deauth_p2p_ie(struct wifidirect_info *pwdinfo, u8 *pbuf)
{
	u32 len=0;
	
	return len;
}

u32 process_probe_req_p2p_ie(struct wifidirect_info *pwdinfo, u8 *pframe, uint len)
{	
	u8 *p;
	u32 ret=_FALSE;
	u8	p2pie[ MAX_P2P_IE_LEN ] = { 0xFF };
	u32	p2pielen = 0;
	int ssid_len=0;

	//	Added comments by Albert 20100906
	//	There are several items we should check here.
	//	1. This probe request frame must contain the P2P IE. (Done)
	//	2. This probe request frame must contain the wildcard SSID. (Done)
	//	3. Wildcard BSSID. (Todo)
	//	4. Destination Address. ( Done in mgt_dispatcher function )
	//	5. Requested Device Type in WSC IE. (Todo)
	//	6. Device ID attribute in P2P IE. (Todo)

	p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _SSID_IE_, (int *)&ssid_len,
			len - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);	

	if((pwdinfo->role == P2P_ROLE_DEVICE) || (pwdinfo->role == P2P_ROLE_GO))
	{
		if(get_p2p_ie( pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_ , len - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_ , p2pie, &p2pielen))
		{
			DBG_871X( "[%s] Got P2P IE \n", __FUNCTION__ );
		
			if ( (p != NULL) && _memcmp( ( void * ) ( p+2 ), ( void * ) pwdinfo->p2p_wildcard_ssid , 7 ))
			{
				//todo:
				//Check Requested Device Type attributes in WSC IE.
				//Check Device ID attribute in P2P IE

				DBG_871X( "[%s] P2P SSID Match!\n", __FUNCTION__ );
				
				ret = _TRUE;			
			}		

		}
		else
		{
			//non -p2p device
		}
		
	}	

	
	return ret;
	
}

u32 process_assoc_req_p2p_ie(struct wifidirect_info *pwdinfo, u8 *p2p_ie, uint p2p_ielen, struct sta_info *psta)
{
	u8 status_code = P2P_STATUS_SUCCESS;
	u8 *pbuf, *pattr_content=NULL;
	u32 attr_contentlen = 0;
	u16 cap_attr=0;

	if(pwdinfo->role != P2P_ROLE_GO)
		return P2P_STATUS_FAIL_REQUEST_UNABLE;

	//Check P2P Capability ATTR
	if(get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_CAPABILITY, (u8*)&cap_attr, (uint*)&attr_contentlen)==_FALSE)
		return P2P_STATUS_FAIL_INVALID_PARAM;

	cap_attr = le16_to_cpu(cap_attr);
	psta->dev_cap = cap_attr&0xff;


	//Check Extended Listen Timing ATTR
	

	//Check P2P Device Info ATTR
	if(get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_DEVICE_INFO, NULL, (uint*)&attr_contentlen)==_TRUE)
	{
		pattr_content = pbuf = rtw_zmalloc(attr_contentlen);
		if(pattr_content)
		{
			u8 num_of_secdev_type;
			u16 dev_name_len;
			
		
			get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_DEVICE_INFO , pattr_content, (uint*)&attr_contentlen);

			_memcpy(psta->dev_addr, 	pattr_content, ETH_ALEN);//P2P Device Address
			
			pattr_content += ETH_ALEN;

			_memcpy(&psta->config_methods, pattr_content, 2);//Config Methods
			psta->config_methods = be16_to_cpu(psta->config_methods);

			pattr_content += 2;

			_memcpy(psta->primary_dev_type, pattr_content, 8);

			pattr_content += 8;

			num_of_secdev_type = *pattr_content;
			pattr_content += 1;

			if(num_of_secdev_type==0)
			{
				psta->num_of_secdev_type = 0;
			}
			else
			{
				u32 len;

				psta->num_of_secdev_type = num_of_secdev_type;

				len = (sizeof(psta->secdev_types_list)<(num_of_secdev_type*8)) ? (sizeof(psta->secdev_types_list)) : (num_of_secdev_type*8);

				_memcpy(psta->secdev_types_list, pattr_content, len);

				pattr_content += (num_of_secdev_type*8);
			}


			//dev_name_len = attr_contentlen - ETH_ALEN - 2 - 8 - 1 - (num_of_secdev_type*8);
			psta->dev_name_len=0;
			if(WPS_ATTR_DEVICE_NAME == be16_to_cpu(*(u16*)pattr_content))
			{				
				dev_name_len = be16_to_cpu(*(u16*)(pattr_content+2));

				psta->dev_name_len = (sizeof(psta->dev_name)<dev_name_len) ? sizeof(psta->dev_name):dev_name_len;

				_memcpy(psta->dev_name, pattr_content+4, psta->dev_name_len);
			}

			rtw_mfree(pbuf, attr_contentlen);

		}

	}
	else
	{
		status_code =  P2P_STATUS_FAIL_INVALID_PARAM;
	}

	return status_code;

}

u32 process_p2p_devdisc_req(struct wifidirect_info *pwdinfo, u8 *pframe, uint len)
{
	u8 *frame_body;
	u8 status, dialogToken;
	struct sta_info *psta = NULL;
	_adapter *padapter = pwdinfo->padapter;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8	p2p_ie[ MAX_P2P_IE_LEN ] = { 0xFF };
	u32	p2p_ielen = 0;	
	
	frame_body = (unsigned char *)(pframe + sizeof(struct ieee80211_hdr_3addr));

	dialogToken = frame_body[7];
	status = P2P_STATUS_FAIL_UNKNOWN_P2PGROUP;
		
	if ( get_p2p_ie( frame_body + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, p2p_ie, &p2p_ielen ) )
	{
		u8 groupid[ 38 ] = { 0x00 };
		u8 dev_addr[ETH_ALEN] = { 0x00 };		
		u32	attr_contentlen = 0;

		if(get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_GROUP_ID, groupid, &attr_contentlen))
		{
			if(_memcmp(pwdinfo->device_addr, groupid, ETH_ALEN) && 
				_memcmp(pwdinfo->p2p_group_ssid, groupid+ETH_ALEN, sizeof(pwdinfo->p2p_group_ssid)))
			{
				attr_contentlen=0;
				if(get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_DEVICE_ID, dev_addr, &attr_contentlen))
				{
					_list	*phead, *plist;					
					phead = &pstapriv->asoc_list;
					plist = get_next(phead);

					//look up sta asoc_queue
					while ((end_of_queue_search(phead, plist)) == _FALSE)	
					{		
						psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		
						plist = get_next(plist);

						if(psta->is_p2p_device && (psta->dev_cap&P2P_DEVCAP_CLIENT_DISCOVERABILITY) &&
							_memcmp(psta->dev_addr, dev_addr, ETH_ALEN))
						{

							//issue GO Discoverability Request
							issue_group_disc_req(pwdinfo, psta->hwaddr);
							
							status = P2P_STATUS_SUCCESS;
							
							break;
						}
						else
						{
							status = P2P_STATUS_FAIL_INFO_UNAVAILABLE;
						}
		
					}				
					
				}
				else
				{
					status = P2P_STATUS_FAIL_INVALID_PARAM;
				}

			}
			else
			{
				status = P2P_STATUS_FAIL_INVALID_PARAM;
			}
			
		}	
		
	}

	
	//issue Device Discoverability Response
	issue_p2p_devdisc_resp(pwdinfo, GetAddr2Ptr(pframe), status, dialogToken);
	
	
	return (status==P2P_STATUS_SUCCESS) ? _TRUE:_FALSE;
	
}

u32 process_p2p_devdisc_resp(struct wifidirect_info *pwdinfo, u8 *pframe, uint len)
{
	return _TRUE;
}

u8 process_p2p_provdisc_req(struct wifidirect_info *pwdinfo,  u8 *pframe)
{
	u8 *frame_body;

	frame_body = (pframe + sizeof(struct ieee80211_hdr_3addr));

	if(pwdinfo->role == P2P_ROLE_GO)
	{
		//todo: check P2P IE - P2P Group ID attr,...
		
	
		issue_p2p_provision_resp( pwdinfo, GetAddr2Ptr(pframe), frame_body);
	}
	else
	{
		pwdinfo->p2p_state = P2P_STATE_RX_PROVISION_DIS_REQ;
		issue_p2p_provision_resp( pwdinfo, GetAddr2Ptr(pframe), frame_body);

		//	Re-enter into the listen state
		//	It means the driver will clear this P2P_STATE_RX_PROVISION_DIS_REQ after 8 seconds
		_set_timer( &pwdinfo->enter_listen_state_timer, 8000 );		
	}

	return _TRUE;
	
}
	
u8 process_p2p_provdisc_resp(struct wifidirect_info *pwdinfo,  u8 *pframe)
{

	return _TRUE;
}



u8 process_p2p_group_negotation_req( struct wifidirect_info *pwdinfo, u8 *pframe, uint len )
{
	u8	result = P2P_STATUS_SUCCESS;
	u32	p2p_ielen, wps_ielen;
	u8	p2p_ie[ 255 ];

	
	if ( get_wps_ie_p2p( pframe + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, NULL, &wps_ielen) )
	{

	}
	else
	{
		printk( "[%s] WPS IE not Found!!\n", __FUNCTION__ );
		result = P2P_STATUS_FAIL_INCOMPATIBLE_PARAM;
	}
					
	if ( get_p2p_ie( pframe + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, p2p_ie, &p2p_ielen ) )
	{
		u8	attr_content = 0x00;
		u32	attr_contentlen = 0;						

		if ( result != P2P_STATUS_SUCCESS )
		{
			pwdinfo->p2p_state = P2P_STATE_GONEGO_FAIL;
		}
		else
		{
			pwdinfo->p2p_state = P2P_STATE_GONEGO_ING;
		}
						
		get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_GO_INTENT , &attr_content, &attr_contentlen);
		printk( "[%s] GO Intent = %d, tie = %d\n", __FUNCTION__, attr_content >> 1, attr_content & 0x01 );
		pwdinfo->peer_intent = attr_content;	//	include both intent and tie breaker values.

		if ( result == P2P_STATUS_SUCCESS )
		{
			if ( pwdinfo->intent == ( pwdinfo->peer_intent >> 1 ) )
			{
				//	Try to match the tie breaker value
				if ( pwdinfo->intent == P2P_MAX_INTENT )
				{
					pwdinfo->role = P2P_ROLE_DEVICE;
					result = P2P_STATUS_FAIL_BOTH_GOINTENT_15;
				}
				else
				{
					if ( attr_content & 0x01 )
					{
						pwdinfo->role = P2P_ROLE_CLIENT;
					}
					else
					{
						pwdinfo->role = P2P_ROLE_GO;
					}
				}							
			}
			else if ( pwdinfo->intent > ( pwdinfo->peer_intent >> 1 ) )
			{
				pwdinfo->role = P2P_ROLE_GO;
			}
			else
			{
				pwdinfo->role = P2P_ROLE_CLIENT;
			}
		}
						
		attr_contentlen = 0;
		get_p2p_attr_content( p2p_ie, p2p_ielen, P2P_ATTR_INTENTED_IF_ADDR, pwdinfo->p2p_peer_interface_addr, &attr_contentlen );
						
		if ( attr_contentlen != ETH_ALEN )
		{
			_memset( pwdinfo->p2p_peer_interface_addr, 0x00, ETH_ALEN );
		}
	}

	return( result );
}

u8 process_p2p_group_negotation_resp( struct wifidirect_info *pwdinfo, u8 *pframe, uint len )
{
	u8	result = P2P_STATUS_SUCCESS;
	u32	p2p_ielen, wps_ielen;
	u8	p2p_ie[ 255 ];


	//	Be able to know which one is the P2P GO and which one is P2P client.
					
	if ( get_wps_ie_p2p( pframe+ _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, NULL, &wps_ielen) )
	{

	}
	else
	{
		printk( "[%s] WPS IE not Found!!\n", __FUNCTION__ );
		result = P2P_STATUS_FAIL_INCOMPATIBLE_PARAM;
	}
					
	if ( get_p2p_ie( pframe + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, p2p_ie, &p2p_ielen ) )
	{
		u8	attr_content = 0x00;
		u32	attr_contentlen = 0;

		get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_STATUS, &attr_content, &attr_contentlen);
		if ( attr_contentlen == 1 )
		{
			printk( "[%s] Status = %d\n", __FUNCTION__, attr_content );
			if ( attr_content == P2P_STATUS_SUCCESS )
			{
				attr_content = 0x00;
				attr_contentlen = 0;
				get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_GO_INTENT , &attr_content, &attr_contentlen);
				printk( "[%s] GO Intent = %d, tie = %d\n", __FUNCTION__, attr_content >> 1, attr_content & 0x01 );
				pwdinfo->peer_intent = attr_content;	//	include both intent and tie breaker values.

				if ( pwdinfo->intent == ( pwdinfo->peer_intent >> 1 ) )
				{
					//	Try to match the tie breaker value
					if ( pwdinfo->intent == P2P_MAX_INTENT )
					{
						pwdinfo->role = P2P_ROLE_DEVICE;
						result = P2P_STATUS_FAIL_BOTH_GOINTENT_15;
					}
					else
					{
						if ( attr_content & 0x01 )
						{
							pwdinfo->role = P2P_ROLE_CLIENT;
						}
						else
						{
							pwdinfo->role = P2P_ROLE_GO;
						}
					}							
				}
				else if ( pwdinfo->intent > ( pwdinfo->peer_intent >> 1 ) )
				{
					pwdinfo->role = P2P_ROLE_GO;
				}
				else
				{
					pwdinfo->role = P2P_ROLE_CLIENT;
				}

				if ( pwdinfo->role == P2P_ROLE_CLIENT )
				{
					u8	operatingch_info[5] = { 0x00 };
					
					attr_contentlen = 0;
					get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_OPERATING_CH, operatingch_info, &attr_contentlen);
					printk( "[%s] Peer's operating channel = %d\n", __FUNCTION__, operatingch_info[4] );
					pwdinfo->peer_operating_ch = operatingch_info[4];
				}
			}
			else
			{
				pwdinfo->role = P2P_ROLE_DEVICE;
				pwdinfo->p2p_state = P2P_STATE_GONEGO_FAIL;
			}
		}
		else
		{
			//	the length of P2P STATUS ATTRIBUTE is not 1, doesn't match with the P2P specification.
			pwdinfo->role = P2P_ROLE_DEVICE;
			pwdinfo->p2p_state = P2P_STATE_GONEGO_FAIL;
		}
	}

	return( result );

}

u8 process_p2p_group_negotation_confirm( struct wifidirect_info *pwdinfo, u8 *pframe, uint len )
{
	u32	p2p_ielen;
	u8	p2p_ie[ 255 ];
	u8	result = P2P_STATUS_SUCCESS;
	
	if ( get_p2p_ie( pframe + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, p2p_ie, &p2p_ielen ) )
	{
		u8	attr_content = 0x00, operatingch_info[5] = { 0x00 };
		u8	groupid[ 38 ] = { 0x00 };
		u32	attr_contentlen = 0;


		pwdinfo->negotiation_dialog_token = 1;
		get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_STATUS, &attr_content, &attr_contentlen);
		if ( attr_contentlen == 1 )
		{
			printk( "[%s] Status = %d\n", __FUNCTION__, attr_content );
			result = attr_content;

			if ( attr_content == P2P_STATUS_SUCCESS )
			{
				u8	bcancelled = 0;
																
				_cancel_timer( &pwdinfo->enter_listen_state_timer, &bcancelled );

				//	Commented by Albert 20100911
				//	Todo: Need to handle the case which both Intents are the same.
				pwdinfo->p2p_state = P2P_STATE_GONEGO_OK;								
				if ( ( pwdinfo->intent ) > ( pwdinfo->peer_intent >> 1 ) )
				{
					pwdinfo->role = P2P_ROLE_GO;
				}
				else if ( ( pwdinfo->intent ) < ( pwdinfo->peer_intent >> 1 ) )
				{
					pwdinfo->role = P2P_ROLE_CLIENT;
				}
				else
				{
					//	Have to compare the Tie Breaker
					if ( pwdinfo->peer_intent & 0x01 )
					{
						pwdinfo->role = P2P_ROLE_CLIENT;
					}
					else
					{
						pwdinfo->role = P2P_ROLE_GO;
					}
				}								
				get_p2p_attr_content( p2p_ie, p2p_ielen, P2P_ATTR_GROUP_ID, groupid, &attr_contentlen);
				printk( "[%s] Ssid = %s, ssidlen = %d\n", __FUNCTION__, &groupid[ETH_ALEN], strlen(&groupid[ETH_ALEN]) );

				attr_contentlen = 0;
				get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_OPERATING_CH, operatingch_info, &attr_contentlen);
				printk( "[%s] Peer's operating channel = %d\n", __FUNCTION__, operatingch_info[4] );
				pwdinfo->peer_operating_ch = operatingch_info[4];
			}
			else
			{
				pwdinfo->role = P2P_ROLE_DEVICE;
				pwdinfo->p2p_state = P2P_STATE_GONEGO_FAIL;
			}
		}
	}

	return( result );
}

u8 process_p2p_presence_req(struct wifidirect_info *pwdinfo, u8 *pframe, uint len)
{
	u8 *frame_body;	
	u8 dialogToken=0;
	u8 status = P2P_STATUS_SUCCESS;
	
	frame_body = (unsigned char *)(pframe + sizeof(struct ieee80211_hdr_3addr));

	dialogToken = frame_body[6];

	//todo: check NoA attribute

	issue_p2p_presence_resp(pwdinfo, GetAddr2Ptr(pframe), status, dialogToken);

	return _TRUE;
}

void process_p2p_ps_ie(PADAPTER padapter, u8 *IEs, u32 IELength)
{
	u32	p2p_ielen = 0;
	u32	attr_contentlen = 0;
	u8	p2p_ie[ MAX_P2P_IE_LEN] = { 0x00 };
	u8	noa_attr[MAX_P2P_IE_LEN] = { 0x00 };// NoA length should be n*(13) + 2
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8	noa_offset, noa_num;

_func_enter_;

	if(get_p2p_ie( IEs, IELength, p2p_ie, &p2p_ielen))
	{
		// Get Notice of Absence IE.
		if(get_p2p_attr_content( p2p_ie, p2p_ielen, P2P_ATTR_NOA, noa_attr, &attr_contentlen))
		{
			pwdinfo->noa_index = noa_attr[0]; // if index change driver should update NoA descriptor value.
			pwdinfo->opp_ps = noa_attr[1] >> 7;
			pwdinfo->ctwindow = noa_attr[1] & 0x7F;

			noa_offset = 2;
			noa_num = 0;
			// NoA length should be n*(13) + 2
			if(attr_contentlen > 2)
			{
				while(noa_offset < attr_contentlen)
				{
					//_memcpy(&wifidirect_info->noa_count[noa_num], &noa_attr[noa_offset], 1);
					pwdinfo->noa_count[noa_num] = noa_attr[noa_offset];
					noa_offset += 1;

					_memcpy(&pwdinfo->noa_duration[noa_num], &noa_attr[noa_offset], 4);
					noa_offset += 4;

					_memcpy(&pwdinfo->noa_interval[noa_num], &noa_attr[noa_offset], 4);
					noa_offset += 4;

					_memcpy(&pwdinfo->noa_start_time[noa_num], &noa_attr[noa_offset], 4);					
					noa_offset += 4;

					noa_num++;
				}
			}
			pwdinfo->noa_num = noa_num;

			if( (pwdinfo->opp_ps == 1) || (pwdinfo->noa_num > 0) )
				p2p_ps_wk_cmd(padapter, P2P_PS_ENABLE, 1);
		}
	}

_func_exit_;
}

void p2p_ps_wk_hdl(_adapter *padapter, u8 p2p_ps_state)
{
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	
_func_enter_;

	padapter->HalFunc.SetHwRegHandler(padapter, HW_VAR_H2C_FW_P2P_PS_OFFLOAD, (u8 *)(&p2p_ps_state));

	// clear P2P SW status
	if(p2p_ps_state == P2P_PS_DISABLE)
	{
		pwdinfo->noa_index = 0;
		pwdinfo->ctwindow = 0;
		pwdinfo->opp_ps = 0;
		pwdinfo->noa_num = 0;
		pwdinfo->p2p_ps_enable = _FALSE;
	}

_func_exit_;
}

u8 p2p_ps_wk_cmd(_adapter*padapter, u8 p2p_ps_state, u8 enqueue)
{
	struct cmd_obj	*ph2c;
	struct drvextra_cmd_parm	*pdrvextra_cmd_parm;
	struct wifidirect_info	*pwdinfo= &(padapter->wdinfo);
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;
	
_func_enter_;

	if ( (pwdinfo->p2p_state == P2P_STATE_NONE) ||
		( pwdinfo->p2p_ps == p2p_ps_state ) )
	{
		return res;
	}

	// driver only perform p2p ps when GO have Opp_Ps or NoA
	if( (pwdinfo->opp_ps == 1) || (pwdinfo->noa_num > 0) )
	{
		pwdinfo->p2p_ps = p2p_ps_state;
	
		if(enqueue)
		{
			ph2c = (struct cmd_obj*)rtw_zmalloc(sizeof(struct cmd_obj));	
			if(ph2c==NULL){
				res= _FAIL;
				goto exit;
			}
			
			pdrvextra_cmd_parm = (struct drvextra_cmd_parm*)rtw_zmalloc(sizeof(struct drvextra_cmd_parm)); 
			if(pdrvextra_cmd_parm==NULL){
				rtw_mfree((unsigned char *)ph2c, sizeof(struct cmd_obj));
				res= _FAIL;
				goto exit;
			}

			pdrvextra_cmd_parm->ec_id = P2P_PS_WK_CID;
			pdrvextra_cmd_parm->sz = p2p_ps_state;
			pdrvextra_cmd_parm->pbuf = NULL;

			init_h2fwcmd_w_parm_no_rsp(ph2c, pdrvextra_cmd_parm, GEN_CMD_CODE(_Set_Drv_Extra));

			enqueue_cmd_ex(pcmdpriv, ph2c);
		}
		else
		{
			p2p_ps_wk_hdl(padapter, p2p_ps_state);
		}
	}
	
exit:
	
_func_exit_;

	return res;

}

#endif

