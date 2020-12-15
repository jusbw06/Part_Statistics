#include "TdmsMetaData.h"
#include "TdmsParser.h"
#include "TdmsObject.h"
#include "TdmsGroup.h"
#include "TdmsChannel.h"
#include "TdmsSegment.h"

using namespace std;

TdmsMetaData::TdmsMetaData(TdmsParser* parser, bool verbose):
d_verbose(verbose),
d_objectCount(0),
d_channel_cnt(0),
d_chunks(0),
d_parser(parser)
{
	read(parser->fileStream(), verbose);
}

TdmsMetaData::~TdmsMetaData()
{
	d_objects.clear();

	if (!d_chunk_raw_data_pos.empty())
		vector<unsigned long long>().swap(d_chunk_raw_data_pos);
}

void TdmsMetaData::read(std::iendianfstream& file, const bool verbose)
{
	file >> d_objectCount;
	if (verbose)
		print();

	for (unsigned int i = 0; i < d_objectCount; i++)
		readObject(file, verbose);

	if (verbose)
		printf ("\tRaw data chunk size: %d\n", (unsigned int)getRawDataChunkSize());
}

void TdmsMetaData::readObject(std::iendianfstream& file, const bool verbose)
{
	TdmsObject *o = new TdmsObject(file, verbose);
	d_objects.push_back(o);

	if (!o)
		return;

	if (o->isRoot()){
		d_parser->setProperties(o->getProperties());
		return;
	}

	std::string path = o->getPath();

	if (o->isGroup()){
		TdmsGroup *group = d_parser->getGroup(path);
		if (!group){
			group = new TdmsGroup(path);
			group->setObject(o);
			d_parser->addGroup(group);
			if (verbose)
				printf("NEW GROUP: %s\n", path.c_str());
		}
	} else {
		int islash = path.find("'/'", 1) + 1;
		std::string channelName = path.substr(islash);
		std::string groupName = path.substr(0, islash);

		TdmsGroup *group = d_parser->getGroup(groupName);
		if (!group){
			group = new TdmsGroup(groupName);
			d_parser->addGroup(group);
			if (verbose)
				printf("NEW GROUP: %s\n", path.c_str());
		}

		TdmsChannel *channel = group->getChannel(channelName);
		if (channel == 0){
			channel = new TdmsChannel(channelName, file);
			channel->setStoreValues(d_parser->getStoreValues());
			channel->setProperties(o->getProperties());
			channel->setDimension(o->getDimension());

			unsigned int type = o->getDataType();
			channel->setDataType(type);
			channel->setTypeSize((type == TdmsChannel::tdsTypeString) ? (unsigned int)o->getBytesCount() : TdmsObject::dataTypeSize(type));

			group->addChannel(channel);
			if (verbose)
				printf("NEW CHANNEL: %s\n", channelName.c_str());
		}

		std::map<std::string, std::string> properties = o->getProperties();
		if (!properties.empty())
			channel->addProperties(properties);

		channel->setValuesCount(o->getValuesCount());
		o->setChannel(channel);
	}
}

TdmsChannel* TdmsMetaData::getChannel(TdmsObject *obj)
{
	if (!obj || obj->isRoot() || obj->isGroup())
		return 0;

	std::string path = obj->getPath();
	int islash = path.find("'/'", 1) + 1;
	std::string channelName = path.substr(islash);
	std::string groupName = path.substr(0, islash);

	TdmsGroup *group = d_parser->getGroup(groupName);
	if (!group)
		return 0;

	return group->getChannel(channelName);
}

unsigned int TdmsMetaData::getChannelCount()
{
	if (d_channel_cnt == 0){
		for (TdmsObjectList::iterator object = d_objects.begin();  object != d_objects.end(); ++object){
			TdmsObject *obj = (*object);
			if (!obj)
				continue;
			if (obj->hasRawData()) // only channels have raw data - gbzs
				d_channel_cnt++;
		}
	}
	return d_channel_cnt;
}

unsigned long long TdmsMetaData::getRawDataChunkSize()
{
	unsigned long long chunk_size = 0;
	for (TdmsObjectList::iterator object = d_objects.begin();  object != d_objects.end(); ++object){
		TdmsObject *obj = (*object);
		if (!obj)
			continue;

		if (obj->hasRawData())
			chunk_size += obj->getChannelSize();
	}
	return chunk_size;
}

unsigned int TdmsMetaData::getInterleavedChannelCount(TdmsObject *obj, TdmsChannel *channel)
{
	if (!channel || !obj)
		return 0;

	TdmsGroup *g = channel->getGroup();
	if (!g)
		return 0;

	std::vector<FormatChangingScaler> formatScalersVector = obj->formatScalersVector();
	if (formatScalersVector.empty())
		return 0;

	unsigned int cnt = 0, rawBufferIndex = formatScalersVector.front().rawBufferIndex;

	std::vector<TdmsChannel*> channels = g->getChannels();
	for (unsigned int i = 0; i < channels.size(); i++){
		TdmsChannel *ch = channels.at(i);
		if (ch == channel)
			continue;
		if (ch){
			for (TdmsObjectList::iterator object = d_objects.begin(); object != d_objects.end(); ++object){
				TdmsObject *o = (*object);
				if (!o)
					continue;
				if (o->getChannel() == ch){
					std::vector<FormatChangingScaler> fsv = o->formatScalersVector();
					if (fsv.empty())
						return 0;

					if (rawBufferIndex == fsv.front().rawBufferIndex)
						cnt++;
					break;
				}
			}
		}
	}
	return cnt;
}

TdmsObject* TdmsMetaData::readRawData(unsigned long long total_chunk_size, TdmsObject *prevObject)
{
	if (!total_chunk_size)
		return 0;

	TdmsObject *lastObj = 0;
	unsigned long long fileSize = d_parser->fileSize();
	std::iendianfstream& file = d_parser->fileStream();

	long long chunk_size = getRawDataChunkSize();
	if (!chunk_size)
		return 0;

	d_chunks = total_chunk_size/chunk_size;
	bool interleavedData = d_parser->currentSegmentInterleaved();
	if (d_verbose)
		printf("\tNumber of data chunks: %d\n\tData is interleaved: %d\n", d_chunks, interleavedData);

	if (interleavedData){
		for (TdmsObjectList::iterator object = d_objects.begin(); object != d_objects.end(); ++object){
			TdmsObject *obj = (*object);
			if (!obj)
				continue;

			TdmsChannel *channel = obj->getChannel();
			if (!channel)
				channel = getChannel(obj);
			if (channel)
				channel->setInterleavedFlag(interleavedData && getInterleavedChannelCount(obj, channel));
		}
	}

	for (unsigned int i = 0; i < d_chunks; i++){
		if (d_verbose)
			printf("\nReading data chunk %d...\n", i + 1);
		for (TdmsObjectList::iterator object = d_objects.begin(); object != d_objects.end(); ++object){
			TdmsObject *obj = (*object);
			if (!obj)
				continue;

			if (obj->hasDAQmxData()){
				TdmsChannel *channel = obj->getChannel();
				if (!channel)
					channel = getChannel(obj);
				if (channel){
					d_chunk_raw_data_pos.push_back((unsigned long long)file.tellg());
					obj->readDAQmxData(channel, d_verbose);
				}
			} else if (obj->hasRawData()){
				if (!obj->getRawDataIndex())
					obj->setRawDataInfo(prevObject);
				else
					lastObj = obj;

				TdmsChannel *channel = obj->getChannel();
				if (!channel)
					channel = getChannel(obj);
				if (channel){
					d_chunk_raw_data_pos.push_back((unsigned long long)file.tellg());
					obj->readRawData(total_chunk_size, channel);
				}
			}

			if ((unsigned long long)file.tellg() >= fileSize)
				break;
		}
	}

	return lastObj;
}

TdmsObject* TdmsMetaData::getObject(TdmsChannel *channel)
{
	if (!channel)
		return 0;

	for (TdmsObjectList::iterator object = d_objects.begin(); object != d_objects.end(); ++object){
		TdmsObject *obj = (*object);
		if (obj){
			TdmsChannel *ch = obj->getChannel();
			if (!ch)
				ch = getChannel(obj);
			if (ch && (channel == ch))
				return obj;
		}
	}

	return 0;
}

TdmsObject* TdmsMetaData::readChannelRawData(TdmsChannel *channel, TdmsSegment *seg, TdmsObject *prevObject)
{
	if (!channel || !seg || !d_chunks)
		return 0;

	TdmsObject *lastObj = 0;
	unsigned long long fileSize = d_parser->fileSize();
	std::iendianfstream& file = d_parser->fileStream();

	long long chunk_size = getRawDataChunkSize();
	if (!chunk_size)
		return 0;

	bool interleavedData =seg->getInterleavedFlag();
	if (d_verbose)
		printf("\tNumber of data chunks: %d\n\tData is interleaved: %d\n", d_chunks, interleavedData);

	unsigned int chIndex = channel->getIndexWithinGroup();
	TdmsObject *obj = getObject(channel);
	if (!obj)
		return 0;

	unsigned int channelCnt = getChannelCount();
	if (d_verbose)
		printf("\tChannel index: %d/%d nvalues: %lld\n", chIndex, channelCnt, obj->getValuesCount());

	channel->setValuesCount(obj->getValuesCount());

	if (interleavedData && obj->hasDAQmxData())
		channel->setInterleavedFlag(interleavedData && getInterleavedChannelCount(obj, channel));

	for (unsigned int i = 0; i < d_chunks; i++){
		if (d_verbose)
			printf("\nReading data chunk %d...\n", i + 1);

		unsigned int posIndex = i*channelCnt + chIndex;
		if (posIndex >= d_chunk_raw_data_pos.size())
			return 0;

		unsigned long long pos = d_chunk_raw_data_pos.at(posIndex);
		file.seekg(pos);
		if (d_verbose)
			printf("\nReading channel data begins @ 0x%llX...\n", pos);

		if (obj->hasDAQmxData())
			obj->readDAQmxData(channel, d_verbose);
		else if (obj->hasRawData()){
			if (!obj->getRawDataIndex())
				obj->setRawDataInfo(prevObject);
			else
				lastObj = obj;
			obj->readRawData(seg->getRawDataChunkSize(), channel);
		}

		if ((unsigned long long)file.tellg() >= fileSize)
			break;
	}

	return lastObj;
}

void TdmsMetaData::print() const
{
	std::cout << "\nRead meta data" << std::endl;
	std::cout << "  Contains " << d_objectCount << " objects." << std::endl;
}
