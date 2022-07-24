#include<cstdlib>
#include<chrono>
#include<cstdio>
#include<iostream>
#include<cassert>

#define command1 "make"
#define command2 "./driver.sh"

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
int test(_clock_dur<DEFAULT_VALUE> dur1, _clock_dur<DEFAULT_VALUE> dur2) {
	_clock_dur<size> pnt0 = std::chrono::duration_cast<_clock_dur<size>>(dur1) ;
	_clock_dur<size> pnt1 = std::chrono::duration_cast<_clock_dur<size>>(dur2); //recasting
	assert(pnt1.count() - pnt0.count() !=0);
	if(pnt0.count() > pnt1.count()) return pnt0.count() - pnt1.count();
	else	return pnt1.count() -  pnt0.count()  + 1;
	
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


void pretty_print(_clock_dur<DEFAULT_VALUE> dur1, _clock_dur<DEFAULT_VALUE>dur2) {
	int count1 = cast_to_duration<1000>(dur1).count();
	int count2 = cast_to_duration<1000>(dur2).count();
	printf("first program existed by %d\n" ,count1);
	printf("second program existed by %d\n" ,count2);
	printf("CURRENT DIFFERNECE IN TIME in (ms)\n");
	
	printf("these two are behind by %d miliseconds\n", test<1000>(dur1, dur2));
}

int main(int argc, char **argv) {
	if(argc<1) return 1;
	while(1) {
	_clock_dur<DEFAULT_VALUE> dur1 = get_diff(command1);
	_clock_dur<DEFAULT_VALUE> dur2 = get_diff(command2);
	
	pretty_print(dur1,dur2);
	}

}
