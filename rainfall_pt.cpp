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
#include <pthread.h>
#include <atomic>

//return 1 if dry, 0 if not dry


void print_matrix(std::vector<std::vector<double>> data){
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[0].size(); j++) {
            std::cout << data[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

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


void add_one_drop(std::vector<std::vector<double>> &land, int i , int j){

     land[i][j] += 1.0;
}

void absorb(std::vector<std::vector<double>> &aboveland_drops, 
            std::vector<std::vector<double>> &absorbed_drops,
            double absorb_rate, int i, int j){
        if (aboveland_drops[i][j] > 0.0) {
            double to_absorb = std::min(aboveland_drops[i][j], absorb_rate);
            aboveland_drops[i][j] = aboveland_drops[i][j] - to_absorb;
            absorbed_drops[i][j] = absorbed_drops[i][j] + to_absorb;
        }
        
            
    }


void trickle_away(std::vector<std::vector<double>> &aboveland_drops,
                  const std::vector<std::vector<int>> &elevation,
                  int height, int width,
                  std::vector<std::vector<double>> &delta,
                  std::vector<std::vector<std::vector<std::pair<int, int>>>>& trickle_direction, 
                  int i, int j,
                  std::vector<std::vector<std::shared_ptr<std::mutex>>>& delta_mutexes) {
    
    
        if (aboveland_drops[i][j] > 0.0 && trickle_direction[i][j].size() > 0) {
            double trickle_amount = std::min(aboveland_drops[i][j], 1.0);
            aboveland_drops[i][j] -= trickle_amount;
            double trickle_per_neighbor = trickle_amount / trickle_direction[i][j].size();
            for (auto &pair : trickle_direction[i][j]) {
                int neighbor_row = pair.first, neighbor_col = pair.second;
                // Lock the mutex for the corresponding delta element
                (*delta_mutexes[neighbor_row][neighbor_col]).lock();
                delta[neighbor_row][neighbor_col] += trickle_per_neighbor;
                (*delta_mutexes[neighbor_row][neighbor_col]).unlock();
            }
        }

}
   
void update_after_trickle(std::vector<std::vector<double>> &absorbed_drops,
                        std::vector<std::vector<double>> &delta, int i, int j) {
        absorbed_drops[i][j] += delta[i][j];
        delta[i][j] = 0.0;
    }
    

std::mutex mtx;
void run_simulation(int time_steps, double absorb_rate, 
                        std::vector<std::vector<double>> &aboveland_drops, 
                        std::vector<std::vector<double>> &absorbed_drops,
                        std::vector<std::vector<int>> &elevation,
                        std::vector<std::vector<double>> &delta,
                        int height, int width,
                        std::vector<std::vector<std::vector<std::pair<int, int>>>>&trickle_direction,
                        int thread_num, int& total_steps, int index, 
                        std::vector<std::vector<std::shared_ptr<std::mutex>>>& delta_mutexes,
                        pthread_barrier_t& barrier, int& all_done){
    int rows_per_thread = height / thread_num;
    int start_row = index * rows_per_thread;
    int end_row = (index + 1) * rows_per_thread;
    
    while(1){
        total_steps++;
    
        for( int i = start_row; i < end_row; i++){
            for (int j = 0; j < width; j++){
                if (total_steps <= time_steps){ 
                    add_one_drop(aboveland_drops, i, j);
                }
                absorb(aboveland_drops, absorbed_drops, absorb_rate, i, j);
                trickle_away(aboveland_drops, elevation, height, width, delta, trickle_direction,i, j, delta_mutexes);
                
            }
        }
        pthread_barrier_wait(&barrier);
        //std::cout<<"waiting to update"<<std::endl;
        int done = 1;
        for( int i = start_row; i < end_row; i++){
            for (int j = 0; j < width; j++){
                update_after_trickle(aboveland_drops, delta, i ,j);
                if (aboveland_drops[i][j] > 0.0){
                    done = 0;
                }
            }
        }
        //print_matrix(aboveland_drops);
        //std::cout<<"updated"<<std::endl;
        mtx.lock();
        all_done  = all_done && done;
        mtx.unlock();
        pthread_barrier_wait(&barrier);
        if (all_done == 1){
            return;
        }
        pthread_barrier_wait(&barrier);
        all_done = 1;
      
       
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
    std::vector<std::vector<double>> aboveland_drops(height, std::vector<double>(width));

    std::vector<std::vector<double>> absorbed_drops(height, std::vector<double>(width));
    std::vector<std::vector<double>> delta(height, std::vector<double>(width));
    std::vector<std::vector<int>> elevation(height, std::vector<int>(width));
    std::vector<std::vector<std::vector<std::pair<int, int>>>> trickle_direction(height, std::vector<std::vector<std::pair<int, int>>>(width));
    std::vector<std::vector<std::shared_ptr<std::mutex>>> delta_mutexes(height, std::vector<std::shared_ptr<std::mutex>>(width));
    //print_matrix(aboveland_drops);
    // print_matrix(absorbed_drops);
    for(auto& mutex_row : delta_mutexes) {
        for(auto& mutex_ptr : mutex_row) {
            mutex_ptr = std::make_shared<std::mutex>();
        }
    }
    pthread_barrier_t barrier;
    pthread_barrier_init(&barrier, NULL, thread_num);

    //read the elevation file and compute trickle direction
    get_elevation_data(elevation_file, elevation);
    compute_trickle_direction(elevation, trickle_direction, height, width);

    //start raining! 
    int total_steps = 0;
    int all_done = 0;
    std::vector<std::thread> threads;
    clock_t start = clock();
    for (int i = 0; i < thread_num; i++) {
        threads.emplace_back(run_simulation, time_steps, absorb_rate, std::ref(aboveland_drops), std::ref(absorbed_drops),
                             std::ref(elevation), std::ref(delta),height, width, std::ref(trickle_direction), thread_num, std::ref(total_steps), i, std::ref(delta_mutexes), std::ref(barrier), std::ref(all_done));
    }
    for (auto &thread : threads) {
        thread.join();
    }
    clock_t end = clock();
    //print results

    double time_took = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "Rainfall simulation completed in " << total_steps << " time steps." << std::endl;
    std::cout << "Runtime:  " << time_took << " seconds" << std::endl;
    std::cout << std::endl;
    std::cout << "The following grid shows the number of raindrops absorbed at each point: " << std::endl;
    print_matrix(absorbed_drops);
    //print_matrix(aboveland_drops);
    std::cout << "Rainfall simulation completed in " << total_steps << " time steps." << std::endl;
    std::cout << "Runtime:  " << time_took << " seconds" << std::endl;
    //print_matrix(aboveland_drops);
    return EXIT_SUCCESS;
}