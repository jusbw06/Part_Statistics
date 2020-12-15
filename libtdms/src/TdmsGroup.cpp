#include "TdmsGroup.h"
#include "TdmsChannel.h"

TdmsGroup::TdmsGroup(const std::string & name):
d_name(name),
d_object(0)
{}

TdmsGroup::~TdmsGroup()
{
	unsigned int channelCnt = d_channels.size();
	if (channelCnt){
		for (unsigned int i = 0; i < channelCnt; i++){
			TdmsChannel *ch = d_channels.at(i);
			if (ch)
				delete ch;
		}
		d_channels.clear();
	}
}

void TdmsGroup::addChannel(TdmsChannel* channel)
{
	if (channel)
		channel->setGroup(this);

	d_channels.push_back(channel);
}

TdmsChannel* TdmsGroup::getChannel(const std::string &name)
{
	for (TdmsChannelSet::iterator iter = d_channels.begin(); iter != d_channels.end(); ++iter){
		if ((*iter)->getName() == name)
			return *iter;
	}
	return 0;
}

TdmsChannel* TdmsGroup::getChannel(unsigned int index) const
{
	if (index >= d_channels.size())
		return 0;

	return d_channels.at(index);
}

unsigned int TdmsGroup::getChannelIndex(TdmsChannel* channel) const
{
	unsigned int cnt = d_channels.size();
	for (unsigned int i = 0; i < cnt; i++){
		if (d_channels.at(i) == channel)
			return i;
	}
	return cnt;
}

unsigned int TdmsGroup::getMaxValuesCount() const
{
	unsigned int rows = 0, channelsCount = d_channels.size();
	for (unsigned int i = 0; i < channelsCount; i++){
		TdmsChannel *ch = getChannel(i);
		if (!ch)
			continue;
		unsigned int dataType = ch->getDataType();
		unsigned int valuesCount = ((dataType == TdmsChannel::tdsTypeString) || (dataType == TdmsChannel::tdsTypeTimeStamp)) ?
		ch->getStringCount() : ch->getDataCount();
		if (valuesCount > rows)
			rows = valuesCount;
	}
	return rows;
}
