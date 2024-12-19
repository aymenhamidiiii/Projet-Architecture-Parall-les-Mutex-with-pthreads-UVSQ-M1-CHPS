#include <stddef.h>
#include <stdio.h>
#include <math.h>

#include <pthread.h>
#include <assert.h>

#define M 5
#define N 8

// ======================== STRUCTURE POUR LES THREADS ===========================

/**
 * Structure pour transmettre les données nécessaires à chaque thread.
 */
typedef struct {
    size_t row;            // Index de la ligne
    size_t n;              // Nombre de colonnes
    double (*A)[N];        // Pointeur vers la matrice
    double *shared_max;    // Pointeur vers la valeur maximale partagée
    pthread_mutex_t *mutex; // Pointeur vers le mutex
} ThreadData;

// ======================= FONCTION EXECUTÉE PAR LES THREADS =====================

/**
 * Fonction exécutée par chaque thread pour trouver le maximum dans une ligne donnée.
 */
void* compute_row_max(void *arg) {
    ThreadData *data = (ThreadData *)arg;  // Cast du paramètre en `ThreadData`
    double local_max = fabs(data->A[data->row][0]);

    // Trouver le maximum pour la ligne spécifiée
    for (size_t j = 1; j < data->n; ++j) {
        if (fabs(data->A[data->row][j]) > local_max) {
            local_max = fabs(data->A[data->row][j]);
        }
    }

    // Mettre à jour la valeur maximale partagée si nécessaire
    pthread_mutex_lock(data->mutex);
    if (local_max > *(data->shared_max)) {
        *(data->shared_max) = local_max;
    }
    pthread_mutex_unlock(data->mutex);

    return NULL;
}

// ============================ FONCTION DE CALCUL ===============================

/**
 * Fonction de référence pour calculer la norme max séquentiellement.
 */
double max_ref(size_t m, size_t n, double A[m][n]) {
  double maxElem = A[0][0];
  for(size_t i = 0; i < m; ++i) {
    for(size_t j = 0; j < n; ++j) {
      if(fabs(A[i][j]) > maxElem) {
        maxElem = A[i][j];
      }
    }
  }

  return maxElem;
}

/**
 * Fonction parallèle pour calculer la norme max en utilisant `pthread`.
 */
double max(size_t m, size_t n, double A[m][n]) {
    double maxElem = A[0][0];  // Valeur maximale partagée
    pthread_mutex_t mutex;    // Mutex pour protéger l'accès à la valeur maximale partagée

    // Initialiser le mutex
    pthread_mutex_init(&mutex, NULL);

    // Créer des threads pour chaque ligne
    pthread_t threads[m];
    ThreadData thread_data[m];

    for (size_t i = 0; i < m; ++i) {
        thread_data[i].row = i;          // Ligne à traiter
        thread_data[i].n = n;            // Nombre de colonnes
        thread_data[i].A = A;            // Pointeur vers la matrice
        thread_data[i].shared_max = &maxElem; // Pointeur vers la valeur maximale partagée
        thread_data[i].mutex = &mutex;   // Pointeur vers le mutex

        // Créer un thread pour traiter la ligne
        pthread_create(&threads[i], NULL, compute_row_max, &thread_data[i]);
    }

    // Attendre la fin de tous les threads
    for (size_t i = 0; i < m; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Détruire le mutex
    pthread_mutex_destroy(&mutex);

    return maxElem;  // Retourner la valeur maximale trouvée
}

// =========================== FONCTIONS UTILES ==================================

/**
 * Initialiser une matrice avec des valeurs incrémentales.
 */
void initMatrix(size_t m, size_t n, double A[m][n]) {
  static double elem = 0.;
  for(size_t i=0; i<m; ++i) {
    for(size_t j=0; j<n; ++j) {
      A[i][j] = elem;
      elem += 1.;
    }
  }
}

/**
 * Afficher une matrice.
 */
void printMatrix(size_t m, size_t n, double A[m][n]) {
  for(size_t i=0; i<m; ++i) {
    for(size_t j=0; j<n; ++j) {
      printf("%lf ", A[i][j]);
    }
    printf("\n");
  }
}

// =============================== MAIN ===========================================

int main(void) {
  size_t n = N;
  size_t m = M;

  double A[m][n];
  initMatrix(m, n, A);

  printf("A=\n");
  printMatrix(m, n, A);

  double ref = max_ref(m, n, A);
  double res = max(m, n, A);
  
  printf("\nref=%lf res=%lf\n", ref, res);
  if(ref == res) {
    printf("OK\n");
  }
  else {
    printf("ERROR: difference between ref and res is above threshold\n");
  }

  return 0;
}
