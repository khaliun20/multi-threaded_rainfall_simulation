#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <mutex>
#include <thread>
#include <ctime>
#include <pthread.h>
#include <climits>

class RainfallSimulation {
public:
    RainfallSimulation(int thread_num, int time_steps, 
                        double absorb_rate, int dimension, 
                        const std::string& elevation_file);
    ~RainfallSimulation();
    void run();
    void printMatrix();
    int total_steps;
    double time_took;
    std::vector<std::vector<double>> absorbed_drops;

private:
    int thread_num;
    int time_steps;
    double absorb_rate;
    int dimension;
    int height;
    int width;
    
    std::string elevation_file;

    std::vector<std::vector<double>> aboveland_drops;
    
    std::vector<std::vector<double>> delta;
    std::vector<std::vector<int>> elevation;
    std::vector<std::vector<std::vector<std::pair<int, int>>>> trickle_direction;

    pthread_barrier_t barrier;
    std::mutex mtx;
    int global_done;

    void getElevationData();
    void computeTrickleDirection();
    void addOneDrop(int i, int j);
    void absorb(int i, int j);
    void trickleAway(int i, int j);
    void updateAfterTrickle(int i, int j);
    void runSimulationThread(int index);
};

RainfallSimulation::RainfallSimulation(int thread_num, int time_steps, double absorb_rate, int dimension, const std::string& elevation_file)
    : thread_num(thread_num), time_steps(time_steps), absorb_rate(absorb_rate), dimension(dimension), elevation_file(elevation_file),
      height(dimension), width(dimension), global_done(0) {
    aboveland_drops.resize(height, std::vector<double>(width));
    absorbed_drops.resize(height, std::vector<double>(width));
    delta.resize(height, std::vector<double>(width));
    elevation.resize(height, std::vector<int>(width));
    trickle_direction.resize(height, std::vector<std::vector<std::pair<int, int>>>(width));
    pthread_barrier_init(&barrier, NULL, thread_num);
    getElevationData();
    computeTrickleDirection();
}

RainfallSimulation::~RainfallSimulation() {
    pthread_barrier_destroy(&barrier);
}

void RainfallSimulation::run() {
    std::clock_t start = std::clock();
    std::vector<std::thread> threads;
    for (int i = 0; i < thread_num; i++) {
        threads.emplace_back(&RainfallSimulation::runSimulationThread, this, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }
    std::clock_t end = std::clock();
    time_took = (end - start) /  CLOCKS_PER_SEC;
}

void RainfallSimulation::printMatrix() {
    std::cout << "Rainfall simulation completed in " << total_steps << " time steps." << std::endl;
    std::cout << "Runtime:  " << time_took << " seconds" << std::endl;
    std::cout << std::endl;
    std::cout<< "The following grid shows the number of raindrops absorbed at each point: " << std::endl;

    for (int i = 0; i < absorbed_drops.size(); i++) {
        for (int j = 0; j < absorbed_drops[0].size(); j++) {
            std::cout << absorbed_drops[i][j]<< " ";
        }
        std::cout << std::endl;
    }
    
   
}

void RainfallSimulation::getElevationData() {
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
// calculate the lowest elevatin of the neighbors for a point

void RainfallSimulation::computeTrickleDirection() {
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

void RainfallSimulation::addOneDrop(int i, int j) {
    aboveland_drops[i][j] += 1;
}

void RainfallSimulation::absorb(int i, int j) {
    if (aboveland_drops[i][j] > 0.0) {
            double to_absorb = std::min(aboveland_drops[i][j], absorb_rate);
            aboveland_drops[i][j] = aboveland_drops[i][j] - to_absorb;
            absorbed_drops[i][j] = absorbed_drops[i][j] + to_absorb;
        }
}

//calculate the amount of water to trickle away from a point
void RainfallSimulation::trickleAway(int i, int j) {
    if (aboveland_drops[i][j] > 0.0 && trickle_direction[i][j].size() > 0) {
            double trickle_amount = std::min(aboveland_drops[i][j], 1.0);
            aboveland_drops[i][j] -= trickle_amount;
            double trickle_per_neighbor = trickle_amount / trickle_direction[i][j].size();
            for (auto &pair : trickle_direction[i][j]) {
                int neighbor_row = pair.first;
                int neighbor_col = pair.second;
                delta[neighbor_row][neighbor_col] += trickle_per_neighbor;
            }
        }
}

//update the amount of water at a point after trickle away -- add the trickle amount to the point
void RainfallSimulation::updateAfterTrickle(int i, int j) {
    aboveland_drops[i][j] += delta[i][j];
    delta[i][j] = 0.0;
}


void RainfallSimulation::runSimulationThread(int index) {
    int rows_per_thread = height / thread_num;
    int start_row = index * rows_per_thread;
    int end_row = (index + 1) * rows_per_thread;
    int step = 0; 
    while (1) {
        step++;
        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < width; j++) {
                if (step <= time_steps) {
                    addOneDrop(i, j);
                }
                absorb(i, j);
                trickleAway(i, j);
            }
        }

        pthread_barrier_wait(&barrier);

        int local_done = 1;
        for (int i = start_row; i < end_row; i++) {
            for (int j = 0; j < width; j++) {
                updateAfterTrickle(i, j);
                if (aboveland_drops[i][j] > 0.0) {
                    local_done = 0;
                }
            }
        }
        {
        std::lock_guard<std::mutex> lock(mtx);
        global_done = global_done && local_done;
        }
        pthread_barrier_wait(&barrier);
        global_done = global_done && local_done;
        if (global_done == 1) {
            total_steps = step;
            return;
        }
        pthread_barrier_wait(&barrier);
        std::lock_guard<std::mutex> lock(mtx);
        global_done = 1;
    }
}


void printMatrixToFile(const std::vector<std::vector<double>>& data, std::ostream& os) {
    for (int i = 0; i < data.size(); i++) {
        for (int j = 0; j < data[0].size(); j++) {
            os << data[i][j] << " ";
        }
        os << std::endl;
    }
}
int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Invalid number of arguments. Expected 5 arguments." << std::endl;
        return EXIT_FAILURE;
    }

    int thread_num = atoi(argv[1]);
    int time_steps = atoi(argv[2]);
    double absorb_rate = atof(argv[3]);
    int dimension = atoi(argv[4]);
    std::string elevation_file = argv[5];

    RainfallSimulation simulation(thread_num, time_steps, absorb_rate, dimension, elevation_file);
    simulation.run();
    simulation.printMatrix();
    std::ofstream outputFile("ptoutput.txt");

    // Check if the file is open
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open output file." << std::endl;
        return EXIT_FAILURE;
    }

    // Write results to the file
    outputFile << "Rainfall simulation completed in " << simulation.total_steps << " time steps." << std::endl;
    outputFile << "Runtime:  " << simulation.time_took << " seconds" << std::endl;
    outputFile << std::endl;
    outputFile << "The following grid shows the number of raindrops absorbed at each point: " << std::endl;
    
    // Call your function to print the matrix to the file
    printMatrixToFile(simulation.absorbed_drops, outputFile);

    // Close the file
    outputFile.close();

    return EXIT_SUCCESS;
}
