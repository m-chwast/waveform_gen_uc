#include "wave/waveform.h"
#include <algorithm>
#include <numeric>
#include <math.h>

////global wave variable
wave waveform;



wave::wave()
{
	period = 1;
	samples.fill(0);
	sample_no = 0;
	dac_divider = 1;
}


float wave::get_period(time_unit time)
{
	float divider = 1;
	switch(time)
	{
	case time_unit::us:
		divider = 1000;
		break;
	case time_unit::ms:
		divider = 1000000;
		break;
	case time_unit::s:
		divider = 10000000000;
		break;
	default:
		break;
	}
	return period / divider;
}

float wave::get_frequency(frequency_unit freq)
{
	uint32_t multiplier = 1;
	switch(freq)
	{
	case frequency_unit::Hz:
		multiplier = 1000000;
		break;
	case frequency_unit::kHz:
		multiplier = 1000;
		break;
	case frequency_unit::MHz:
		break;
	default:
		break;
	}
	return multiplier * 1.0 / period;
}

inline uint16_t wave::amplitude_steps()
{
	auto res = std::minmax_element(samples.begin(), samples.end());
	return *(res.second) - *(res.first);
}

inline float wave::amplitude_volts()	{return amplitude_steps() * volts_per_step;}

float wave::mean()	{return std::accumulate(samples.begin(), samples.end(), (uint32_t)0) / (float)samples.size();}

float wave::rms()
{
	float res = 0;
	for(uint16_t elem : samples)
		res += elem * elem;
	res /= samples.size();
	return sqrt(res);
}

bool wave::resize_by(float scale)
{
	if(scale < 0)
		return false;
	bool not_distorted = true;
	for(int16_t& val : samples)
	{
		if(val * scale > 1023)
		{
			val = 1023;
			not_distorted = false;
		}
		else
			val *= scale;
	}
	return not_distorted;
}

inline bool wave::resize_to(uint16_t desired_amplitude_steps)	{return resize_by(float(desired_amplitude_steps) / amplitude_steps());}

inline bool wave::resize_to(float desired_amplitude_volts)	{return resize_by(desired_amplitude_volts / amplitude_volts()); }

bool wave::offset_steps(uint16_t offset)
{
	bool not_distorted = true;
	for(int16_t& val : samples)
	{
		if((int32_t)val + offset > 1023)
		{
			val = 1023;
			not_distorted = false;
		}
		else if((int32_t)val + offset < 0)
		{
			val = 0;
			not_distorted = false;
		}
		else
			val += offset;
	}
	return not_distorted;
}

