/*
 * signal.cpp
 *
 *  Created on: 04.10.2013
 *      Author: downtimes
 * 
 *  Modified : 07.21.2022
 * 		Author: yuxuan.he
 */

#include "signal.hpp"

#include <istream>
#include <sstream>
#include <limits>
#include <iterator>
#include <algorithm>

double Signal::getValue(uint64_t payload){
	int64_t result = 0;
	uint8_t *data = (uint8_t *)&payload;
	unsigned short bit = startBit;
	for (int bitpos = 0; bitpos < length; bitpos++) {
        if (data[bit / 8] & (1 << (bit % 8))) {
            if (order == ByteOrder::INTEL) 
			{
                result |= (1ULL << bitpos);
            } 
			else 
			{
                result |= (1ULL << (length - bitpos - 1));
            }
        }
        bit++;
    }

    if ((sign == Sign::SIGNED) && (result & (1ULL << (length - 1)))) 
	{
        result |= ~((1ULL << length) - 1);
    }

    return (double)result*factor + offset;
}

std::string& trim(std::string& str, const std::string& toTrim = " ") {
	std::string::size_type pos = str.find_last_not_of(toTrim);
	if (pos == std::string::npos) {
		str.clear();
	} else {
		str.erase(pos + 1);
		str.erase(0, str.find_first_not_of(toTrim));
	}
	return str;
}

std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

std::istream& operator>>(std::istream& in, Signal& sig) {
	int c = in.peek();
	if ('B' == c) {
		in.setstate(std::ios_base::failbit);
		return in;
	}
	std::string line;
	std::getline(in, line);
	if (!line.empty() && *line.rbegin() == '\r') line.erase(line.length() - 1, 1);
	if (line.empty()) {
		in.setstate(std::ios_base::failbit);
		return in;
	}

	std::istringstream sstream(line);
	std::string preamble;
	sstream >> preamble;
	//Check if we are actually reading a Signal otherwise fail the stream
	if (preamble != "SG_") {
		sstream.setstate(std::ios_base::failbit);
		return in;
	}

	//Parse the Signal Name
	sstream >> sig.name;

	std::string multi;
	sstream >> multi;

	//This case happens if there is no Multiplexor present
	if (multi == ":") {
		sig.multiplexor = Multiplexor::NONE;
	//Case with multiplexor
	} else {
		if (multi == "M") {
			sig.multiplexor = Multiplexor::MULTIPLEXOR;
		} else {
			//The multiplexor looks like that 'm12' so we ignore the m and parse it as integer
			std::istringstream multstream(multi);
			multstream.ignore(1);
			unsigned short multiNum;
			multstream >> multiNum;
			sig.multiplexor = Multiplexor::MULTIPLEXED;
			sig.multiplexNum = multiNum;
		}
		//ignore the next thing which is a ':'
		sstream >> multi;
	}

	sstream >> sig.startBit;
	sstream.ignore(1);
	sstream >> sig.length;
	sstream.ignore(1);

	int order;
	sstream >> order;
	if (order == 0) {
		sig.order = ByteOrder::MOTOROLA;
                int pos = sig.startBit - (sig.startBit%8)+7-(sig.startBit%8);
                int temp = pos+sig.length-1;
                sig.startBit = temp -(temp%8)+7-(temp%8);

		//int pos = 7 - (sig.startBit % 8) + (sig.length - 1);
		//if (pos < 8)
		//{
			//sig.startBit -= (sig.length - 1);
		//}
		//else
		//{
			//int cpos = 7 - (pos*8);
			//int bytes = (int)(pos / 8);
			//sig.startBit = cpos + (bytes * 8) + (int)(sig.startBit/8) * 8;
		//}
		
	} else {
		sig.order = ByteOrder::INTEL;
	}

	char sign;
	sstream >> sign;
	if (sign == '+') {
		sig.sign = Sign::UNSIGNED;
	} else {
		sig.sign = Sign::SIGNED;
	}

	sstream.ignore(std::numeric_limits<std::streamsize>::max(), '(');
	sstream >> sig.factor;
	sstream.ignore(1);
	sstream >> sig.offset;
	sstream.ignore(1);

	sstream.ignore(std::numeric_limits<std::streamsize>::max(), '[');
	sstream >> sig.minimum;
	sstream.ignore(1);
	sstream >> sig.maximum;
	sstream.ignore(1);

	std::string unit;
	sstream >> unit;
	sig.unit = trim(unit, "\"");

	std::string to;
	sstream >> to;
	std::vector<std::string> toStrings = split(to, ',');
	std::move(toStrings.begin(), toStrings.end(), std::inserter(sig.to, sig.to.begin()));

	return in;
}
