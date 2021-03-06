#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <regex>
#include "FirstAlg.h"
#include "SecondAlg.h"
#include "CompareTopo.h"
#include "AnalyzeTopo.h"
#include "OnRealData.h"
#include "TopoOneTrial.h"
#include "test.h"

using namespace std;


void infer(const string& fn_st, const string& fn_inf,
	const tp_t window_size, const tp_t start, const tp_t end,
	const double cor_th, const tp_t delay_th, const double delay_acc_rate)
{
	cout << fn_st << endl;
	FirstAlg alg(window_size, start, end, fn_st);
	size_t n = alg.size();
	alg.set_mpps_param(delay_th, delay_acc_rate);
	alg.set_mpps(cor_th);
//	alg.output_mpps(cout);

	alg.set_ps_by_mpps();
//	alg.output_ps(cout);
	ofstream fout(fn_inf);
	alg.output_ps(fout);
	fout.close();
}

void infer2(const string&fn_st, const string& fn_inf,
	const tp_t window_size, const tp_t start, const tp_t end, const double cor_th)
{
	cout << fn_st << endl;
	SecondAlg alg(window_size, start, end, fn_st);
	size_t n = alg.size();
	alg.set_mpps(cor_th);
//	alg.output_mpps(cout);

	alg.set_ps_by_mpps();
//	alg.output_ps(cout);
	ofstream fout(fn_inf);
	alg.output_ps(fout);
	fout.close();
}


void go_trial(const string& fn_st, const string& fn_if_head,
	int start, int end, int period, int window, int amp2ms, double cor_th, bool output_dyn)
{
	TopoOneTrial trial(fn_st, start, end, period, window, amp2ms);
	trial.set_param(cor_th);
	trial.go();
	trial.gen_static_structure();
	size_t n = trial.size();
	if(output_dyn){
		for(size_t i = 0; i < n; ++i){
			ofstream fout(fn_if_head + to_string(i) + ".txt");
			AlgBase::output_vps(fout, trial.get_structure(i));
			fout.close();
		}
	}
	ofstream fout(fn_if_head +  "static.txt");
	AlgBase::output_vps(fout, trial.get_static_structure());
	fout.close();
}

pair<AnalyzeTopo::adj_g_t, AnalyzeTopo::adj_g_t> delta_structure(
	const AnalyzeTopo::adj_g_t& g1, const AnalyzeTopo::adj_g_t& g2)
{
	size_t n = g1.size();
	AnalyzeTopo::adj_g_t add, remove;
	for(size_t i = 0; i < n; ++i){
		vector<size_t> v1, v2;
		set_difference(g1[i].begin(), g1[i].end(), g2[i].begin(), g2[i].end(), back_inserter(v1));
		remove.push_back(move(v1));
		set_difference(g2[i].begin(), g2[i].end(), g1[i].begin(), g1[i].end(), back_inserter(v2));
		add.push_back(move(v2));
	}
	return make_pair(move(add), move(remove));
}

pair<double, double> consistence(const size_t n, const AnalyzeTopo& anl){
	double res = 0.0, res_nonzero = 0.0;
	auto prob_g = anl.get_prob_g();//0.5 is the most inconsistent; 0 and 1 are the most consistent
	for(size_t i = 0; i < n; ++i){
		for(size_t j = 0; j < n; ++j){
			double t = 2 * abs(prob_g[i][j] - 0.5);//normalize to 1; 0 is the most inconsistent
			res += t;
			if(prob_g[i][j] != 0.0)
				res_nonzero += t;
		}
	}
	return make_pair(res / n / n, res_nonzero / n / n);
}

pair<double,double> consistence_static(const size_t n, const string& st_fld, const vector<string>& name_list,
	int start, int end, int period, int window, int amp2ms, double cor_th)
{
	AnalyzeTopo anl(n);
	for(size_t i = 0; i<name_list.size(); ++i){
		TopoOneTrial trial(st_fld+name_list[i], start, end, period, window, amp2ms);
		trial.set_param(cor_th);
		trial.go();
		trial.gen_static_structure();
		anl.add(trial.get_static_structure());
	}
	return consistence(n,anl);
}

vector<pair<pair<double, double>, pair<double, double>>> consistence_dynamic(
	const size_t n, const string& st_fld, const vector<string>& name_list,
	const vector<size_t>& idx, int start, int end, int period, int window, int amp2ms, double cor_th)
{
	vector<AnalyzeTopo> anls_add(idx.size() - 1, AnalyzeTopo(n));
	vector<AnalyzeTopo> anls_rmv(idx.size() - 1, AnalyzeTopo(n));
	for(size_t i = 0; i<name_list.size(); ++i){
		TopoOneTrial trial(st_fld + name_list[i], start, end, period, window, amp2ms);
		trial.set_param(cor_th);
		trial.go();
		for(size_t j = 1; j < idx.size(); ++j){
			auto p=delta_structure(trial.get_structure(j - 1), trial.get_structure(j));
			anls_add[j - 1].add(move(p.first));
			anls_rmv[j - 1].add(move(p.second));
		}
	}
	//vector<pair<double, double>> res;
	vector<pair<pair<double, double>, pair<double, double>>> res;
	for(size_t i = 0; i < idx.size()-1;++i){
		auto t1 = consistence(n, anls_add[i]);
		auto t2 = consistence(n, anls_rmv[i]);
		//res.push_back(make_pair((t1.first + t2.first) / 2, (t1.second + t2.second) / 2));
		res.push_back(make_pair(move(t1), move(t2)));
	}
	return res;
}

void compare(const string& base_fn, const string& suffix){
	cout << base_fn << "\t" << suffix << endl;
	CompareTopo cmp(base_fn);
	string fn2 = base_fn;
	fn2.replace(fn2.rfind('.'), 1, suffix + ".");
	auto cm = cmp.cmatrix(cmp.compare(fn2));
	cout << "\tR=t\tR=f" << endl;
	cout << "P=t\t" << cm.tp << '\t' << cm.tn<<'\t'<<cm.predict_t() << endl;
	cout << "P=f\t" << cm.fn << '\t' << cm.fp<<'\t'<<cm.predict_f() << endl;
	cout << "\t"<<cm.real_t()<<'\t'<<cm.real_f()<<'\t'<<cm.total()<<endl;

	auto scores = cmp.scores(cm);
	cout << "precision=" << scores.precision << endl;
	cout << "recall=" << scores.recall << endl;
	cout << "f1=" << scores.f1 << endl;
	cout << "accurancy=" << scores.accurancy << endl;

}


int main(int argc, char* argv[])
{
	string base_dir("../data/");
	string base_dir2(base_dir + "real/");
	vector<string> test_files{ "sparent.txt", "mparent.txt", "indirect1.txt", "indirect2.txt", "common1.txt",
		"common2.txt", "common3.txt", "big100.txt", "big25.txt", "big10.txt" };
//	test(base_dir); return 0;

//	infer2(base_dir2 + "st/cue1_1.txt", base_dir2 + "if2/cue1_1.txt", 200 * 10, 0, 1020 * 10, 0.2);
//	return 0;

	const int n_c1 = 54, n_c2 = 52;
	vector<string> cue1_list, cue2_list;
	for(int i = 0; i < n_c1; ++i)
		cue1_list.push_back("cue1_" + to_string(i) + ".txt");
	for(int i = 0; i < n_c2; ++i)
		cue2_list.push_back("cue2_" + to_string(i) + ".txt");

	OnRealData real(base_dir2, 10, 19);
	real.set_time_start(0);
	real.set_time_end(1000);
	function<void(const string&, const string&, const string&, double, double)> go_mid =
		[&](const string& head, const string& st_fld, const string& if_fld, double cor_th, double pro_th)
	{
		real.set_st_fld(st_fld);
		real.set_if_fld(if_fld);
		real.go_batch("cue1_" + head, cue1_list, true, cor_th, pro_th);
		real.go_batch("cue2_" + head, cue2_list, true, cor_th, pro_th);
	};
//	go_mid("prob", "st/", "if2", 0.2, 0.5);
//	go_mid("prob", "st0-3/", "if2_0-3/", 0.2, 0.5);
//	go_mid("prob", "st3-6/", "if2_3-6/", 0.2, 0.5);
//	go_mid("prob", "st6-9/", "if2_6-9/", 0.2, 0.5);
//	return 0;
	function<void(const string&, const string&, const string&, const vector<double>&, const vector<double>&)> go_multi =
		[&](const string& head, const string& st_fld, const string& if_fld, const vector<double>& cor_ths, const vector<double>& pro_ths)
	{
		real.set_st_fld(st_fld);
		real.set_if_fld(if_fld);
		real.go_multi_parameter("cue1_" + head, cue1_list, cor_ths, pro_ths);
		real.go_multi_parameter("cue2_" + head, cue2_list, cor_ths, pro_ths);
	};
//	go_multi("prob", "st0-3/", "if2_0-3m/", { 0.2, 0.4, 0.6, 0.8 }, { 0.2, 0.4, 0.5, 0.6, 0.8 });
//	go_multi("prob", "st3-6/", "if2_3-6m/", { 0.2, 0.4, 0.6, 0.8 }, { 0.2, 0.4, 0.5, 0.6, 0.8 });
//	go_multi("prob", "st6-9/", "if2_6-9m/", { 0.2, 0.4, 0.6, 0.8 }, { 0.2, 0.4, 0.5, 0.6, 0.8 });
//	return 0;

	function<void(const string&, const double)> go_trials = [&](const string& if_fld, const double cor_th){
//		go_trial(base_dir2 + "st/cue1_0.txt", base_dir2 + if_fld + "cue1_0_", 0, 10000, 1000, 100, 10, cor_th, true);
//		go_trial(base_dir2 + "st/cue1_1.txt", base_dir2 + if_fld + "cue1_1_", 0, 10000, 1000, 100, 10, cor_th, true);
//		go_trial(base_dir2 + "st/cue1_2.txt", base_dir2 + if_fld + "cue1_2_", 0, 10000, 1000, 100, 10, cor_th, true);
		for(int i = 0; i < n_c1; ++i){
			go_trial(base_dir2 + "st/cue1_" + to_string(i) + ".txt",
				base_dir2 + if_fld + "cue1_" + to_string(i) + "_", 0, 10000, 1000, 100, 10, cor_th, false);
		}
	};
//	go_trials("if2_trial/", 0.7);
//	return 0;

//	auto p = consistence_static(19, base_dir2 + "st/", cue1_list, 0, 10000, 1000, 100, 10, 0.7);
//	cout << "Overall consistence: "<<p.first<<"\nEdge consistence: "<<p.second<< endl;
	auto vp = consistence_dynamic(19, base_dir2 + "st/", cue1_list, { 0,1,2,3,4, 5, 6,7,8,9 }, 0, 10000, 1000, 100, 10, 0.7);
	pair<double, double> padd,prmv;
	for(pair<pair<double, double>, pair<double, double>>& t : vp){
		padd.first += t.first.first;
		padd.second += t.first.second;
		prmv.first += t.second.first;
		prmv.second += t.second.second;
		cout <<"Phase:\nadd "<< t.first.first << " : " << t.first.second;
		cout << " remove " << t.second.first << " : " << t.second.second;
		cout << " total " << (t.first.first + t.second.first) / 2 << " : " << (t.first.second + t.second.second) / 2 << endl;
	}
	cout << "Together:" << endl;
	cout << "add: " << padd.first / vp.size() << " : " << padd.second / vp.size() << endl;
	cout << "remove: " << prmv.first / vp.size() << " : " << prmv.second / vp.size() << endl;
	cout << "Overall consistence: " << (padd.first + prmv.first) / 2/vp.size() << 
		"\nEdge consistence: " << (padd.second + prmv.second) / 2/vp.size() << endl;
	return 0;

//	infer(base_dir + "big100.txt", "_st2", 20, 0, 1040, 0.6, 8, 0.8);
//	for(size_t i = 0; i < test_files.size(); ++i)
	for(size_t i = 8; i < 9; ++i)
	{
//		infer(base_dir + test_files[i], "_st", 20, 0, 1040, 0.6, 8, 0.8);
		compare(base_dir + test_files[i], "_st_if");
		cout << endl;
//		infer(base_dir + test_files[i], "_st2", 20, 0, 1040, 0.6, 8, 0.8);
		compare(base_dir + test_files[i], "_st2_if");
	}
	return 0;
}

