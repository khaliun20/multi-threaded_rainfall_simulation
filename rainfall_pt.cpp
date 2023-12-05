#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector> 
#include <cmath>
#include <climits>
#include<iomanip>
#include <thread>
#include <mutex>

std::mutex absorb_mutex;
std::mutex add_one_drop_mutex;
std::mutex trickle_mutex;
std::mutex after_trickle_mutex;
void absorb(std::vector<std::vector<double>> &aboveland_drops, 
            std::vector<std::vector<double>> &absorbed_drops,
            double absorb_rate, int start_row, int end_row, int height, int width){
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < width; j++) {
            if (aboveland_drops[i][j] > 0.0) {
                //absorb_mutex.lock();
                double to_absorb = std::min(aboveland_drops[i][j], absorb_rate);
                aboveland_drops[i][j] = aboveland_drops[i][j] - to_absorb;
                absorbed_drops[i][j] = absorbed_drops[i][j] + to_absorb;
                //absorb_mutex.unlock();
            }
            
        }     
    }

}


void print_matrix(std::vector<std::vector<double>> data){
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[0].size(); j++) {
            std::cout<< std::setw(8) << std::setprecision(6) << data[i][j];
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


void add_one_drop(std::vector<std::vector<double>> &land, int height, int width, int start_row, int end_row){

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < width; j++) {
            //add_one_drop_mutex.lock();
            land[i][j] += 1.0;
            //add_one_drop_mutex.unlock();
        }
    }
}

void trickle_away(std::vector<std::vector<double>> &aboveland_drops,
                  const std::vector<std::vector<int>> &elevation,
                  int height, int width,
                  std::vector<std::vector<double>> &delta,
                  std::vector<std::vector<std::vector<std::pair<int, int>>>>&trickle_direction, int start_row, int end_row) {

    for (int i =start_row; i < end_row; i++){
        for (int j = 0; j <width; j++){
            if( aboveland_drops[i][j]> 0.0 && trickle_direction[i][j].size() > 0){
                double trickle_amount  = std::min (aboveland_drops[i][j], 1.0);
                aboveland_drops[i][j] -= trickle_amount;
                double trickle_per_neighbor = trickle_amount / trickle_direction[i][j].size();
                for (auto &pair : trickle_direction[i][j]) {
                    int neighbor_row = pair.first, neighbor_col = pair.second;
                    //trickle_mutex.lock();
                    delta[neighbor_row][neighbor_col] += trickle_per_neighbor;
                    //trickle_mutex.unlock();
                }
            }
        }
    }

}
   
void update_after_trickle(std::vector<std::vector<double>> &absorbed_drops,
                        std::vector<std::vector<double>> &delta, int height, int width, int start_row, int end_row) {
    for (int i =start_row; i < end_row; i++){
        for (int j = 0; j < width; j++){
            //after_trickle_mutex.lock();
            absorbed_drops[i][j] += delta[i][j];
            delta[i][j] = 0.0;
           // after_trickle_mutex.unlock();
        }
    }
    

}
void run_simulation(int time_steps, double absorb_rate, 
                        std::vector<std::vector<double>> &aboveland_drops, 
                        std::vector<std::vector<double>> &absorbed_drops,
                        std::vector<std::vector<int>> &elevation,
                        std::vector<std::vector<double>> &delta,
                        int height, int width,
                        std::vector<std::vector<std::vector<std::pair<int, int>>>>&trickle_direction,
                        int thread_num){
    int flag = 0;
    std::vector<std::thread> threads;
    if (time_steps > 0){ // not dry landscape has water at a point
        threads.clear();
        for (int i = 0 ; i < thread_num; i++){
            int start_row = i * height / thread_num;
            int end_row = (i + 1) * height / thread_num;
            threads.push_back(std::thread(add_one_drop, std::ref(aboveland_drops), height, width, start_row, end_row));
        }

        for (auto &thread : threads) {
            thread.join();
        }
        
        threads.clear();
        for (int i = 0 ; i < thread_num; i++){
            int start_row = i * height / thread_num;
            int end_row = (i + 1) * height / thread_num;
            threads.push_back(std::thread(absorb, std::ref(aboveland_drops), std::ref(absorbed_drops), absorb_rate, start_row, end_row, height, width));
        }
        for (auto &thread : threads) {
            thread.join();
        }

        
        threads.clear();
        for (int i = 0 ; i < thread_num; i++){
            int start_row = i * height / thread_num;
            int end_row = (i + 1) * height / thread_num;
            threads.push_back(std::thread(trickle_away, std::ref(aboveland_drops), std::ref(elevation), height, width, std::ref(delta), std::ref(trickle_direction),start_row, end_row));
        }
        for (auto &thread : threads) {
            thread.join();
        }

        threads.clear();
        for (int i = 0 ; i < thread_num; i++){
            int start_row = i * height / thread_num;
            int end_row = (i + 1) * height / thread_num;
            threads.push_back(std::thread(update_after_trickle, std::ref(absorbed_drops), std::ref(delta), height, width, start_row, end_row));
        }
        for (auto &thread : threads) {
            thread.join();
        }


        
    } else{
        threads.clear();
        for (int i = 0 ; i < thread_num; i++){
            int start_row = i * height / thread_num;
            int end_row = (i + 1) * height / thread_num;
            threads.push_back(std::thread(absorb, std::ref(aboveland_drops), std::ref(absorbed_drops), absorb_rate, start_row, end_row, height, width));
        }
        for (auto &thread : threads) {
            thread.join();
        }

        
        threads.clear();
        for (int i = 0 ; i < thread_num; i++){
            int start_row = i * height / thread_num;
            int end_row = (i + 1) * height / thread_num;
            threads.push_back(std::thread(trickle_away, std::ref(aboveland_drops), std::ref(elevation), height, width, std::ref(delta), std::ref(trickle_direction),start_row, end_row));
        }
        for (auto &thread : threads) {
            thread.join();
        }

        threads.clear();
        for (int i = 0 ; i < thread_num; i++){
            int start_row = i * height / thread_num;
            int end_row = (i + 1) * height / thread_num;
            threads.push_back(std::thread(update_after_trickle, std::ref(absorbed_drops), std::ref(delta), height, width, start_row, end_row));
        }
        for (auto &thread : threads) {
            thread.join();
        }

    }
}

void get_elevation_data(const std::string& elevation_file, std::vector<std::vector<int>>& elevation) {
    std::ifstream file(elevation_file);
    if (!file) {
        std::cerr << "Failed to open elevation file: " << elevation_file << std::endl;
        return;
    }

    std::string line;
    int row = 0;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int value;
        int col = 0;
        while (iss >> value) {
            elevation[row][col] = value;
            col++;
        }
        row++;
    }

    file.close();
}

void compute_trickle_direction(std::vector<std::vector<int>>&  elevation,
std::vector<std::vector<std::vector<std::pair<int, int>>>> &trickle_direction,
int height, int width){
    std::vector<std::vector<int>> directions{{-1, 0}, {1, 0}, {0, 1}, {0, -1}};
    for (int i =0; i < height; i++){
        for (int j = 0; j <width; j++){
            int lowest = INT_MAX; 
            for (const auto& direction : directions) {
                int neighbor_row = i + direction[0], neighbor_col = j + direction[1];
                if (0 <= neighbor_row && neighbor_row< height && 0 <= neighbor_col && neighbor_col < width) {
                    lowest = std::min(lowest, elevation[neighbor_row][neighbor_col]);
                }
            }
            if (elevation[i][j] <= lowest) {
                continue;
            }
            for (const auto& direction : directions){
                int neighbor_row = i + direction[0], neighbor_col = j + direction[1];
                if (0 <= neighbor_row && neighbor_row < height && 0 <= neighbor_col && neighbor_col < width){
                    int neighVal = elevation[neighbor_row][neighbor_col];
                    if (neighVal== lowest) {
                        trickle_direction[i][j].push_back({neighbor_row, neighbor_col});
                    }
                }

            }
        }

    }
}

void print_trickle_direction(std::vector<std::vector<std::vector<std::pair<int, int>>>> &trickle_direction){
    for (int i = 0; i < trickle_direction.size(); i++) {
        for (int j = 0; j < trickle_direction[0].size(); j++) {
            for (auto &pair : trickle_direction[i][j]) {
                std::cout << "(" << pair.first << ", " << pair.second << ") ";
            }
            std::cout << " | ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char *argv[]){
    if (argc != 6) {
        std::cerr << "Invalid number of arguments. Expected 4 arguments." << std::endl;
        return EXIT_FAILURE;
    }
    //handle input arguments
    int thread_num = atoi(argv[1]);
    int time_steps = atoi(argv[2]);
    double absorb_rate = atof(argv[3]);
    int dimension = atoi(argv[4]);
    int height = dimension;
    int width = dimension;
    std::string elevation_file = argv[5];
    
    //initialize variables
    int not_dry = 1;
    std::vector<std::vector<double>> aboveland_drops(height, std::vector<double>(width));
    std::vector<std::vector<double>> absorbed_drops(height, std::vector<double>(width));
    std::vector<std::vector<double>> delta(height, std::vector<double>(width));
    std::vector<std::vector<int>> elevation(height, std::vector<int>(width));\
    std::vector<std::vector<std::vector<std::pair<int, int>>>> trickle_direction(height, std::vector<std::vector<std::pair<int, int>>>(width));

    //read the elevation file and compute trickle direction
    get_elevation_data(elevation_file, elevation);
    compute_trickle_direction(elevation, trickle_direction, height, width);

    //start raining! 
    int total_steps = 0;
    clock_t start = clock();
    while (time_steps > 0 || check_dryness(aboveland_drops) == 0) {
        run_simulation(time_steps, absorb_rate,aboveland_drops, absorbed_drops, elevation, delta,height, width, trickle_direction, thread_num);
        time_steps--;
        total_steps++;
        }
    clock_t end = clock();
    //print results
    double time_took = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Rainfall simulation completed in " << total_steps << " time steps." << std::endl;
    std::cout << "Runtime:  " << time_took << " seconds" << std::endl;
    std::cout << std::endl;
    std::cout << "The following grid shows the number of raindrops absorbed at each point: " << std::endl;
    print_matrix(absorbed_drops);

    std::cout << check_dryness(aboveland_drops)<<std::endl;
    return EXIT_SUCCESS;
}
