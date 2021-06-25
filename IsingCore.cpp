#include "IsingCore.h"

MEbuff metro(int model_size, bool random_init, double temperature,
           int init_flip, int step_flip, int steps, int wid) {
    int& n = model_size;
    double& t = temperature;
    MicroStates states(n, n);
    // define rand_spin()
    std::default_random_engine generator;
	generator.seed(static_cast<unsigned int>(time(nullptr)*(wid+2)));
	std::uniform_int_distribution<int> distribution(0, 1);
	auto rand_bool = bind(distribution, generator);
	auto rand_spin = [&rand_bool]() { return rand_bool() * 2 - 1; };
    // init states
    if (random_init == true)
        for (int i = 0; i != n; ++i)
			for (int j = 0; j != n; ++j)
				states.at(i, j) = rand_spin();
    else
        for (int i = 0; i != n; ++i)
			for (int j = 0; j != n; ++j)
				states.at(i, j) = 1;
    // define rand_axis() and rand_real()
    std::uniform_int_distribution<int> uid(0, n - 1);
	auto rand_axis = bind(uid, generator);
	std::uniform_real_distribution<double> urd(0.0, 1.0);
	auto rand_real = bind(urd, generator);
    // warm up
    for (int flip = 0; flip != init_flip; ++flip)
    {
        int x = rand_axis();
        int y = rand_axis();
        int dE = states.EnergyChange(x, y);
        if (dE <= 0)
            states.flip(x, y);
        else {
            double p = exp(-dE / t);
            double r = rand_real();
            if (r <= p) states.flip(x, y);
        }
    }
    // collect data
    MEbuff buff;
    for (int step = 0; step != steps; ++step) {
        for (int flip = 0; flip != step_flip; ++flip)
        {
            int x = rand_axis();
            int y = rand_axis();
            int dE = states.EnergyChange(x, y);
            if (dE <= 0)
                states.flip(x, y);
            else {
                double p = exp(-dE / t);
                double r = rand_real();
                if (r <= p) states.flip(x, y);
            }
        }
        buff.push_back(MEpair(states.M(), states.E()));
    }
    return buff;
}

MEbuff wolff(int model_size, bool random_init, double temperature,
           int init_flip, int step_flip, int steps, int wid) {
    int& n = model_size;
    double& t = temperature;
    MicroStates states(n, n);
    // define rand_spin()
    std::default_random_engine generator;
	generator.seed(static_cast<unsigned int>(time(nullptr)*(wid+2)));
	std::uniform_int_distribution<int> distribution(0, 1);
	auto rand_bool = bind(distribution, generator);
	auto rand_spin = [&rand_bool]() { return rand_bool() * 2 - 1; };
    // init states
    if (random_init == true)
        for (int i = 0; i != n; ++i)
			for (int j = 0; j != n; ++j)
				states.at(i, j) = rand_spin();
    else
        for (int i = 0; i != n; ++i)
			for (int j = 0; j != n; ++j)
				states.at(i, j) = 1;
    // define rand_axis() and rand_real()
    std::uniform_int_distribution<int> uid(0, n - 1);
	auto rand_axis = bind(uid, generator);
	std::uniform_real_distribution<double> urd(0.0, 1.0);
	auto rand_real = bind(urd, generator);
    // warm up
    for (int flip = 0; flip != init_flip; ++flip)
    {
        int x = rand_axis();
        int y = rand_axis();
        XYset toflip;
        toflip.insert(XYpair(x, y));
        XYqueue q;
        q.push(XYpair(x, y));
        while (!q.empty()) {
            // where?
            auto f = q.front(); q.pop();
            int& u = f.first;
            int& v = f.second;
            int u1 = (u-1+n)%n, u2 = (u+1+n)%n;
            int v1 = (v-1+n)%n, v2 = (v+1+n)%n;
            // flip?
            if (!(states.at(u,v) == states.at(u1,v) &&
                    states.at(u,v) == states.at(u2,v) &&
                    states.at(u,v) == states.at(u,v1) &&
                    states.at(u,v) == states.at(u,v2) )) continue;
            auto p = rand_real();
            if (p > 1 - exp(-2/t)) continue;
            // flip and update queue
            XYpair nb1 = XYpair(u1, v );
            XYpair nb2 = XYpair(u2, v );
            XYpair nb3 = XYpair(u , v1);
            XYpair nb4 = XYpair(u , v2);
            if (toflip.find(nb1) == toflip.end()) {
                q.push(nb1);
                toflip.insert(nb1);
            }
            if (toflip.find(nb2) == toflip.end()) {
                q.push(nb2);
                toflip.insert(nb2);
            }
            if (toflip.find(nb3) == toflip.end()) {
                q.push(nb3);
                toflip.insert(nb3);
            }
            if (toflip.find(nb4) == toflip.end()) {
                q.push(nb4);
                toflip.insert(nb4);
            }
        }
        // flip all in toflip
        for (auto& i : toflip)
            states.flip(i.first, i.second);
    }
    // collect data
    MEbuff buff;
    for (int step = 0; step != steps; ++step) {
        for (int flip = 0; flip != step_flip; ++flip)
        {
            int x = rand_axis();
            int y = rand_axis();
            XYset toflip;
            toflip.insert(XYpair(x, y));
            XYqueue q;
            q.push(XYpair(x, y));
            while (!q.empty()) {
                // where?
                auto f = q.front(); q.pop();
                int& u = f.first;
                int& v = f.second;
                int u1 = (u-1+n)%n, u2 = (u+1+n)%n;
                int v1 = (v-1+n)%n, v2 = (v+1+n)%n;
                // flip?
                if (!(states.at(u,v) == states.at(u1,v) &&
                      states.at(u,v) == states.at(u2,v) &&
                      states.at(u,v) == states.at(u,v1) &&
                      states.at(u,v) == states.at(u,v2) )) continue;
                auto p = rand_real();
                if (p > 1 - exp(-2/t)) continue;
                // flip and update queue
                XYpair nb1 = XYpair(u1, v );
                XYpair nb2 = XYpair(u2, v );
                XYpair nb3 = XYpair(u , v1);
                XYpair nb4 = XYpair(u , v2);
                if (toflip.find(nb1) == toflip.end()) {
                    q.push(nb1);
                    toflip.insert(nb1);
                }
                if (toflip.find(nb2) == toflip.end()) {
                    q.push(nb2);
                    toflip.insert(nb2);
                }
                if (toflip.find(nb3) == toflip.end()) {
                    q.push(nb3);
                    toflip.insert(nb3);
                }
                if (toflip.find(nb4) == toflip.end()) {
                    q.push(nb4);
                    toflip.insert(nb4);
                }
            }
            // flip all in toflip
            for (auto& i : toflip)
                states.flip(i.first, i.second);
        }
        buff.push_back(MEpair(states.M(), states.E()));
    }
    return buff;
}

PYBIND11_MODULE(IsingCore, m) {
    m.doc() = "pybind11 IsingCore plugin";
    m.def("metro", &metro, "The Metropolis Algorithm");
    m.def("wolff", &wolff, "The Wolff Algorithm");
}