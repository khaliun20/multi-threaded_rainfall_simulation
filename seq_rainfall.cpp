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




std::vector<std::vector<double>> absorb(std::vector<std::vector<double>> data, float absorb_rate, std::vector<std::vector<double>> & new_data){
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[0].size(); j++) {
            new_data[i][j] = data[i][j] - absorb_rate;
        }     
    }
    return new_data;
}


void print_matrix(std::vector<std::vector<double>> data){
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[0].size(); j++) {
            std::cout << data[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void empty_file(std::string filename){
    std::ofstream ofs;
    ofs.open(filename, std::ofstream::out | std::ofstream::trunc);
    ofs.close();
}

void start_rain(int height, int width, int time_steps, float absorb_rate, std::string elevation_file, std::string filename){
    std::fstream file (filename, std::ios::in | std::ios::out);
    if (file.is_open()) {
        double rain_drop = 1.0;
        std::vector<std::vector<double>> data(height, std::vector<double>(width));
        std::vector<std::vector<double>> new_data(height, std::vector<double>(width));
        
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                file >> data[i][j];
                file.ignore();
            }
        }
        //print_matrix(data);
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                data[i][j] += rain_drop;
            }
        }
        new_data = absorb(data, absorb_rate, new_data);
        empty_file(filename);

        file.seekp(0);
        for (int i = 0; i <height; ++i) {
            for (int j = 0; j < width; ++j) {
                file << new_data[i][j];
                if (j < new_data[i].size() - 1) {
                    file << " "; 
                }
            }
            file << std::endl; 
    }
        file.close();
    } else {
        std::cerr << "Unable to open file." << std::endl;     
    }
}

//Create table and initiazlie to 0
std::fstream create_table(int height, int width, std::string filename) {
    std::fstream rainfall_table(filename, std::ios::in | std::ios::out);
    if (rainfall_table.is_open()) {
        rainfall_table.close();
        return rainfall_table;
    } else {
        rainfall_table.open(filename, std::ios::out);
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++){
                rainfall_table << 0;
                if (j < width - 1) {
                    rainfall_table << " ";
                }
            }
            rainfall_table << std::endl;
        }
        rainfall_table.close();
        return rainfall_table;
    }
}

//Print the rainfall table
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
    double absorb_rate = atof(argv[2]);
    std::string dimension = argv[3];
    int height = get_height(dimension);
    int width = get_width(dimension);
    std::string elevation_file = argv[4];

    // create rainfall table
    std::string filename = "rainfall_table.txt";
    std::fstream rainfall_table = create_table(height, width, filename);  
    
    //process rain drops  
    for (int i = 0; i < time_steps; i++){
        start_rain(height, width, time_steps, absorb_rate, elevation_file, filename);
        print_rainfall_table();
    }

    //print_rainfall_table();
    return EXIT_SUCCESS;
}


