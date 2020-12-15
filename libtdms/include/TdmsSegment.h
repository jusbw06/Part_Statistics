#ifndef TDMS_SEGMENT_H
#define TDMS_SEGMENT_H

#include "endianfstream.hh"
#include "TdmsParser.h"
#include "TdmsLeadIn.h"
#include "TdmsMetaData.h"

class TdmsSegment
{
public:
	TdmsSegment(TdmsParser*);
	~TdmsSegment();

	unsigned long long getDataOffset();
	unsigned long long getAbsDataOffset();
	unsigned long long getCursorPosition() const {return d_absolute_pos;}

	TdmsMetaData *getMetaData() const {return d_meta;}
	void setMetaData(TdmsMetaData *m){d_meta = m;}
	bool hasMetaData() const {return d_lead_in->hasMetaData();}

	TdmsLeadIn *getLeadIn() const {return d_lead_in;}
	void setLeadIn(TdmsLeadIn *l){d_lead_in = l;}

	unsigned long long getRawDataChunkSize() const {return d_data_chunk_size;}
	void setRawDataChunkSize(unsigned long long size) {d_data_chunk_size = size;}

	bool getInterleavedFlag() const {return d_lead_in ? d_lead_in->getInterleavedFlag() : false;}

private:
	TdmsParser *d_parser;
	TdmsLeadIn *d_lead_in;
	TdmsMetaData *d_meta;
	unsigned long long d_absolute_pos;
	unsigned long long d_data_chunk_size;
};

#endif // TDMS_SEGMENT_H
