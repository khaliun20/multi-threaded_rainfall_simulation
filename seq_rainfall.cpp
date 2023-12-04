#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector> 


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


void absorb(std::vector<std::vector<double>> &aboveland_drops, 
            std::vector<std::vector<double>> &absorbed_drops,
            double absorb_rate){
    for (int i = 0; i < aboveland_drops.size(); i++) {
        for (int j = 0; j < aboveland_drops[0].size(); j++) {
            aboveland_drops[i][j] = aboveland_drops[i][j] - absorb_rate;
            absorbed_drops[i][j] = absorbed_drops[i][j] + absorb_rate;
        }     
    }

}


void print_matrix(std::vector<std::vector<double>> data){
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[0].size(); j++) {
            std::cout << data[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

//return 1 if dry, 0 if not dry
int check_dryness(std::vector<std::vector<double>> &aboveland_drops){
    int dry = 1;  
    for (int i = 0; i < aboveland_drops.size(); i++) {
        for (int j = 0; j < aboveland_drops[0].size(); j++) {
            if (aboveland_drops[i][j] > 0.0 ) {
                dry = 0;
                break;
            }
        }
        if (dry == 0) {
            break;
        }
    }
    return dry;
}

void add_one_drop(std::vector<std::vector<double>> &land){
    int height = land.size();
    int width = land[0].size();
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            land[i][j] += 1.0;
        }
    }
}

void trickle_away(std::vector<std::vector<double>> &aboveland_drops,
                  std::vector<std::vector<double>> &elevation) {
    int height = aboveland_drops.size();
    int width = aboveland_drops[0].size();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (aboveland_drops[i][j] > 1.0) {
                // Calculate the number of raindrops that will trickle away
                double trickle_amount = aboveland_drops[i][j] - 1.0;

                // Calculate the lowest neighbor(s) based on elevation
                double lowest_neighbor_elevation = elevation[i][j];
                int lowest_i = i, lowest_j = j;

                if (i > 0 && elevation[i - 1][j] < lowest_neighbor_elevation) {
                    lowest_neighbor_elevation = elevation[i - 1][j];
                    lowest_i = i - 1;
                    lowest_j = j;
                }
                if (i < height - 1 && elevation[i + 1][j] < lowest_neighbor_elevation) {
                    lowest_neighbor_elevation = elevation[i + 1][j];
                    lowest_i = i + 1;
                    lowest_j = j;
                }
                if (j > 0 && elevation[i][j - 1] < lowest_neighbor_elevation) {
                    lowest_neighbor_elevation = elevation[i][j - 1];
                    lowest_i = i;
                    lowest_j = j - 1;
                }
                if (j < width - 1 && elevation[i][j + 1] < lowest_neighbor_elevation) {
                    lowest_neighbor_elevation = elevation[i][j + 1];
                    lowest_i = i;
                    lowest_j = j + 1;
                }

                // Update the number of raindrops at each lowest neighbor
                aboveland_drops[lowest_i][lowest_j] += trickle_amount;

                // Update the current point with the remaining raindrops
                aboveland_drops[i][j] = 1.0;
            }
        }
    }
}

void run_simulation(int time_steps, double absorb_rate, 
                        std::string elevation_file,std::vector<std::vector<double>> &aboveland_drops, 
                        std::vector<std::vector<double>> &absorbed_drops,
                        std::vector<std::vector<double>> &elevation){
    int flag = 0;
    if (time_steps > 0){ // not dry landscape has water at a point
        time_steps--;
        //std::cout << time_steps << std::endl;
        add_one_drop(aboveland_drops);
        //print_matrix(aboveland_drops);
        absorb(aboveland_drops, absorbed_drops, absorb_rate);
        //print_matrix(absorbed_drops);
        //print_matrix(aboveland_drops);
        //trickle_away(aboveland_drops, elevation);
        
    } else{
        absorb(aboveland_drops, absorbed_drops, absorb_rate);
    }
}

void get_elevation_data(const std::string& elevation_file, std::vector<std::vector<double>>& elevation) {
    std::ifstream file(elevation_file);
    if (!file) {
        std::cerr << "Failed to open elevation file: " << elevation_file << std::endl;
        return;
    }

    std::string line;
    int row = 0;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        double value;
        int col = 0;
        while (iss >> value) {
            elevation[row][col] = value;
            col++;
        }
        row++;
    }

    file.close();
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
    int not_dry = 1;
    // create rainfall table
    std::string filename = "rainfall_table.txt";
    std::vector<std::vector<double>> aboveland_drops(height, std::vector<double>(width));
    std::vector<std::vector<double>> absorbed_drops(height, std::vector<double>(width));
    std::vector<std::vector<double>> elevation(height, std::vector<double>(width));\
    get_elevation_data(elevation_file, elevation);
    //print_matrix(elevation);
    while (time_steps > 0 || check_dryness(aboveland_drops) == 0) {
        run_simulation(time_steps, absorb_rate, elevation_file, aboveland_drops, absorbed_drops, elevation);
        time_steps--;
        }
    print_matrix(aboveland_drops);
    print_matrix(absorbed_drops);
    return EXIT_SUCCESS;
}


