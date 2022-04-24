#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <vector>
#include <array>
#include <stdint.h>


////constants:

constexpr float reference_voltage = 3.3;
constexpr float volts_per_step = reference_voltage / 1024;


/////enums:

enum class frequency_unit {Hz, kHz, MHz};
enum class time_unit {s, ms, us};


//////classes:

class wave
{
private:

protected:

	float period;	//period is stored in microseconds



public:

	std::array<int16_t, 20000> samples;
	uint16_t sample_no;
	uint16_t dac_divider;


	wave();
	~wave() {}

	float get_period(time_unit time);
	float get_period()	{return get_period(time_unit::s);}
	float get_frequency(frequency_unit freq);
	float get_frequency() {return get_frequency(frequency_unit::Hz);}
	uint16_t amplitude_steps();
	float amplitude_volts();
	float amplitude()	{return amplitude_volts();}
	float rms();
	float mean();
	uint16_t get_sample(uint16_t sample_no)	{if(sample_no < samples.size())	return samples[sample_no]; else return 0;}
	float get_sample_volts(uint16_t sample_no)	{return volts_per_step * get_sample(sample_no);}
	bool resize_by(float scale);
	bool resize_to(float desired_amplitude_volts);
	bool resize_to(uint16_t desired_amplitude_steps);
	bool maximize() {return resize_to(reference_voltage);}
	bool offset_steps(uint16_t offset);
	bool offset_volts(float offset)	{return offset_steps(offset / volts_per_step);}
	//bool clip(float voltage);
};


class sine_wave : public wave
{
private:

public:
	sine_wave()	{sine_wave((float)1, (float)50);}
	sine_wave(float amplitude, float frequency)	{sine_wave(amplitude / volts_per_step, frequency);}
	sine_wave(uint16_t amplitude_steps, float frequency);

	~sine_wave() {}

};


extern wave waveform;







#endif
