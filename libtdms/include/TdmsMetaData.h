#ifndef TDMS_META_DATA_H
#define TDMS_META_DATA_H

#include "endianfstream.hh"
#include <list>
#include <vector>

class TdmsParser;
class TdmsObject;
class TdmsChannel;
class TdmsSegment;

class TdmsMetaData
{
public:
	typedef std::list<TdmsObject*> TdmsObjectList;

	TdmsMetaData(TdmsParser*, bool = false);
	~TdmsMetaData();

	void read(std::iendianfstream &infile, bool verbose);
	TdmsObject* readRawData(unsigned long long, TdmsObject *);
	const TdmsObjectList& getObjectList() const {return d_objects;}
	unsigned long long getRawDataChunkSize();
	unsigned int getChannelCount();
	unsigned int getRawDataChunkCount(){return d_chunks;}

	TdmsObject* readChannelRawData(TdmsChannel*, TdmsSegment*, TdmsObject*);

private:
	TdmsChannel* getChannel(TdmsObject *);
	TdmsObject* getObject(TdmsChannel*);
	void readObject(std::iendianfstream&, const bool);
	void print() const;
	unsigned int getInterleavedChannelCount(TdmsObject*, TdmsChannel*);

	bool d_verbose;
	unsigned int d_objectCount;
	unsigned int d_channel_cnt;
	unsigned int d_chunks;
	std::vector<unsigned long long> d_chunk_raw_data_pos;
	TdmsObjectList d_objects;
	TdmsParser* d_parser;
};

#endif
