#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector> 

void print_rainfall_table();
int get_height(std::string dimension){
    std::stringstream ss(dimension);
    std::string token;
    std::getline(ss, token, 'x');
    std::getline(ss, token, 'x');
    return std::stoi(token);
}   

int get_width(std::string dimension){
    std::stringstream ss(dimension);
    std::string token;
    std::getline(ss, token, 'x');
    return std::stoi(token);
}   

void start_rain(int height, int width, int time_steps, float absorb_rate, std::string elevation_file){
    std::string filename = "rainfall_table.txt";
    std::fstream file(filename, std::ios::in | std::ios::out);
     if (file.is_open()) {
        // File exists, read and update values
        std::vector<std::vector<int>> data(height, std::vector<int>(width, 0));
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                file >> data[i][j];
                file.ignore();
            }
        }
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                data[i][j]++;
            }
        }
        file.seekp(0);
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                file << data[i][j] << " ";
                if (j < width - 1) {
                    file << " ";
                }
            }
            file << std::endl;
        }
        file.close();
    } else {
        // create file and write initial values
        file.open(filename, std::ios::out);
        if (file.is_open()) {
            for (int i = 0; i < height; ++i) {
                for (int j = 0; j < width; ++j) {
                    file << 1;
                    if (j < width - 1) {
                        file << " ";
                    }
                }
                file << std::endl;
            }

            file.close();
        } else {
            std::cerr << "Error creating or updating the file." << std::endl;
        }
    }
}
void print_rainfall_table() {
    std::ifstream rainfall_table("rainfall_table.txt");
    std::string line;
    while (std::getline(rainfall_table, line)) {
        std::cout << line << std::endl;
    }
    rainfall_table.close();
}

int main(int argc, char *argv[]){
    if (argc != 5) {
        std::cerr << "Invalid number of arguments. Expected 4 arguments." << std::endl;
        return EXIT_FAILURE;
    }
    int time_steps = atoi(argv[1]);
    float absorb_rate = atof(argv[2]);
    std::string dimension = argv[3];
    int height = get_height(dimension);
    int width = get_width(dimension);
    std::string elevation_file = argv[4];
    for (int i = 0; i < time_steps; i++){
        start_rain(height, width, time_steps, absorb_rate, elevation_file);
    }
    print_rainfall_table();
    return EXIT_SUCCESS;
}


