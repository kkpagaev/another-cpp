#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

#include <bits/stdc++.h>
#include <mpi.h>
#define start_end tuple<int, int>
#define coords tuple<int, int>

using namespace std;

struct Element {
  int pos;
  float_t value;
};

void read_element(Element &element, int n) {
  std::string line;
  std::getline(std::cin, line);
  std::istringstream iss(line);
  int x, y;
  iss >> x >> y >> element.value;
  element.pos = x + (y * n);
}

void read_fixed(int n, int k, Element *sequence) {
  for (int i = 0; i < k; i++) {
    read_element(sequence[i], n);
  }
}

enum AntiAliasingDirection {
  FIRST = 0,
  MIDDLE = 1,
  LAST = 2,
  FULL = 3,
};

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

MPI_Datatype myDataType;
MPI_Aint extent;

void iterate(const double *chunk, double *res, int chunk_size, int n,
             const int *fixed, int fixed_size, const AntiAliasingDirection dir,
             int m) {
  int fixed_i = 0;
  int start_row = dir == AntiAliasingDirection::FIRST ? 0 : 1;
  int end_row = dir == AntiAliasingDirection::LAST ? chunk_size / n : chunk_size/n - 1; 
  if (dir == AntiAliasingDirection::FULL) {
    start_row = 0;
    end_row = chunk_size / n;
  }
  // cout << "start_row " << start_row << " end_row " << end_row << " dir " << dir << endl;
  for (int y = start_row; y < end_row; y++) {
    for (int x = 0; x < n; x++) {
      int pos = x + (y * n);

      if (fixed_i < fixed_size && pos == fixed[fixed_i]) {
        fixed_i++;
        continue;
      }
      double sum = chunk[pos];
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

void read_settings(int rank, int &I, int &m, int &n, int &k) {
  if (rank == 0) {
    read_integer(I);
    read_two_integers(m, n);
    read_integer(k);
  }

  MPI_Bcast(&I, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

// start and finish
start_end getchunk(int rank, int threads, int rows) {
  int rows_per_thread = rows / threads;
  int remainder = rows % threads;

  int start = 0;
  int end = rows_per_thread - 1;

  if (remainder > 0) {
    end += 1;
  }

  for (int i = 1; i <= rank; i++) {
    start = end + 1;
    end += rows_per_thread;

    if (remainder > i) {
      end++;
    }
  }

  return make_tuple(start, end);
}

int get_chunk_size_with_offset(int task, int start, int end, int threads,
                               int rows) {
  int chunk_size = end - start + 1;

  if (threads > 1) {
    if (task == 0) {
      chunk_size++;
    } else if (task == threads - 1) {
      chunk_size++;
    } else {
      chunk_size += 2;
    }
  }

  return chunk_size * rows;
}

void init_base(int m, int n, double *v1, Element *sequence, int k) {
  int size = m * n;
  for (int i = 0; i < size; i++) {
    v1[i] = 0.0;
  }
}

void send_chunks(int m, int n, int threads, double *res, const int *fixed,
                 int k, int fixed_start) {
  for (int i = 1; i < threads; i++) {
    int start, end;
    tie(start, end) = getchunk(i, threads, n);
    int chunk_size = get_chunk_size_with_offset(i, start, end, threads, m);

    MPI_Send(&(res[(start - 1) * (n)]), chunk_size, MPI_DOUBLE, i, 0,
             MPI_COMM_WORLD);

    int fixed_end = fixed_start;
    int fixed_count = 0;
    int end_cap = (end + 1) * n;
    while (fixed_end < k && fixed[fixed_end] < end_cap) {
      fixed_count++;
      fixed_end++;
    }
    fixed_start = fixed_end;

    MPI_Send(&fixed_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

    MPI_Send(&(fixed[fixed_start - fixed_count]), fixed_count, MPI_INT, i, 0,
             MPI_COMM_WORLD);
  }
}

AntiAliasingDirection get_direction(int task, int threads) {
  if (threads == 1) {
    return AntiAliasingDirection::FULL;
  } else if (task == 0) {
    return AntiAliasingDirection::FIRST;
  } else if (task == threads - 1) {
    return AntiAliasingDirection::LAST;
  } else {
    return AntiAliasingDirection::MIDDLE;
  }
}

void print_m(const int m, const int n, const double *v1) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      cout << v1[i * n + j] << " ";
    }
    cout << endl;
  }
}

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);

  int task, threads;
  MPI_Comm_rank(MPI_COMM_WORLD, &task);
  MPI_Comm_size(MPI_COMM_WORLD, &threads);

  // I - iterantions
  // m - columns
  // n - rows
  // k - elements in sequence
  int I, m, n, k;

  read_settings(task, I, m, n, k);

  int local_start, local_end;
  tie(local_start, local_end) = getchunk(task, threads, n);

  int local_chunk_size =
      get_chunk_size_with_offset(task, local_start, local_end, threads, n);

  double *v1;
  double *res = new double[m * n];
  double *chunk;
  int *fixed;
  int fixed_size = 0;

  double start_time = MPI_Wtime();

  // init
  if (task == 0) {
    Element *sequence = new Element[k];
    read_fixed(n, k, sequence);

    v1 = new double[m * n];

    init_base(m, n, v1, sequence, k);

    int *fixed_local = new int[k];
    for (int i = 0; i < k; i++) {
      fixed_local[i] = sequence[i].pos;
      v1[sequence[i].pos] = sequence[i].value;
    }


    std::sort(fixed_local, fixed_local + k);

    int end_cap = (local_end + 1) * n;

    while (fixed_size < k && fixed_local[fixed_size] < end_cap) {
      fixed_size++;
    }
    // cout << "fixed_size " << fixed_size << endl;

    send_chunks(m, n, threads, v1, fixed_local, k, fixed_size);

    chunk = v1;
    fixed = fixed_local;

    // cout << "fixed: ";
    // for (int i = 0; i < k; i++) {
    //   cout << fixed[i] << " ";
    // }
    // cout << endl;

    // print_m(m, n, chunk);
    // cout << endl;
  } else {
    chunk = new double[local_chunk_size];
    MPI_Recv(chunk, local_chunk_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    MPI_Recv(&fixed_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    fixed = new int[fixed_size];
    MPI_Recv(fixed, fixed_size, MPI_INT, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
    for (int i = 0; i < fixed_size; i++) {
      int f = fixed[i];
      // cout << "fixed[" << i << "] = " << f << endl;
      fixed[i] = f - ((local_start - 1) * n);
    }
  }

  // string ss = "task: " + to_string(task) + "fixed: ";
  //
  // for (int i = 0; i < fixed_size; i++) {
  //   ss += to_string(fixed[i]) + " ";
  // }
  //
  // cout << ss << endl;
  //

  // cout << "task " << task << " bababar" << fixed_size << endl;

  double *flat = new double[local_chunk_size];

  for (int i = 0; i < local_chunk_size; i++) {
    flat[i] = chunk[i];
  }

  std::vector<MPI_Request> requests;
  requests.reserve(4);

  auto dir = get_direction(task, threads);
  for (int i = 0; i < I; i++) {
    if (task > 0) {
      requests.push_back(MPI_Request());
      MPI_Isend(&chunk[n], n, MPI_DOUBLE, task - 1, 0, MPI_COMM_WORLD,
                &requests.back());
      requests.push_back(MPI_Request());
      MPI_Irecv(&chunk[0], n, MPI_DOUBLE, task - 1, 0, MPI_COMM_WORLD,
                &requests.back());
    }
    if (task < threads - 1) {
      requests.push_back(MPI_Request());
      MPI_Isend(&chunk[local_chunk_size - (n * 2)], n, MPI_DOUBLE, task + 1, 0,
                MPI_COMM_WORLD, &requests.back());
      requests.push_back(MPI_Request());
      MPI_Irecv(&chunk[local_chunk_size - n], n, MPI_DOUBLE, task + 1, 0,
                MPI_COMM_WORLD, &requests.back());
    }
    MPI_Waitall(requests.size(), requests.data(), MPI_STATUSES_IGNORE);
    requests.clear();

    iterate(chunk, flat, local_chunk_size, n, fixed, fixed_size, dir, local_chunk_size / n);
    auto temp = chunk;
    chunk = flat;
    flat = temp;
  }

  // Gather
  int *recvcounts = new int[threads];
  int *displs = new int[threads];

  for (int i = 0; i < threads; i++) {
    int start, end;
    tie(start, end) = getchunk(i, threads, n);
    recvcounts[i] = (end - start + 1) * n;

    displs[i] = (i == 0) ? 0 : displs[i - 1] + recvcounts[i - 1];
  }

  int gather_size = (local_end - local_start + 1) * n;

  MPI_Gatherv(&chunk[task == 0 ? 0 : n], gather_size, MPI_DOUBLE, res,
              recvcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  double end_time = MPI_Wtime();

  if (task == 0) {
    print_m(m, n, res);
    cout << endl;
    cout << "threads " << threads << endl << end_time - start_time << endl;
  }

  MPI_Finalize();
  return 0;
}
