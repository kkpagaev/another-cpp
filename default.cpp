#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>

using namespace std;

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
  int pos;
  float_t value;
};

void read_element(Element &element, int m) {
  std::string line;
  std::getline(std::cin, line);
  std::istringstream iss(line);
  int x, y;
  iss >> x >> y >> element.value;
  element.pos = x + (y * m);
}

void iterate(const float *chunk, float *res, int m, int n, int k,
             Element *sequence, const int *fixed) {
  int fixed_i = 0;
  for (int y = 0; y < m; y++) {
    for (int x = 0; x < n; x++) {
      int pos = x + (y * m);
      if (fixed_i < k && pos == fixed[fixed_i]) {
        fixed_i++;
        continue;
      }
      float sum = res[pos];
      int count = 1;

      if (y > 0) {
        sum += chunk[pos - n];
        count++;
      }
      if (y < m - 1) {
        sum += chunk[pos + n];
        count++;
      }
      if (x > 0) {
        sum += chunk[pos - 1];
        count++;
      }
      if (x < n - 1) {
        sum += chunk[pos + 1];
        count++;
      }

      res[pos] = sum / count;
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
    read_element(sequence[i], m);
  }

  float *initial = new float[m*n];
  float *result = new float[m*n];

  for (int i = 0; i < m * n; i++) {
    initial[i] = 0.0;
    result[i] = 0.0;
  }

  cout << " I = " << I << endl;
  cout << " m = " << m << endl;
  cout << " n = " << n << endl;
  cout << " k = " << k << endl;

  int *fixed = new int[k];

  sort(fixed, fixed + k);

  for (int i = 0; i < k; i++) {
    auto el = sequence[i];
    initial[el.pos] = el.value;
    result[el.pos] = el.value;
    fixed[i] = el.pos;
  }

  auto t1 = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < I; i++) {
    iterate(initial, result, m, n, k, sequence, fixed);

    std::swap(initial, result);
  }

  auto t2 = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() / 1000.0;

  std::cout << std::to_string(duration) << std::endl;

  // prints result
  for (int i = 0; i < m; i++) {
    // for (int j = 0; j < n; j++) {
    //   std::cout << initial[i * n + j] << " ";
    // }
    // std::cout << std::endl;
  }
  // std::cout << duration << std::endl;

  return 0;
}
