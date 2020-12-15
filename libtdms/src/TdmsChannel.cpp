#include "TdmsChannel.h"
#include <cstring>
#include <stdlib.h>

using namespace std;

TdmsChannel::TdmsChannel(const std::string& name, std::iendianfstream& f):
d_name(name),
file(f),
dataType(0),
typeSize(0),
dimension(0),
nvalues(0),
d_data_cnt(0),
d_string_cnt(0),
flagHasRawData(false),
flagIsInterleaved(false),
d_store_values(true),
d_min_value(.0),
d_max_value(.0),
d_data_sum(.0),
d_group(0)
{}

TdmsChannel::~TdmsChannel()
{
	freeMemory();
}

std::string TdmsChannel::getUnit() const
{
	return getProperty("unit_string");
}

std::string TdmsChannel::getDescription() const
{
	return getProperty("description");
}

std::string TdmsChannel::getProperty(const std::string& name) const
{
	map<string, string>::const_iterator it = d_properties.find(name);
	if (it != d_properties.end())
		return it->second;

	return "";
}

string TdmsChannel::propertiesToString() const
{
	string s;
	unsigned int cnt = d_properties.size(), aux = 0;
	for (map<string, string>::const_iterator it = d_properties.begin(); it != d_properties.end(); ++it){
		s.append(it->first + ": ");
		s.append(it->second);
		aux++;
		if (aux < cnt)
			s.append("\n");
	}
	return s;
}

void TdmsChannel::addProperties(std::map<std::string, std::string> props)
{
	for (map<string, string>::const_iterator it = props.begin(); it != props.end(); ++it)
		d_properties.insert(std::pair<string, string>(it->first, it->second));
}

void TdmsChannel::freeMemory()
{
	if (!dataVector.empty())
		vector<double>().swap(dataVector);

	if (!imagDataVector.empty())
		vector<double>().swap(imagDataVector);

	if (!stringVector.empty())
		vector<string>().swap(stringVector);

	d_properties.clear();
}

void TdmsChannel::setTypeSize(unsigned int size)
{
	typeSize = size;
}

void TdmsChannel::setDataType(unsigned int type)
{
	dataType = type;
}

void TdmsChannel::setValuesCount(unsigned long long n)
{
	if (!n)
		return;

	nvalues = n;
}

unsigned long long TdmsChannel::getChannelSize() const
{
	return typeSize*dimension*nvalues;
}

unsigned long long TdmsChannel::getMaxVectorSize() const
{
	return ((dataType == TdmsChannel::tdsTypeString) || (dataType == TdmsChannel::tdsTypeTimeStamp)) ? stringVector.size() : dataVector.size();
}

bool TdmsChannel::readRawData(unsigned long long total_chunk_size, bool verbose)
{
	if (nvalues == 0 && typeSize != 0)
		nvalues = total_chunk_size/typeSize;

	if (verbose)
		printf("\tChannel %s: reading %d raw data value(s) of type %d.", d_name.c_str(), (unsigned int)nvalues, dataType);

	if (!reserveDataMemory(nvalues)){
		if (verbose)
			printf("\t\tCouldn't allocate memory for storing data in channel %s!\n", d_name.c_str());
		return false;
	}

	if (dataType == TdmsChannel::tdsTypeString)
		readStrings();
	else {
		for (unsigned long long i = 0; i < nvalues; ++i)
			readValue(dataType, false);
	}

	if (verbose)
		printf("\tFinished reading raw data (POS: 0x%X).\n", (unsigned int)file.tellg());

	return true;
}

bool TdmsChannel::readDAQmxData(std::vector<FormatChangingScaler> formatScalers, std::vector<unsigned int> dataWidths, bool verbose)
{
	if (formatScalers.empty() || dataWidths.empty())
		return false;

	if (!reserveDataMemory(nvalues))
		return false;

	FormatChangingScaler formatScaler = formatScalers.front();
	unsigned int type = formatScaler.DAQmxDataType;
	unsigned int formatTypeSize = TdmsObject::dataTypeSize(type)*dataWidths.front();

	if (formatTypeSize == TdmsObject::dataTypeSize(tdsTypeI64))
		type = tdsTypeI64;
	else if (formatTypeSize == TdmsObject::dataTypeSize(tdsTypeI32))
		type = tdsTypeI32;
	else if (formatTypeSize == TdmsObject::dataTypeSize(tdsTypeI16))
		type = tdsTypeI16;

	if (type != formatScaler.DAQmxDataType){
		std::string slopeString = getProperty("NI_Scale[1]_Linear_Slope");
		double slope = slopeString.empty() ? 1.0 : atof(slopeString.c_str());

		std::string interceptString = getProperty("NI_Scale[1]_Linear_Y_Intercept");
		double intercept = interceptString.empty() ? 0.0 : atof(interceptString.c_str());

		unsigned int rawByteOffset = formatScaler.rawByteOffset;
		unsigned int channelCnt = d_group ? d_group->getGroupSize() : 1;
		unsigned int channelIndex = d_group ? d_group->getChannelIndex(this) : 0;
		if (verbose){
			printf("\tChannel %s: reading %d values. Slope: %g intercept: %g interleaved: %d rawByteOffset: %d\n",
			d_name.c_str(), (unsigned int)nvalues, slope, intercept, flagIsInterleaved, rawByteOffset);
		}
		unsigned long long pos = (unsigned long long)file.tellg();//start position of the data chunk
		for (unsigned int i = 0; i < nvalues; ++i)
			readDAQmxValue(type, slope, intercept, rawByteOffset, channelIndex, channelCnt);
		if (flagIsInterleaved && !rawByteOffset && (channelIndex < channelCnt - 1))
			file.seekg(pos, ios_base::beg);
		if (verbose)
			printf("\t\tCursor at: 0x%X\n", (unsigned int)file.tellg());
	} else {
		for (unsigned int i = 0; i < nvalues; ++i)
			readValue(type);
	}
	return true;
}

void TdmsChannel::readDAQmxValue(unsigned int type, double slope, double intercept, unsigned int rawByteOffset,
unsigned int index, unsigned int channelCnt, bool verbose)
{
	if (verbose)
		printf("\tRead DAQmx value for channel %s index in group: %d/%d. rawByteOffset: %d\n", d_name.c_str(), index, channelCnt, rawByteOffset);

	switch (type){
	case tdsTypeI32:
		{
			if (flagIsInterleaved && rawByteOffset)
				file.seekg(rawByteOffset, ios_base::cur);

			int val;
			file >> val;
			if (verbose)
				printf("\t%d -> %.10g (type = %d)\n", val, val*slope + intercept, type);
			appendValue(val*slope + intercept);

			if (flagIsInterleaved){
				for (unsigned int i = index + 1; i < channelCnt; i++)
					file.seekg(sizeof(int), ios_base::cur);
			}
		}
	break;

	default:
		{
			if (verbose)
				printf("\t(unknown type = %d)\n", type);
		}
	break;
	}
}

double TdmsChannel::getImaginaryValue(unsigned int i) const
{
	if (imagDataVector.empty() || (i >= imagDataVector.size()))
		return .0;

	return imagDataVector.at(i);
}

double TdmsChannel::getValue(unsigned int i) const
{
	if (dataVector.empty() || (i >= dataVector.size()))
		return .0;

	return dataVector.at(i);
}

void TdmsChannel::appendValue(double val)
{
	if (!d_data_cnt){
		d_min_value = val;
		d_max_value = val;
	} else {
		if (val < d_min_value)
			d_min_value = val;
		if (val > d_max_value)
			d_max_value = val;
	}

	d_data_sum += val;
	++d_data_cnt;

	if (d_store_values)
		dataVector.push_back(val);
}

bool TdmsChannel::reserveMemory()
{
	if (!d_store_values)
		return true;

	if (d_data_cnt){
		try {
			dataVector.reserve(d_data_cnt);
		} catch (std::bad_alloc&){
			return false;
		}

		if ((dataType == tdsTypeComplexSingleFloat) || (dataType == tdsTypeComplexDoubleFloat)){
			try {
				imagDataVector.reserve(d_data_cnt);
			} catch (std::bad_alloc&){
				return false;
			}
		}
		//printf("\nreserveMemory: Succesfully reserved memory for channel %s, new data vector capacity: %ld\n", d_name.c_str(), dataVector.capacity());

		d_data_cnt = 0;
		d_data_sum = .0;
	} else if (d_string_cnt){
		try {
			stringVector.reserve(d_string_cnt);
		} catch (std::bad_alloc&){
			return false;
		}

		d_string_cnt = 0;
	}
	return true;
}

bool TdmsChannel::reserveDataMemory(unsigned long long cnt)
{
	if (!d_store_values)
		return true;

	if (dataType == tdsTypeString){
		unsigned long long size = stringVector.size() + cnt;
		if (size <= stringVector.capacity())
			return true;

		try {
			stringVector.reserve(size);
		} catch (std::bad_alloc&){
			return false;
		}
	} else {
		unsigned long long size = dataVector.size() + cnt;
		if ((size <= dataVector.capacity()) && (size <= imagDataVector.capacity()))
			return true;

		try {
			dataVector.reserve(size);
		} catch (std::bad_alloc&){
			return false;
		}

		if ((dataType == tdsTypeComplexSingleFloat) || (dataType == tdsTypeComplexDoubleFloat)){
			try {
				imagDataVector.reserve(size);
			} catch (std::bad_alloc&){
				return false;
			}
		}
		//printf("\nreserveDataMemory: Succesfully reserved memory for channel %s, new data vector capacity: %ld\n", d_name.c_str(), dataVector.capacity());
	}
	return true;
}

void TdmsChannel::readStrings()
{
	vector<unsigned int> offsets;
	offsets.push_back(0);
	for (unsigned int i = 0; i < nvalues; ++i){
		unsigned int offset;
		file >> offset;
		//printf("i: %d offset = %d POS @ 0x%X\n", i, offset, (unsigned int)file.tellg());
		offsets.push_back(offset);
	}

	unsigned int POS = offsets.at(0);
	for (unsigned int i = 1; i <= nvalues; ++i){
		unsigned int offset = offsets.at(i);
		unsigned int size = offset - POS;
		string s(size, 0);
		file >> s;
		if (d_store_values)
			stringVector.push_back(s);
		//printf("i: %d offset: %d size = %d s: %s POS %d @ 0x%X\n", i, offset, size, s.c_str(), POS, (unsigned int)file.tellg());
		POS = offset;
	}
}

std::string TdmsChannel::getString(unsigned int i) const
{
	if (stringVector.empty() || (i >= stringVector.size()))
		return "";

	return stringVector.at(i);
}

void TdmsChannel::appendString(std::string s)
{
	++d_string_cnt;

	if (d_store_values)
		stringVector.push_back(s);
}

void TdmsChannel::readValue(unsigned int itype, bool verbose)
{

	if (verbose)
		printf("	Read value for channel: %s\n", d_name.c_str());

	switch (itype){
	case tdsTypeI8: //INT8
		{
			char val;
			file >> val;
			appendValue((double)val);
			if (verbose)
				printf("%d (type = %d)\n", (int)val, itype);
		}
	break;

	case tdsTypeI16: //INT16
		{
			short val;
			file >> val;
			appendValue((double)val);
			if (verbose)
				printf("%d (type = %d)\n", (int)val, itype);
		}
	break;

	case tdsTypeI32: //INT32
		{
			int val;
			file >> val;
			appendValue((double)val);
			if (verbose)
				printf("%d (type = %d)\n", val, itype);
		}
	break;

	case tdsTypeI64: //INT64
		{
			long long val;
			file >> val;
			appendValue((double)val);
			if (verbose)
				printf("%d (type = %d)\n", (int)val, itype);
		}
	break;

	case tdsTypeU8: //UINT8
		{
			unsigned char val;
			file >> val;
			appendValue((double)val);
			if (verbose)
				printf("%d (type = %d)\n", (int)val, itype);
		}
	break;


	case tdsTypeU16: //UINT16
		{
			unsigned short val;
			file >> val;
			appendValue((double)val);
			if (verbose)
				printf("%d (type = %d)\n", (int)val, itype);
		}
	break;

	case tdsTypeU32: //UINT32
		{
			unsigned int val;
			file >> val;
			appendValue((double)val);
			if (verbose)
				printf("%u (type = %d)\n", val, itype);
		}
	break;

	case tdsTypeU64: //UINT64
		{
			unsigned long long val;
			file >> val;
			appendValue((double)val);
			if (verbose)
				printf("%u (type = %d)\n", (unsigned int)val, itype);
		}
	break;

	case tdsTypeSingleFloat: //FLOAT32
	case tdsTypeSingleFloatWithUnit:
		{
			float val;
			file >> val;
			appendValue((float)val);
			if (verbose)
				printf("%f (type = %d)\n", val, itype);
		}
	break;

	case tdsTypeDoubleFloat: //FLOAT64
	case tdsTypeDoubleFloatWithUnit:
		{
			double val;
			file >> val;
			appendValue(val);
			if (verbose)
				printf("%f (type = %d)\n", val, itype);
		}
	break;

	case tdsTypeExtendedFloat: //FLOAT128
	case tdsTypeExtendedFloatWithUnit:
		{
			long double val;
			file >> val;
			appendValue((double)val);
			if (verbose)
				printf("%f (type = %d)\n", (double)val, itype);
		}
	break;

	case tdsTypeString: //string values are read in readRawData function directly
	break;

	case tdsTypeBoolean: //bool
		{
			bool val;
			file >> val;
			appendValue((double)val);
			if (verbose)
				printf("%d (type = %d)\n", val, itype);
		}
	break;

	case tdsTypeTimeStamp: //time stamp
		{
			unsigned long long fractionsSecond;
			file >> fractionsSecond;
			long long secondsSince;
			file >> secondsSince;
			string ts = TdmsObject::timestamp(secondsSince, fractionsSecond);
			appendString(ts);
			if (verbose)
				printf("%s (type = %d)\n", ts.c_str(), itype);
		}
	break;

	case tdsTypeComplexSingleFloat:
		{
			float rval, ival;
			file >> rval;
			appendValue((float)rval);
			file >> ival;
			appendImaginaryValue((float)ival);
			if (verbose)
				printf("%g+i*%g (type = 0x%X)\n", rval, ival, itype);
		}
	break;

	case tdsTypeComplexDoubleFloat:
		{
			double rval, ival;
			file >> rval;
			appendValue(rval);
			file >> ival;
			appendImaginaryValue(ival);
			if (verbose)
				printf("%g+i*%g (type = 0x%X)\n", rval, ival, itype);
		}
	break;

	default:
		{
			if (verbose)
				printf(" (unknown type = %d)\n", itype);
		}
	break;
	}
}
