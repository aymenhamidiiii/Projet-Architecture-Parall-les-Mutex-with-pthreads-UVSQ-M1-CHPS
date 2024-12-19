#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#include <assert.h>

#define M 5  // Nombre de lignes
#define N 8  // Nombre de colonnes

// ======================== STRUCTURE POUR LES THREADS ===========================

/**
 * Structure pour transmettre les données nécessaires à chaque thread.
 */
typedef struct {
    size_t row;            // Index de la ligne
    size_t n;              // Nombre de colonnes
    double (*A)[N];        // Pointeur vers la matrice
    double *shared_sum;    // Pointeur vers la somme partagée
    pthread_mutex_t *mutex; // Pointeur vers le mutex
} ThreadData;

// ======================= FONCTION EXECUTÉE PAR LES THREADS =====================

/**
 * Fonction exécutée par chaque thread pour calculer la somme des carrés d'une ligne donnée.
 */
void* compute_row_sum(void *arg) {
    ThreadData *data = (ThreadData *)arg;  // Cast du paramètre en `ThreadData`
    double row_sum = 0.0;

    // Calculer la somme des carrés pour la ligne spécifiée
    for (size_t j = 0; j < data->n; ++j) {
        row_sum += data->A[data->row][j] * data->A[data->row][j];
    }

    // Accéder à la somme partagée de manière sûre
    pthread_mutex_lock(data->mutex);
    *(data->shared_sum) += row_sum;
    pthread_mutex_unlock(data->mutex);

    return NULL;
}

// ============================ FONCTION DE CALCUL ===============================

/**
 * Fonction de référence pour calculer la norme de Frobenius séquentiellement.
 */
double frobenius_ref(size_t m, size_t n, double A[m][n]) {
    double frob = 0.;
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            frob += A[i][j] * A[i][j];
        }
    }
    return sqrt(frob);
}

/**
 * Fonction parallèle pour calculer la norme de Frobenius en utilisant `pthread`.
 */
double frobenius(size_t m, size_t n, double A[m][n]) {
    double frob = 0.0;  // Somme partagée initialisée à 0
    pthread_mutex_t mutex;  // Mutex pour protéger l'accès à la somme partagée

    // Initialiser le mutex
    pthread_mutex_init(&mutex, NULL);

    // Créer des threads pour chaque ligne
    pthread_t threads[m];
    ThreadData thread_data[m];

    for (size_t i = 0; i < m; ++i) {
        thread_data[i].row = i;          // Ligne à traiter
        thread_data[i].n = n;            // Nombre de colonnes
        thread_data[i].A = A;            // Pointeur vers la matrice
        thread_data[i].shared_sum = &frob; // Pointeur vers la somme partagée
        thread_data[i].mutex = &mutex;   // Pointeur vers le mutex

        // Créer un thread pour traiter la ligne
        pthread_create(&threads[i], NULL, compute_row_sum, &thread_data[i]);
    }

    // Attendre la fin de tous les threads
    for (size_t i = 0; i < m; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Détruire le mutex
    pthread_mutex_destroy(&mutex);

    return sqrt(frob);  // Retourner la racine carrée de la somme
}

// =========================== FONCTIONS UTILES ==================================

/**
 * Initialiser une matrice avec des valeurs incrémentales.
 */
void initMatrix(size_t m, size_t n, double A[m][n]) {
    static double elem = 0.;
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            A[i][j] = elem;
            elem += 1.;
        }
    }
}

/**
 * Afficher une matrice.
 */
void printMatrix(size_t m, size_t n, double A[m][n]) {
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            printf("%lf ", A[i][j]);
        }
        printf("\n");
    }
}

/**
 * Vérifier si deux nombres flottants sont proches (à une tolérance donnée).
 */
inline bool isClose(double a, double b, double threshold) {
    return fabs(a - b) < threshold;
}

// =============================== MAIN ===========================================

int main(void) {
    size_t m = M;  // Nombre de lignes
    size_t n = N;  // Nombre de colonnes

    // Déclaration et initialisation de la matrice
    double A[m][n];
    initMatrix(m, n, A);

    // Affichage de la matrice
    printf("Matrice A =\n");
    printMatrix(m, n, A);

    // Calcul de la norme de Frobenius en version séquentielle
    double ref = frobenius_ref(m, n, A);

    // Calcul de la norme de Frobenius en version parallèle
    double res = frobenius(m, n, A);

    // Affichage des résultats
    printf("\nNorme de Frobenius (référence) = %lf\n", ref);
    printf("Norme de Frobenius (parallèle) = %lf\n", res);

    // Vérification de la validité des résultats
    if (isClose(ref, res, 0.0001)) {
        printf("Résultat correct : OK\n");
    } else {
        printf("Erreur : différence entre les résultats supérieure au seuil\n");
    }

    return 0;
}
