#include "TdmsSegment.h"

TdmsSegment::TdmsSegment(TdmsParser *p):
d_parser(p),
d_lead_in(0),
d_meta(0),
d_absolute_pos(0),
d_data_chunk_size(0)
{
	std::iendianfstream *file = &(d_parser->fileStream());
	if (file)
		d_absolute_pos = file->tellg();
}

TdmsSegment::~TdmsSegment()
{
	if (d_lead_in)
		delete d_lead_in;

	if (d_meta)
		delete d_meta;
}

unsigned long long TdmsSegment::getDataOffset()
{
	return d_lead_in ? d_lead_in->getDataOffset() + 28 : 0; // leadin size
}

unsigned long long TdmsSegment::getAbsDataOffset()
{
	return d_lead_in ? d_absolute_pos + d_lead_in->getDataOffset() + 28 : 0;
}
