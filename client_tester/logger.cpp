#include <iostream>
#include <cassert>
#include <string>
#include <ctime>





struct DATA { 
    uint8_t misc;
    double response_time;
};


void spawn_task(int num_frames) { 


    
}

int num_args = 10;

int num_connections = 10;

int low_range;

int upper_range;

int main(int argc, char* argv[]) { 
    
    assert(argc == num_args);

    num_connections = std::stoi(&argv[1]);

    low_range = argv[2];

    upper_range = argv[3];

    std::cout << "NUM CONNECTIONS: " << num_connections << "\n";

    std::cout << "RANGE: " << low_range << " - " << upper_range << "\n";

    srand(time(NULL));

    for (int i = 0; i < num_connections; ++i) { 


        int num_frames = low_range + rand() % (upper_range - low_range + 1);

        spawn_task(num_frames);

    }

    return 0;
}