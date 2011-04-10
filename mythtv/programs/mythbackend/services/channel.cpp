//////////////////////////////////////////////////////////////////////////////
// Program Name: channel.cpp
// Created     : Apr. 8, 2011
//
// Copyright (c) 2011 Robert McNamara <rmcnamara@mythtv.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or at your option any later version of the LGPL.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library.  If not, see <http://www.gnu.org/licenses/>.
//
//////////////////////////////////////////////////////////////////////////////

#include <QList>

#include "channel.h"

#include "compat.h"
#include "mythversion.h"
#include "mythcorecontext.h"
#include "channelutil.h"

#include "serviceUtil.h"

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

DTC::ChannelInfoList* Channel::GetChannelInfoList( int nSourceID,
                                                   int nStartIndex,
                                                   int nCount )
{
    vector<uint> chanList;

    chanList = ChannelUtil::GetChanIDs(nSourceID);

    // ----------------------------------------------------------------------
    // Build Response
    // ----------------------------------------------------------------------

    DTC::ChannelInfoList *pChannelInfos = new DTC::ChannelInfoList();

    nStartIndex   = min( nStartIndex, (int)chanList.size() );
    nCount        = (nCount > 0) ? min( nCount, (int)chanList.size() ) : chanList.size();
    int nEndIndex = nStartIndex + nCount;

    for( int n = nStartIndex; n < nEndIndex; n++)
    {
        DTC::ChannelInfo *pChannelInfo = pChannelInfos->AddNewChannelInfo();

        int chanid = chanList.at(n);
        QString channum = ChannelUtil::GetChanNum(chanid);
        QString format, modulation, freqtable, freqid, dtv_si_std,
                xmltvid, default_authority;
        int finetune, program_number;
        uint64_t frequency;
        uint atscmajor, atscminor, transportid, networkid, mplexid;
        bool commfree = false;
        bool eit = false;
        bool visible = true;

        if (ChannelUtil::GetExtendedChannelData( nSourceID, channum, format, modulation,
                            freqtable, freqid, finetune, frequency,
                            dtv_si_std, program_number, atscmajor,
                            atscminor, transportid, networkid, mplexid,
                            commfree, eit, visible, xmltvid, default_authority ))
        {
            pChannelInfo->setChanId(chanid);
            pChannelInfo->setChanNum(channum);
            pChannelInfo->setCallSign(ChannelUtil::GetCallsign(chanid));
            pChannelInfo->setIconURL(ChannelUtil::GetIcon(chanid)); //Wrong!
            pChannelInfo->setChannelName(ChannelUtil::GetServiceName(chanid));
            pChannelInfo->setMplexId(mplexid);
            pChannelInfo->setServiceId(program_number);
            pChannelInfo->setATSCMajorChan(atscmajor);
            pChannelInfo->setATSCMinorChan(atscminor);
            pChannelInfo->setFormat(format);
            pChannelInfo->setModulation(modulation);
            pChannelInfo->setFrequencyTable(freqtable);
            pChannelInfo->setFineTune(finetune);
            pChannelInfo->setFrequency(frequency);
            pChannelInfo->setFrequencyId(freqid);
            pChannelInfo->setSIStandard(dtv_si_std);
            pChannelInfo->setTransportId(transportid);
            pChannelInfo->setNetworkId(networkid);
            pChannelInfo->setChanFilters(ChannelUtil::GetVideoFilters(nSourceID, channum));
            pChannelInfo->setSourceId(nSourceID);
            pChannelInfo->setCommFree(commfree);
            pChannelInfo->setUseEIT(eit);
            pChannelInfo->setVisible(visible);
            pChannelInfo->setXMLTVID(xmltvid);
            pChannelInfo->setDefaultAuth(default_authority);
        }
    }

    pChannelInfos->setStartIndex    ( nStartIndex     );
    pChannelInfos->setCount         ( nCount          );
    pChannelInfos->setTotalAvailable( chanList.size() );
    pChannelInfos->setAsOf          ( QDateTime::currentDateTime() );
    pChannelInfos->setVersion       ( MYTH_BINARY_VERSION );
    pChannelInfos->setProtoVer      ( MYTH_PROTO_VERSION  );

    return pChannelInfos;
}

bool Channel::UpdateDBChannel( uint          MplexID,
                               uint          SourceID,
                               uint          ChannelID,
                               const QString &CallSign,
                               const QString &ChannelName,
                               const QString &ChannelNumber,
                               uint          ServiceID,
                               uint          ATSCMajorChannel,
                               uint          ATSCMinorChannel,
                               bool          UseEIT,
                               bool          visible,
                               const QString &FrequencyID,
                               const QString &Icon,
                               const QString &Format,
                               const QString &XMLTVID,
                               const QString &DefaultAuthority )
{
    bool bResult = false;

    bResult = ChannelUtil::UpdateChannel( MplexID, SourceID, ChannelID,
                             CallSign, ChannelName, ChannelNumber,
                             ServiceID, ATSCMajorChannel, ATSCMinorChannel,
                             UseEIT, !visible, false, FrequencyID,
                             Icon, Format, XMLTVID, DefaultAuthority );

    return bResult;
}

bool Channel::CreateDBChannel( uint          MplexID,
                               uint          SourceID,
                               uint          ChannelID,
                               const QString &CallSign,
                               const QString &ChannelName,
                               const QString &ChannelNumber,
                               uint          ServiceID,
                               uint          ATSCMajorChannel,
                               uint          ATSCMinorChannel,
                               bool          UseEIT,
                               bool          visible,
                               const QString &FrequencyID,
                               const QString &Icon,
                               const QString &Format,
                               const QString &XMLTVID,
                               const QString &DefaultAuthority )
{
    bool bResult = false;

    bResult = ChannelUtil::CreateChannel( MplexID, SourceID, ChannelID,
                             CallSign, ChannelName, ChannelNumber,
                             ServiceID, ATSCMajorChannel, ATSCMinorChannel,
                             UseEIT, !visible, false, FrequencyID,
                             Icon, Format, XMLTVID, DefaultAuthority );

    return bResult;
}

bool Channel::DeleteDBChannel( uint nChannelID )
{
    bool bResult = false;

    bResult = ChannelUtil::DeleteChannel( nChannelID );

    return bResult;
}