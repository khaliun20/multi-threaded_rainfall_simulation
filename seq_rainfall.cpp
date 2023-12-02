#include <iostream>
#include <fstream>
#include <string>



int main(int argc, char *argv[]){
    if (argc != 5) {
        std::cerr << "Invalid number of arguments. Expected 4 arguments." << std::endl;
        return EXIT_FAILURE;
    }

    int time_steps = atoi(argv[1]);
    float absorb_rate = atof(argv[2]); // Use atof to convert string to float
    std::string dimension = argv[3];
    std::string elevation_file = argv[4];

    std::cout << "m: " << time_steps << std::endl;
    std::cout << "a: " << absorb_rate << std::endl;
    std::cout << "dimension: " << dimension << std::endl;
    std::cout << "elevation_file: " << elevation_file << std::endl;

    return EXIT_SUCCESS;
}

