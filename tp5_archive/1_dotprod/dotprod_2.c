#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#include <assert.h>

// Définitions des constantes
#define N 9   // Taille totale des tableaux
#define K 3   // Taille d'un bloc (nombre d'éléments par thread)

// ======================== STRUCTURE POUR LES THREADS ===========================

/**
 * Structure pour transmettre les paramètres nécessaires à chaque thread.
 * Cette structure contient les indices du bloc, les tableaux concernés,
 * un pointeur vers la somme partagée et un mutex pour la synchronisation.
 */
typedef struct {
    size_t start;          // Index de début du bloc
    size_t end;            // Index de fin du bloc
    double *a;             // Pointeur vers le tableau `a`
    double *b;             // Pointeur vers le tableau `b`
    double *shared_sum;    // Pointeur vers la somme partagée
    pthread_mutex_t *mutex; // Pointeur vers le mutex
} ThreadData;

// ======================= FONCTION EXECUTÉE PAR LES THREADS =====================

/**
 * Fonction de calcul exécutée par chaque thread.
 * Chaque thread calcule le produit scalaire pour un bloc donné et met à jour
 * la somme partagée dans une section critique protégée par un mutex.
 */
void* compute_block(void *arg) {
    ThreadData *data = (ThreadData *)arg;  // Cast du paramètre reçu en `ThreadData`
    double block_sum = 0.0;

    // Calcul du produit scalaire pour les éléments du bloc
    for (size_t i = data->start; i < data->end; ++i) {
        block_sum += data->a[i] * data->b[i];
    }

    // Mise à jour de la somme partagée (section critique)
    pthread_mutex_lock(data->mutex);
    *(data->shared_sum) += block_sum;
    pthread_mutex_unlock(data->mutex);

    return NULL;  // Les threads renvoient NULL ici par convention
}

// ============================ FONCTIONS DE CALCUL ==============================

/**
 * Fonction de référence (version séquentielle).
 * Elle calcule le produit scalaire classique pour un tableau donné.
 * La définition est supposée être dans un fichier séparé.
 */
double dotprod_ref(size_t n, double a[n], double b[n]);

/**
 * Fonction parallèle pour calculer le produit scalaire.
 * Les tableaux `a` et `b` sont divisés en blocs de taille `k`, et chaque bloc
 * est traité par un thread indépendant.
 */
double dotprod_blocks(size_t n, size_t k, double a[n], double b[n]) {
    double sum = 0.0;  // Somme partagée initialisée à 0
    pthread_mutex_t mutex;  // Mutex pour protéger l'accès à la somme partagée

    // Initialisation du mutex
    pthread_mutex_init(&mutex, NULL);

    // Calcul du nombre de threads nécessaires
    size_t nb_threads = n / k;
    pthread_t threads[nb_threads];         // Tableau des identifiants des threads
    ThreadData thread_data[nb_threads];   // Tableau des données pour chaque thread

    // Création des threads
    for (size_t i = 0; i < nb_threads; ++i) {
        thread_data[i].start = i * k;        // Début du bloc
        thread_data[i].end = (i + 1) * k;   // Fin du bloc
        thread_data[i].a = a;               // Pointeur vers le tableau `a`
        thread_data[i].b = b;               // Pointeur vers le tableau `b`
        thread_data[i].shared_sum = &sum;   // Pointeur vers la somme partagée
        thread_data[i].mutex = &mutex;      // Pointeur vers le mutex

        // Création du thread pour traiter le bloc
        pthread_create(&threads[i], NULL, compute_block, &thread_data[i]);
    }

    // Attendre la fin de tous les threads
    for (size_t i = 0; i < nb_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Destruction du mutex (nettoyage)
    pthread_mutex_destroy(&mutex);

    return sum;  // Retourner la somme calculée
}

// =========================== FONCTIONS UTILES ==================================

/**
 * Initialisation d'un tableau avec des valeurs incrémentales.
 * Exemple : a[0] = 0, a[1] = 1, a[2] = 2, etc.
 */
void initArray(size_t n, double a[n]) {
    static double elem = 0.0;  // Valeur initiale statique
    for (size_t i = 0; i < n; ++i) {
        a[i] = elem;
        elem += 1.0;  // Incrémentation
    }
}

/**
 * Affichage des éléments d'un tableau.
 * Chaque élément est imprimé séparément.
 */
void printArray(size_t n, double a[n]) {
    for (size_t i = 0; i < n; ++i) {
        printf("%lf ", a[i]);
    }
    printf("\n");
}

/**
 * Vérification si deux nombres flottants sont proches (à une tolérance donnée).
 * Utile pour comparer les résultats avec une tolérance fixée.
 */
inline bool isClose(double a, double b, double threshold) {
    return fabs(a - b) < threshold;
}

// =============================== MAIN ===========================================

int main(void) {
    size_t n = N;  // Taille des tableaux
    size_t k = K;  // Taille des blocs

    // Déclaration et initialisation des tableaux
    double a[n];
    double b[n];
    initArray(n, a);
    initArray(n, b);

    // Affichage des tableaux
    printf("Tableau a =\n");
    printArray(n, a);
    printf("Tableau b =\n");
    printArray(n, b);

    // Calcul du produit scalaire en version séquentielle (référence)
    double ref = dotprod_ref(n, a, b);

    // Calcul du produit scalaire en version parallèle
    double res = dotprod_blocks(n, k, a, b);

    // Affichage des résultats
    printf("\nProduit scalaire (référence) = %lf\n", ref);
    printf("Produit scalaire (parallèle) = %lf\n", res);

    // Vérification de la validité des résultats
    if (isClose(ref, res, 0.0001)) {
        printf("Résultat correct : OK\n");
    } else {
        printf("Erreur : différence entre les résultats supérieure au seuil\n");
    }

    return 0;  // Fin du programme
}
