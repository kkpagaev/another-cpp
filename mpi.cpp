#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

#include <mpi.h>
#define start_end tuple<int, int>
#define coords tuple<int, int>

using namespace std;

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

void init_base(int m, int n, double *v1) {
  int size = m * n;
  for (int i = 0; i < size; i++) {
    v1[i] = 0.0;
  }
  v1[0] = 10;
  v1[m - 1] = 20;
  v1[n + 2] = 33;
  v1[2 * n] = 55;
  v1[(n * 3)] = 30;
  v1[m - 3] = 40;
  v1[m - 1] = 40;

  v1[size - n] = 20;
}

void send_chunks(int m, int n, int threads, double *res) {
  for (int i = 1; i < threads; i++) {
    int start, end;
    tie(start, end) = getchunk(i, threads, n);
    int chunk_size = get_chunk_size_with_offset(i, start, end, threads, m);

    MPI_Send(&(res[(start - 1) * (n)]), chunk_size, MPI_DOUBLE, i, 0,
             MPI_COMM_WORLD);
  }
}

void print_m(int m, int n, double *v1) {
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

  double start_time = MPI_Wtime();

  // init
  if (task == 0) {
    v1 = new double[m * n];
    init_base(m, n, v1);

    send_chunks(m, n, threads, v1);

    chunk = v1;

    print_m(m, n, v1);
    cout << endl;
  } else {
    chunk = new double[local_chunk_size];
    MPI_Recv(chunk, local_chunk_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);
  }

  // AntiAliasing
  chunk[n + 1] = 101;

  // if (task == 0) {
  //   print_m(local_chunk_size/n, n, chunk);
  // }


  // Gather
  int *recvcounts = new int[threads];
  int *displs = new int[threads];

  for (int i = 0; i < threads; i++) {
    int start, end;
    tie(start, end) = getchunk(i, threads, n);
    recvcounts[i] = (end - start + 1) * n;

    displs[i] = (i == 0) ? 0 : displs[i - 1] + recvcounts[i - 1];
  }

  // if (task == 0) {
  //   for (int i = 0; i < threads; i++) {
  //     cout << recvcounts[i] << " " << displs[i] << endl;
  //   }
  // }

  int gather_size = (local_end - local_start + 1) * n;

  MPI_Gatherv(&chunk[task == 0 ? 0 : n], gather_size, MPI_DOUBLE, res,
              recvcounts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  double end_time = MPI_Wtime();

  if (task == 0) {
    print_m(m, n, res);
    // cout << endl;
    // cout << end_time - start_time << endl;
  }

  MPI_Finalize();
  return 0;
}
