#include "TdmsParser.h"
#include "TdmsLeadIn.h"
#include "TdmsMetaData.h"
#include "TdmsGroup.h"
#include "TdmsChannel.h"
#include "TdmsObject.h"
#include "TdmsSegment.h"

using namespace std;

TdmsParser::TdmsParser(char *fileName, bool storeValues)
:	file(fileName, ios::binary),
d_file_name(fileName),
d_store_values(storeValues),
d_prev_object(0)
{}

TdmsParser::TdmsParser(const std::string& fileName, bool storeValues)
:	file(fileName.c_str(), ios::binary),
d_file_name(fileName),
d_store_values(storeValues),
d_prev_object(0)
{}

TdmsParser::~TdmsParser()
{
	clear();
	close();
}

void TdmsParser::close()
{
	file.close();
	file.clear();
}

void TdmsParser::freeMemory()
{
	unsigned int groupCnt = getGroupCount();
	for (unsigned int i = 0; i < groupCnt; i++){
		TdmsGroup *g = getGroup(i);
		if (g){
			unsigned int cnt = g->getGroupSize();
			for (unsigned int j = 0; j < cnt; j++){
				TdmsChannel *ch = g->getChannel(j);
				if (ch)
					ch->freeMemory();
			}
		}
	}
}

void TdmsParser::clear()
{
	unsigned int groupCnt = d_groups.size();
	if (groupCnt){
		for (unsigned int i = 0; i < groupCnt; i++){
			TdmsGroup *g = getGroup(i);
			if (g)
				delete g;
		}
		d_groups.clear();
	}

	unsigned int segCnt = d_segments.size();
	if (segCnt){
		for (unsigned int i = 0; i < segCnt; i++){
			TdmsSegment *s = d_segments.at(i);
			if (s)
				delete s;
		}
		d_segments.clear();
	}

	d_properties.clear();
}

bool TdmsParser::TdmsParser::reopen()
{
	file.close();
	file.clear();
	file.open(d_file_name.c_str(), ios::binary);
	return file.is_open();
}

void TdmsParser::init()
{
	file.seekg(0, std::ios::end);
	d_file_size = file.tellg();
	file.seekg(0, std::ios::beg);
	d_prev_object = 0;
}

void TdmsParser::read(const bool verbose)
{
	init();

	if (verbose){
		printf("Parsing file: '%s'\n", d_file_name.c_str());
		printf("File size is: %lld bytes (0x%llX).\n", (unsigned long long)d_file_size, (unsigned long long)d_file_size);
	}

	int seg = 0;
	unsigned long long nextSegmentOffset = 0;
	bool atEnd = false;
	while (file.tellg() < (long long)d_file_size){
		d_segments.push_back(new TdmsSegment(this));
		nextSegmentOffset = readSegment(verbose, &atEnd);
		seg++;

		if (verbose){
			printf("\nPOS after segment %d: 0x%llX\n", seg, (unsigned long long)file.tellg());
			if (atEnd)
				printf("Should skip to the end of file...File format error?!\n");
		}

		if (nextSegmentOffset >= (unsigned long long)d_file_size){
			if (verbose)
				printf("\tEnd of file is reached after segment %d!\n", seg);
			break;
		}
	}

	if (!atEnd && ((unsigned long long)file.tellg() < d_file_size)){
		if (verbose)
			printf("\nFile contains raw data at the end!\n");

		readRawData((unsigned long long)(nextSegmentOffset - file.tellg()), verbose);
	}

	if (verbose)
		printf("\nNumber of segments: %d\n", (unsigned int)d_segments.size());
}

unsigned long long TdmsParser::readSegment(const bool verbose, bool *atEnd)
{
	TdmsLeadIn *leadIn = new TdmsLeadIn(this, verbose);//read Lead In;

	TdmsSegment *seg = getCurrentSegment();
	if (seg)
		seg->setLeadIn(leadIn);

	unsigned long long posAfterLeadIn = (unsigned long long)file.tellg();

	long long nextSegmentOffset = leadIn->getNextSegmentOffset();
	if (nextSegmentOffset == -1)
		nextSegmentOffset = d_file_size;

	*atEnd = (nextSegmentOffset >= (long long)d_file_size);
	long long nextOffset = (*atEnd) ? d_file_size : nextSegmentOffset + (long long)file.tellg();
	if (verbose)
		printf("NEXT OFFSET: %d (0x%X)\n", (unsigned int)nextOffset, (unsigned int)nextOffset);

	unsigned long long offset = leadIn->getDataOffset(), total_chunk_size = 0;

	if (leadIn->hasMetaData()){
		TdmsMetaData *metaData = new TdmsMetaData(this, verbose);//read Meta Data;
		if (seg)
			seg->setMetaData(metaData);

		if (leadIn->hasRawData()){
			file.seekg(posAfterLeadIn + offset, ios_base::beg);
			if (verbose)
				printf("\tRaw data starts at POS: 0x%X\n", (unsigned int)file.tellg());

			total_chunk_size = nextSegmentOffset - offset;
			d_prev_object = metaData->readRawData(total_chunk_size, d_prev_object);

			if (verbose)
				printf("\tPOS after metadata: 0x%X\n", (unsigned int)file.tellg());
		}
	} else if (leadIn->hasRawData()){
		if (verbose)
			printf("\tSegment without metadata!\n");

		total_chunk_size = nextSegmentOffset - offset;

		file.seekg(posAfterLeadIn + offset, ios_base::beg);
		if (verbose)
			printf("\tRaw data starts at POS: 0x%X\n", (unsigned int)file.tellg());

		readRawData(total_chunk_size, verbose);
	} else if (verbose)
		printf("\tSegment without metadata or raw data!\n");

	if (seg)
		seg->setRawDataChunkSize(total_chunk_size);

	return nextOffset;
}

bool TdmsParser::readChannelData(TdmsChannel *ch, const bool verbose)
{
	if (!ch)
		return false;

	init();

	if (verbose)
		printf("\nReading channel %s data from file: '%s'\n", ch->getName().c_str(), d_file_name.c_str());

	if (!ch->reserveMemory()){
		if (verbose)
			printf("\nCouldn't allocate memory for storing data in channel %s!\n", ch->getName().c_str());
		return false;
	}

	unsigned long long nextSegmentOffset = 0;
	for (unsigned int i = 0; i < d_segments.size(); i++){
		if (verbose)
			printf("\nReading segment: %d\n", i + 1);

		TdmsSegment *seg = d_segments.at(i);
		if (seg)
			nextSegmentOffset = readChannelDataFromSegment(ch, seg, verbose);
	}

	if (verbose && (nextSegmentOffset != d_file_size))
		printf("\nFile contains raw data at the end!\n");

	return true;
}

unsigned long long TdmsParser::readChannelDataFromSegment(TdmsChannel *ch, TdmsSegment *seg, bool verbose)
{
	if (!ch || !seg)
		return 0;

	unsigned long long pos = seg->getAbsDataOffset();
	file.seekg(pos);

	unsigned long long total_chunk_size = seg->getRawDataChunkSize();
	if (verbose)
		printf("\tCursor pos: %lld (0x%llX), raw data chunk size: %lld\n", pos, pos, total_chunk_size);

	if (!total_chunk_size)
		return pos;

	const TdmsLeadIn *leadIn = seg->getLeadIn();
	if (!leadIn)
		return pos;

	if (leadIn->hasMetaData()){
		TdmsMetaData* metaData = seg->getMetaData();
		if (metaData && leadIn->hasRawData())
			d_prev_object = metaData->readChannelRawData(ch, seg, d_prev_object);
	} else if (leadIn->hasRawData()){
		if (verbose)
			printf("\tSegment without metadata!\n");
		readChannelRawDataFromSegment(ch, seg, total_chunk_size, verbose);
	} else if (verbose)
		printf("\tSegment without metadata or raw data!\n");

	return (unsigned long long)file.tellg();
}

bool TdmsParser::readChannelRawDataFromSegment(TdmsChannel *ch, TdmsSegment *seg, unsigned long long total_chunk_size, const bool verbose)
{
	if (!ch || !seg || !total_chunk_size)
		return false;

	unsigned long long pos = (unsigned long long)file.tellg();
	if (verbose){
		if (seg->getInterleavedFlag())
			printf("\tShould read %d raw interleaved data bytes from 0x%llX\n", (unsigned int)total_chunk_size, pos);
		else
			printf("\tShould read %d raw non-interleaved data bytes from 0x%llX\n", (unsigned int)total_chunk_size, pos);
	}

	unsigned int groupCount = d_groups.size();
	for (unsigned int i = 0; i < groupCount; i++){
		TdmsGroup *group = getGroup(i);
		if (!group)
			continue;

		unsigned int channels = group->getGroupSize();
		unsigned long long chunkSize = 0;
		for (unsigned int j = 0; j < channels; j++){
			TdmsChannel *channel = group->getChannel(j);
			if (channel)
				chunkSize += channel->getChannelSize();
		}

		if (!chunkSize)
			continue;

		unsigned int chunks = total_chunk_size/chunkSize;
		if (verbose)
			printf("\tTotal: %d chunks of raw data.\n", chunks);

		unsigned long long offset = 0;
		for (unsigned int j = 0; j < channels; j++){
			TdmsChannel *channel = group->getChannel(j);
			if (channel != ch)
				offset += channel->getChannelSize();
			else
				break;
		}

		if (verbose)
			printf("\tChannel offset is %lld\n", offset);

		for (unsigned int k = 0; k < chunks; k++){
			file.seekg(pos + (k + 1)*offset);
			if (!ch->readRawData(total_chunk_size, verbose))
				return false;
		}
	}
	return true;
}

bool TdmsParser::readRawData(unsigned long long total_chunk_size, const bool verbose)
{
	if (!total_chunk_size)
		return false;

	bool interleavedData = false;
	TdmsSegment *seg = getCurrentSegment();
	if (seg){
		TdmsLeadIn *leadIn = seg->getLeadIn();
		if (leadIn)
			interleavedData = leadIn->getInterleavedFlag();
	}

	if (verbose){
		if (interleavedData)
			printf("\tShould read %d raw interleaved data bytes\n", (unsigned int)total_chunk_size);
		else
			printf("\tShould read %d raw non-interleaved data bytes\n", (unsigned int)total_chunk_size);
	}

	unsigned int groupCount = d_groups.size();
	for (unsigned int i = 0; i < groupCount; i++){
		TdmsGroup *group = getGroup(i);
		if (!group)
			continue;

		unsigned int channels = group->getGroupSize();
		unsigned long long chunkSize = 0;
		for (unsigned int j = 0; j < channels; j++){
			TdmsChannel *channel = group->getChannel(j);
			if (channel){
				unsigned long long channelSize = channel->getChannelSize();
				if (verbose)
					printf("\tChannel %s size is: %d\n", channel->getName().c_str(), (unsigned int)channelSize);

				chunkSize += channelSize;
			}
		}

		unsigned int chunks = total_chunk_size/chunkSize;
		if (verbose)
			printf("Total: %d chunks of raw data.\n", chunks);

		for (unsigned int k = 0; k < chunks; k++){
			for (unsigned int j = 0; j < channels; j++){
				TdmsChannel *channel = group->getChannel(j);
				if (channel){
					if (!channel->readRawData(total_chunk_size, false))
						return false;
				}
			}
		}
	}
	return true;
}

TdmsGroup* TdmsParser::getGroup(const std::string &name)
{
	for (TdmsGroupSet::iterator iter = d_groups.begin(); iter != d_groups.end(); ++iter){
		if ((*iter)->getName() == name)
			return *iter;
	}
	return 0;
}

TdmsGroup* TdmsParser::getGroup(unsigned int index) const
{
	if (index >= d_groups.size())
		return 0;

	return d_groups.at(index);
}

TdmsSegment* TdmsParser::getSegment(unsigned int index) const
{
	if (d_segments.empty() || (index >= d_segments.size()))
		return 0;

	return d_segments.at(index);
}

TdmsSegment* TdmsParser::getCurrentSegment() const
{
	return d_segments.empty() ? 0 : d_segments.back();
}

bool TdmsParser::currentSegmentInterleaved() const
{
	TdmsSegment *seg = getCurrentSegment();
	return seg ? seg->getInterleavedFlag() : false;
}

string TdmsParser::propertiesToString() const
{
	string s;
	for (map<string, string>::const_iterator it = d_properties.begin(); it != d_properties.end(); ++it){
		s.append(it->first + ": ");
		s.append(it->second + "\n");
	}
	return s;
}

void TdmsParser::setStoreValues(bool on)
{
	if (d_store_values == on)
		return;

	d_store_values = on;

	unsigned int groupCount = d_groups.size();
	for (unsigned int i = 0; i < groupCount; i++){
		TdmsGroup *g = getGroup(i);
		if (g){
			unsigned int cnt = g->getGroupSize();
			for (unsigned int j = 0; j < cnt; j++){
				TdmsChannel *channel = g->getChannel(j);
				if (channel)
					channel->setStoreValues(on);
			}
		}
	}
}
