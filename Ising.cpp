﻿// print
#include <iostream>
#include <string>

// random
#include <random>
#include <functional>
#include <vector>
#include <ctime>

// multi-threading
#include <thread>
#include <mutex>

class MicroStates
{
	int* array; int x, y;
public:
	MicroStates(int x, int y);
	MicroStates(const MicroStates& other);
	~MicroStates();
	int& at(const int& i, const int& j);
	int& at(const int& i, const int& j) const;
	void flip(const int& i, const int& j);
	int M();
	void show() const;
	void show(const int& i) const;
	void show(const std::string& s) const;
	int EnergyChange(const int& i, const int& j) const;
protected:
	int& at_edge(const int& i, const int& j) const;
	int index(const int& i, const int& j) const;
	void show_() const;
	static std::mutex mtx_stdout;
};

int n, steps = 0; // size and thermalization steps
float tb, te, ts; // temperature begin/end/step

std::mutex mtx_report; // mutex on report vector

void worker(const int i, const MicroStates& init, std::vector<int>& report)
{
	// copy init state to local
	MicroStates local_state(init);

	// define local random engine
	std::default_random_engine generator;
	generator.seed(static_cast<unsigned int>(time(nullptr))
		* (static_cast<unsigned int>(i) + 2));
	std::uniform_int_distribution<int> uid(0, n - 1);
	auto rand_axis = bind(uid, generator);
	std::uniform_real_distribution<float> urd(0.0, 1.0);
	auto rand_real = bind(urd, generator);

	std::vector<int> local_report;
	local_report.reserve(static_cast<size_t>((te - tb) / ts) * 2 + 2);
	int M = local_state.M();
	//int E = local_state.E(); TODO

	for (float t = tb; t <= te; t += ts) {
		for (int step = 0; step != steps; ++step)
		{
			int x = rand_axis();
			int y = rand_axis();
			int dE = local_state.EnergyChange(x, y);
			if (dE <= 0) { // trace and flip
				if (local_state.at(x, y) == -1) M += 2;
				else M -= 2;
				local_state.flip(x, y);
			}
			else {
				float p = exp(-dE / t);
				float r = rand_real();
				if (r <= p) { // trace and flip
					if (local_state.at(x, y) == -1) M += 2;
					else M -= 2;
					local_state.flip(x, y);
				}
			}
			//local_state.show(M);
		}
		local_report.push_back(M);
		//local_state.show(M);
	}

	// report to main
	mtx_report.lock();
	report = local_report;
	mtx_report.unlock();
}

int main()
{
	std::cout << "Ising started." << std::endl;
	// define rand_spin()
	std::default_random_engine generator;
	generator.seed(static_cast<unsigned int>(time(nullptr)));
	std::uniform_int_distribution<int> distribution(0, 1);
	auto rand_bool = bind(distribution, generator);
	auto rand_spin = [&rand_bool]() { return rand_bool() * 2 - 1; };

	// initialize micro-state
	std::cout << "Size of the state?" << std::endl;
	std::cin >> n;
	MicroStates init(n, n);
	int orthered_init = 0;
	std::cout << "0: orthered; others: disordered." << std::endl;
	std::cin >> orthered_init;
	if (orthered_init == 0)
		for (int i = 0; i != n; ++i)
			for (int j = 0; j != n; ++j)
				init.at(i, j) = 1;
	else
		for (int i = 0; i != n; ++i)
			for (int j = 0; j != n; ++j)
				init.at(i, j) = rand_spin();

	// set temperature begin/end/step
	std::cout << "Temperature begin/end/step?" << std::endl;
	std::cin >> tb >> te >> ts;

	//set steps
	while (steps < 1) {
		std::cout << "Thermalization steps? (10000+)" << std::endl;
		std::cin >> steps;
	}

	// launch workers
	std::cout << "Number of the workers?" << std::endl;
	int w; std::cin >> w;
	std::vector<std::vector<int>> reports; reports.reserve(w);
	std::vector<std::thread> works; std::vector<int> empty_report;
	for (int i = 0; i != w; ++i) {
		reports.push_back(empty_report);
		// i changes fast, captures by copy.
		works.emplace_back([&, i]() { worker(i, init, reports[i]); });
	}
	// wait for all works
	for (auto& t : works)
		t.join();

	// Mavg
	auto rz = reports[0].size();
	for (int i = 0; i != rz; ++i) {
		float Mavg = 0.0;
		for (int j = 0; j != w; ++j) {
			Mavg += reports[j][i];
		}
		Mavg /= w;
		std::cout << Mavg << ' ';
	}
	std::cout << std::endl;

	return 0;
}

inline MicroStates::MicroStates(int x, int y)
	: x(x), y(y),
	array(new int[static_cast<unsigned __int64>(x)
		* static_cast<unsigned __int64>(y)]) {}

inline MicroStates::MicroStates(const MicroStates& other) {
	this->x = other.x;
	this->y = other.y;
	this->array = new int[static_cast<unsigned __int64>(x)
		* static_cast<unsigned __int64>(y)];
	for (int i = 0; i != x; ++i)
		for (int j = 0; j != y; ++j)
			this->at(i, j) = other.at(i, j);
}

inline MicroStates::~MicroStates() { delete[] array; }

inline int& MicroStates::at(const int& i, const int& j) { return array[index(i, j)]; }

inline int& MicroStates::at(const int& i, const int& j) const { return array[index(i, j)]; }

inline void MicroStates::flip(const int& i, const int& j) { this->at(i, j) *= -1; }

inline int MicroStates::M() {
	int ret = 0;
	for (int i = 0; i != x; ++i)
		for (int j = 0; j != y; ++j)
			ret += this->at(i, j);
	return ret;
}

inline void MicroStates::show() const {
	mtx_stdout.lock();
	this->show_();
	mtx_stdout.unlock();
}

inline void MicroStates::show(const int& i) const {
	mtx_stdout.lock();
	std::cout << "#" << i << std::endl;
	this->show_();
	mtx_stdout.unlock();
}

inline void MicroStates::show(const std::string& s) const {
	mtx_stdout.lock();
	std::cout << s << std::endl;
	this->show_();
	mtx_stdout.unlock();
}

inline int MicroStates::EnergyChange(const int& i, const int& j) const {
	int ret = this->at_edge(i + 1, j) +
		this->at_edge(i - 1, j) +
		this->at_edge(i, j + 1) +
		this->at_edge(i, j - 1);
	ret *= this->at(i, j) * 2;
	return ret;
}

inline int& MicroStates::at_edge(const int& i, const int& j) const { return array[index((i + x) % x, (j + y) % y)]; }

inline int MicroStates::index(const int& i, const int& j) const { return x * i + j; }

inline void MicroStates::show_() const {
	for (int i = 0; i != x; ++i) {
		for (int j = 0; j != y; ++j)
			if (this->at(i, j) == 1)
				std::cout << '+';
			else if (this->at(i, j) == -1)
				std::cout << '-';
			else
				std::cout << '?';
		std::cout << std::endl;
	}
}
