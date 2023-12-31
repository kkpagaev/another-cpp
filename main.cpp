#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

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
  for (int i = 0; i < k; i++) {
    int x = sequence[i].x;
    int y = sequence[i].y;
    initial[x][y] = sequence[i].value;
  }

  for (int i = 1; i < m - 1; i++) {
    float* r = res[i];
    for (int j = 1; j < n - 1; j++) {
      const float* f = initial[i] ;
      r[j] = 0.2 * (initial[i - 1][j] + initial[i + 1][j] + f[i] + f[j - 1] + f[j + 1]);
    }
  }
}

int main() {
  int I, m, n, k;

  read_integer(I);
  read_two_integers(m, n);
  read_integer(k);

  std::cout << "I = " << I << std::endl;
  std::cout << "m = " << m << std::endl;
  std::cout << "n = " << n << std::endl;
  std::cout << "k = " << k << std::endl;

  Element sequence[k];

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

  for (int i = 0; i < I; i++) {
    iterate(initial, result, m, n, k, sequence);

    std::swap(initial, result);
  }

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      std::cout << result[i][j] << " ";
    }
    std::cout << std::endl;
  }

  return 0;
}
