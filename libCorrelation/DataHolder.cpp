#include "DataHolder.h"

using namespace std;

DataHolder::DataHolder(const tp_t window_size, const tp_t start, const tp_t end, const string& fn)
	:window_size(window_size), start_t(start), end_t(end)
{
	SpikeTrains sts(fn);
	_init(move(sts));
}

DataHolder::DataHolder(const tp_t window_size, const tp_t start, const tp_t end, const SpikeTrains& sts)
	: window_size(window_size), start_t(start), end_t(end)
{
	_init(sts);
}

DataHolder::DataHolder(const tp_t window_size, const tp_t start, const tp_t end, const std::vector<SCV>& cont)
	: window_size(window_size), start_t(start), end_t(end), cont(cont)
{
}
DataHolder::DataHolder(const tp_t window_size, const tp_t start, const tp_t end, std::vector<SCV>&& cont)
	: window_size(window_size), start_t(start), end_t(end), cont(cont)
{
}

void DataHolder::_init(const SpikeTrains& sts){
	cont.reserve(sts.size());
	for(const SpikeTrains::SpikeTrain& st : sts.get()){
		cont.push_back(SCV(window_size, start_t, end_t, st));
	}
	cont.shrink_to_fit();
}

double DataHolder::pearson_correlation(const SCV& first, const SCV& second){
	size_t l = first.get_length();
	int sx1 = 0, sx2 = 0, sy1 = 0, sy2 = 0, xy = 0;
	for(size_t i = 0; i < l; ++i){
		auto x = first[i];
		sx1 += x;
		sx2 += x*x;
		auto y = second[i];
		sy1 += y;
		sy2 += y*y;
		xy += x*y;
	}
	double a = l*sx2 - sx1*sx1;
	double b = l*sy2 - sy1*sy1;
	if(a == 0.0 || b == 0.0)
		return 0.0;
	double down = sqrt(a*b);
	double up = static_cast<double>(l)*xy - static_cast<double>(sx1)*sy1;
	return up / down;
}
double DataHolder::pearson_correlation(const size_t first, const size_t second){
	return pearson_correlation(cont[first], cont[second]);
}
double DataHolder::pearson_correlation(const size_t first, const std::vector<size_t>& second){
	std::vector<reference_wrapper<const SCV>> temp;
	for(size_t idx : second)
		temp.push_back(cref(cont[idx]));
	return pearson_correlation(cont[first], SCV::union_v(temp));
}
