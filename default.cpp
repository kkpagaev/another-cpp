#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>

void read_integer(int &value) {
  std::string line;
  std::getline(std::cin, line);
  value = std::stoi(line);
}

void read_two_integers(int &value1, int &value2) {
  std::string line;
  std::getline(std::cin, line);
  std::istringstream iss(line);
  iss >> value1 >> value2;
}

struct Element {
  int x;
  int y;
  float_t value;
};

void read_element(Element &element) {
  std::string line;
  std::getline(std::cin, line);
  std::istringstream iss(line);
  iss >> element.x >> element.y >> element.value;
}

void iterate(float **initial, float **res, int m, int n, int k,
             Element *sequence) {
  for (int y = 0; y < m; y++) {
    
    for (int x = 0; x < n; x++) {
      float sum = res[y][x];
      int count = 1;

      if (y > 0) {
        sum += initial[y - 1][x];
        count++;
      }
      if (y < m - 1) {
        sum += initial[y + 1][x];
        count++;
      }
      if (x > 0) {
        sum += initial[y][x - 1];
        count++;
      }
      if (x < n - 1) {
        sum += initial[y][x + 1];
        count++;
      }

      res[y][x] = sum / count;
    }
  }
}

int main(int argc, const char* argv[]) {
  int I, m, n, k;

  read_integer(I);
  read_two_integers(m, n);
  read_integer(k);

  Element *sequence = new Element[k];

  for (int i = 0; i < k; i++) {
    read_element(sequence[i]);
  }

  float **initial = new float *[m];
  float **result = new float *[m];

  for (int i = 0; i < m; i++) {
    initial[i] = new float[n];
    result[i] = new float[n];

    for (int j = 0; j < n; j++) {
      initial[i][j] = 0.0;
      result[i][j] = 0.0;
    }
  }

  for (int i = 0; i < k; i++) {
    auto el = sequence[i];
    initial[el.x][el.y] = el.value;
  }

  auto t1 = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < I; i++) {
    iterate(initial, result, m, n, k, sequence);

    std::swap(initial, result);
  }

  auto t2 = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() / 1000.0;

  std::cout << std::to_string(duration) << std::endl;

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      std::cout << result[i][j] << " ";
    }
    std::cout << std::endl;
  }
  // std::cout << duration << std::endl;

  return 0;
}
