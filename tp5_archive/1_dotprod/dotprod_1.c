#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#include <assert.h>

// Définir la taille du tableau
#define N 10

// ========================= STRUCTURE POUR LES THREADS ============================

// Structure utilisée pour transmettre les paramètres nécessaires à chaque thread
typedef struct {
    size_t index;          // Index de l'élément du tableau à traiter
    double *a;             // Pointeur vers le tableau `a`
    double *b;             // Pointeur vers le tableau `b`
    double *shared_sum;    // Pointeur vers la somme partagée (variable globale)
    pthread_mutex_t *mutex; // Pointeur vers le mutex pour la synchronisation
} ThreadData;

// ======================== FONCTION EXECUTÉE PAR LES THREADS ======================

/**
 * Fonction de calcul exécutée par chaque thread.
 * Elle calcule le produit de deux éléments des tableaux `a` et `b` pour un index donné,
 * et ajoute ce produit à la somme partagée en utilisant un mutex pour la synchronisation.
 */
void* compute_product(void *arg) {
    ThreadData *data = (ThreadData *)arg;  // Cast du paramètre reçu en `ThreadData`

    // Calcul du produit de deux éléments des tableaux
    double product = data->a[data->index] * data->b[data->index];

    // Ajouter le produit à la somme partagée (section critique protégée par un mutex)
    pthread_mutex_lock(data->mutex);
    *(data->shared_sum) += product;
    pthread_mutex_unlock(data->mutex);

    return NULL;  // Les threads renvoient NULL par convention ici
}

// =========================== FONCTIONS DE CALCUL ================================

/**
 * Fonction de référence (version séquentielle) pour calculer le produit scalaire.
 * La définition est supposée donnée dans un fichier séparé.
 */
double dotprod_ref(size_t n, double a[n], double b[n]);

/**
 * Fonction de calcul parallèle du produit scalaire.
 * Chaque thread calcule une contribution individuelle, qui est ensuite accumulée
 * dans une variable partagée protégée par un mutex.
 */
double dotprod_pairs(size_t n, double a[n], double b[n]) {
    double sum = 0.0;  // Somme partagée initialisée à 0
    pthread_t threads[n];  // Tableau pour stocker les identifiants des threads
    ThreadData thread_data[n];  // Tableau pour stocker les données des threads
    pthread_mutex_t mutex;  // Mutex pour protéger l'accès à la somme partagée

    // Initialisation du mutex
    pthread_mutex_init(&mutex, NULL);

    // Création des threads pour traiter chaque élément du tableau
    for (size_t i = 0; i < n; ++i) {
        thread_data[i].index = i;        // Définir l'index du thread
        thread_data[i].a = a;           // Passer le tableau `a`
        thread_data[i].b = b;           // Passer le tableau `b`
        thread_data[i].shared_sum = &sum; // Passer la somme partagée
        thread_data[i].mutex = &mutex;  // Passer le mutex

        // Création du thread et affectation de la fonction `compute_product`
        pthread_create(&threads[i], NULL, compute_product, &thread_data[i]);
    }

    // Attente de la terminaison de tous les threads
    for (size_t i = 0; i < n; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Destruction du mutex (nettoyage)
    pthread_mutex_destroy(&mutex);

    return sum;  // Retourner la somme calculée
}

// ========================= FONCTIONS UTILES =====================================

/**
 * Initialiser un tableau avec des valeurs incrémentales (0, 1, 2, ...).
 */
void initArray(size_t n, double a[n]) {
    static double elem = 0.;
    for (size_t i = 0; i < n; ++i) {
        a[i] = elem;
        elem += 1.;
    }
}

/**
 * Afficher les éléments d'un tableau.
 */
void printArray(size_t n, double a[n]) {
    for (size_t i = 0; i < n; ++i) {
        printf("%lf ", a[i]);
    }
    printf("\n");
}

/**
 * Vérifier si deux valeurs flottantes sont proches l'une de l'autre.
 * Utile pour comparer les résultats avec une tolérance donnée.
 */
inline bool isClose(double a, double b, double threshold) {
    return fabs(a - b) < threshold;
}

// =============================== MAIN ============================================

int main(void) {
    size_t n = N;  // Taille des tableaux

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
    double res = dotprod_pairs(n, a, b);

    // Affichage des résultats
    printf("\nProduit scalaire (référence) = %lf\n", ref);
    printf("Produit scalaire (parallèle) = %lf\n", res);

    // Vérification de la validité des résultats
    if (isClose(ref, res, 0.0001)) {
        printf("Résultat correct : OK\n");
    } else {
        printf("Erreur : différence entre les résultats supérieure au seuil\n");
    }

    return 0;  // Terminer le programme
}
