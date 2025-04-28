#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>  // Incluir OpenMP para paralelização

#define THRESHOLD 1000  // Limite mínimo para paralelismo


void selectionSortSerial(int arr[], int n) {
    int i, j, minIdx, temp;

    for (i = 0; i < n - 1; i++) {
        minIdx = i;
        for (j = i + 1; j < n; j++) {
            if (arr[j] < arr[minIdx]) {
                minIdx = j;
            }
        }

        temp = arr[minIdx];
        arr[minIdx] = arr[i];
        arr[i] = temp;
    }
}

// Versão paralela do Selection Sort
void selectionSortParalelo(int arr[], int n) {
    int i, j, minIdx, maxIdx, temp;
    int metade = n / 2;

    #pragma omp parallel
    {
        for (i = 0; i < metade; i++) {
            minIdx = i;
            maxIdx = n - i - 1;

            #pragma omp single nowait
            {

                #pragma omp task shared(minIdx)
                //#pragma omp task
                {
                    for (j = i + 1; j < n - i; j++) {
                        if (arr[j] < arr[minIdx]) {
                            minIdx = j;
                        }
                    }
                }


                #pragma omp task shared(maxIdx)
                //#pragma omp task
                {
                    for (j = i; j < n - i - 1; j++) {
                        if (arr[j] > arr[maxIdx]) {
                            maxIdx = j;
                        }
                    }
                }

                #pragma omp taskwait

                if (minIdx != i) {
                    temp = arr[i];
                    arr[i] = arr[minIdx];
                    arr[minIdx] = temp;
                }

                if (maxIdx == i) {
                    maxIdx = minIdx;
                }

                if (maxIdx != n - i - 1) {
                    temp = arr[n - i - 1];
                    arr[n - i - 1] = arr[maxIdx];
                    arr[maxIdx] = temp;
                }
            }
        }
    }
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high) {
    int pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

// QuickSort Sequencial
void quicksort_sequential(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quicksort_sequential(arr, low, pi - 1);
        quicksort_sequential(arr, pi + 1, high);
    }
}

// QuickSort Paralelo com OpenMP
void quicksort_parallel(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);

        if ((high - low) > THRESHOLD) {  // Evita paralelismo em pequenas partições
            #pragma omp parallel num_threads(4)
            {
                #pragma omp single nowait
                {
                    #pragma omp task
                    quicksort_parallel(arr, low, pi - 1);

                    #pragma omp task
                    quicksort_parallel(arr, pi + 1, high);

                    #pragma omp taskwait  // Garante que todas as tarefas terminem
                }
            }
        } else {
            quicksort_sequential(arr, low, high);  // Troca para versão sequencial em pequenas partições
        }
    }
}

// Função para imprimir o vetor
void printArray(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void gerarVetorAleatorio(int arr[], int n, int max_valor) {
    srand(time(NULL));

    for (int i = 0; i < n; i++) {
        arr[i] = rand() % (max_valor + 1);
    }
}

int main() {
    int n = 100000;
    int max_valor = 100000;

    int arrSerial[n];
    int arrParalelo[n];

    // Gera o vetor aleatório
    gerarVetorAleatorio(arrSerial, n, max_valor);
    for (int i = 0; i < n; i++) {
        arrParalelo[i] = arrSerial[i];
    }
    //printArray(arrParalelo, n);

    double inicio,fim;
    inicio = omp_get_wtime();
    selectionSortSerial(arrSerial, n);
    fim = omp_get_wtime();
    double t_serial = fim-inicio;
    printf("Execucao serial: %f\n",t_serial);

   // printArray(arrSerial, n);

    inicio = omp_get_wtime();
    selectionSortParalelo(arrParalelo, n);
	fim = omp_get_wtime();
    double t_paralelo = fim - inicio;
	printf("Execucao paralela: %f\n",t_paralelo);

    //printArray(arrParalelo, n);


    int a = 1000000;  // Teste com tamanho grande para notar diferença
    int *vet_seq = (int *)malloc(sizeof(int) * a);
    int *vet_par = (int *)malloc(sizeof(int) * a);
    int m = a;

    // Verifica se a alocação de memória foi bem-sucedida
    if (vet_seq == NULL || vet_par == NULL) {
        printf("Erro ao alocar memória!\n");
        return 1;
    }

    srand(time(NULL));

    // Preenchendo os vetores com números aleatórios dentro de um intervalo menor
    for (int i = 0; i < a; i++) {
        int num = rand() % 1000000;  // Evita números muito grandes
        vet_seq[i] = num;
        vet_par[i] = num; // Cópia para manter os mesmos valores nos dois testes
    }


      // Execução Sequencial
    double start_seq = omp_get_wtime();
    quicksort_sequential(vet_seq, 0, m - 1);
    double end_seq = omp_get_wtime();
    printf("Tempo de execução (Sequencial): %.6f segundos\n", end_seq - start_seq);

    // Execução Paralela
    double start_par = omp_get_wtime();
    quicksort_parallel(vet_par, 0, m - 1);
    double end_par = omp_get_wtime();
    printf("Tempo de execução (Paralelo): %.6f segundos\n", end_par - start_par);

    // Liberação de memória
    free(vet_seq);
    free(vet_par);


    return 0;
}





