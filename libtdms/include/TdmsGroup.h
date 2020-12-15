#ifndef TDMS_GROUP_H
#define TDMS_GROUP_H

#include <string>
#include <vector>
class TdmsChannel;
class TdmsObject;

class TdmsGroup
{
public:
	typedef std::vector<TdmsChannel*> TdmsChannelSet;

	TdmsGroup(const std::string&);
	~TdmsGroup();

	void addChannel(TdmsChannel*);

	const std::string& getName() const{return d_name;}
	unsigned int getGroupSize() const {return d_channels.size();}
	unsigned int getMaxValuesCount() const;
	const TdmsChannelSet& getChannels() const{return d_channels;}
	TdmsChannel* getChannel(const std::string &);
	TdmsChannel* getChannel(unsigned int) const;
	unsigned int getChannelIndex(TdmsChannel*) const;

	void setObject(TdmsObject *o){d_object = o;}
	TdmsObject* getObject(){return d_object;}

private:
	const std::string d_name;
	TdmsObject *d_object;
	TdmsChannelSet d_channels;
};
#endif
