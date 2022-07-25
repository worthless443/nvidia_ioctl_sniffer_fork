#include<cstdlib>
#include<chrono>
#include<cstdio>
#include<iostream>
#include<cassert>
#include<fstream>
#include<sstream>
#include<cstring>
#include<string>
#include<cstdlib>
#include<vector>
#include<regex>

#define command1 "make -j16"
#define command2 "./driver.sh"
#define BENCHMARK_FILE "benchmark/benchmark.txt"

//#include<definations.h> //casues hell of an issue 

#define DEFAULT_VALUE 1000000000 
typedef  std::chrono::high_resolution_clock _clock_t;

template<int size>
using _clock_dur = std::chrono::duration<long int,std::ratio<1,size>>;

template<int size>
using time_point =  std::chrono::time_point<std::chrono::_V2::system_clock, _clock_dur<size>>;
template<int size>

constexpr int convert(const int a) {return a;}

template<int size>
int test(_clock_dur<DEFAULT_VALUE> dur1, _clock_dur<DEFAULT_VALUE> dur2, int &flags) {
	_clock_dur<size> pnt0 = std::chrono::duration_cast<_clock_dur<size>>(dur1) ;
	_clock_dur<size> pnt1 = std::chrono::duration_cast<_clock_dur<size>>(dur2); //recasting
	assert(pnt1.count() - pnt0.count() !=0);
	if(pnt0.count() > pnt1.count()) return pnt0.count() - pnt1.count();
	else	{
		flags=1;
		return pnt1.count() -  pnt0.count();
	}
	
}
_clock_dur<DEFAULT_VALUE> get_diff(const char *command) {
	time_point<DEFAULT_VALUE> prev = _clock_t::now();
	system(command);
	auto cur = _clock_t::now();
	return cur - prev;
}

template<int size>
_clock_dur<size> cast_to_duration(_clock_dur<DEFAULT_VALUE> d) {
	return std::chrono::duration_cast<_clock_dur<size>>(d);
}


char *pretty_print(_clock_dur<DEFAULT_VALUE> dur1, _clock_dur<DEFAULT_VALUE>dur2) {
	int flag=0;
	int count1 = cast_to_duration<1000>(dur1).count();
	int count2 = cast_to_duration<1000>(dur2).count();
	char *notice_buffer = (char*)malloc(4000);
	int dif = test<1000>(dur1,dur2,flag);
	printf((flag==1) ? "program1 > program2\n" : "program2 > program\n");
	sprintf(notice_buffer, "first program existed by %d\n" ,count1);
	sprintf(notice_buffer, "second program existed by %d\n" ,count2);
	sprintf(notice_buffer, "CURRENT DIFFERNECE IN TIME in %d (ms)\n", dif);
	
	return notice_buffer;
}
template<class T>
int logger(T dur1, T dur2) {
	std::ofstream outstream(BENCHMARK_FILE, std::ios_base::app);
	if(!outstream.is_open()) return -1;
	outstream << pretty_print(dur1,dur2) << "\n";
	return  outstream.good();
}

std::vector<int> parseLines(char *buf) {
	std::stringstream linestream(buf);
	std::regex r("[0-9]+");
	std::vector<int> vec;
	char *buf_ = new char[10000];
	for(int i=0;*(int*)(buf+i)!=0;i++) buf_[i] = buf[i];
	for(;linestream>>buf_;buf_!=NULL) {
		if(std::regex_match(std::string{buf_}, r)) {
			vec.push_back(atoi(buf_));
		}
	}

	return vec;
}

void match_matrix(std::vector<int> vec) {
	std::vector<std::vector<int>> mat;
	for(int i=0;i<vec.size();i++) {
		std::vector<int> vec_;
		for(int j=0;j<vec.size();j++) {
			if(j!=i) vec_.push_back(vec.at(i) - vec.at(j));
		}
		mat.push_back(vec_);
	}
	for(auto v : mat) {
		for(int e : v) {
			std::cout << e << " ";
		}
		std::cout << "\n";
	}
}

void ParseBenchmarkFile() {
	std::stringstream lines("fuck\nnigger");
	char buf[100];
	for(;lines>>buf;buf!=NULL); //std::cout << std::string{buf} + "you" << "\n";
	std::ifstream is(BENCHMARK_FILE, std::ifstream::binary);
	std::filebuf* fbuf = is.rdbuf();
	size_t size = fbuf->pubseekoff(0,is.end, is.in);
	fbuf->pubseekpos(0,is.in);
	char *buf_ = new char[size];
	fbuf->sgetn(buf_,size);
	std::vector<int> vec = parseLines(buf_);
	match_matrix(vec);
	//std::cout << buf_;
	
}
int main(int argc, char **argv) {
	char *buf = new char;
	char *prev = new char;//[256];
	std::ofstream outstream(BENCHMARK_FILE, std::ios_base::app);
	if(!outstream.is_open()) return -1;

	_clock_dur<DEFAULT_VALUE> dur1 = get_diff(command1);
	_clock_dur<DEFAULT_VALUE> dur2 = get_diff(command2);
	logger(dur1, dur2);
	ParseBenchmarkFile();

}
