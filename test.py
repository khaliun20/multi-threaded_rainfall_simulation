#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <climits>
#include <iomanip>
#include <thread>
#include <algorithm>

// ... (Other functions remain unchanged)

void parallel_trickle_away(std::vector<std::vector<double>> &aboveland_drops,
                            const std::vector<std::vector<int>> &elevation,
                            int height, int width,
                            std::vector<std::vector<double>> &delta,
                            std::vector<std::vector<std::vector<std::pair<int, int>>>> &trickle_direction,
                            int start_row, int end_row) {

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < width; j++) {
            if (aboveland_drops[i][j] > 0.0 && trickle_direction[i][j].size() > 0) {
                double trickle_amount = std::min(aboveland_drops[i][j], 1.0);
                aboveland_drops[i][j] -= trickle_amount;
                double trickle_per_neighbor = trickle_amount / trickle_direction[i][j].size();
                for (auto &pair : trickle_direction[i][j]) {
                    int neighbor_row = pair.first, neighbor_col = pair.second;
                    delta[neighbor_row][neighbor_col] += trickle_per_neighbor;
                }
            }
        }
    }
}

void parallel_run_simulation(int time_steps, double absorb_rate,
                             std::vector<std::vector<double>> &aboveland_drops,
                             std::vector<std::vector<double>> &absorbed_drops,
                             std::vector<std::vector<int>> &elevation,
                             std::vector<std::vector<double>> &delta,
                             int height, int width,
                             std::vector<std::vector<std::vector<std::pair<int, int>>>> &trickle_direction,
                             int num_threads) {

    int rows_per_thread = height / num_threads;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; i++) {
        int start_row = i * rows_per_thread;
        int end_row = (i == num_threads - 1) ? height : start_row + rows_per_thread;
        threads.emplace_back(parallel_trickle_away, std::ref(aboveland_drops), std::ref(elevation),
                             height, width, std::ref(delta), std::ref(trickle_direction), start_row, end_row);
    }

    for (auto &thread : threads) {
        thread.join();
    }

    // The rest of the run_simulation function remains unchanged
    // ...

}

int main(int argc, char *argv[]) {
    // ...

    // Adjust the number of threads based on your system and requirements
    int num_threads = 4;

    // ...

    while (time_steps > 0 || check_dryness(aboveland_drops) == 0) {
        parallel_run_simulation(time_steps, absorb_rate, aboveland_drops, absorbed_drops,
                                 elevation, delta, height, width, trickle_direction, num_threads);
        time_steps--;
        total_steps++;
    }

    // ...

    return EXIT_SUCCESS;
}