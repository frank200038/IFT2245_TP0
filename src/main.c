#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "errno.h"


typedef unsigned char byte;
typedef int error_code;

#define ERROR (-1)
#define HAS_ERROR(code) ((code) < 0)
#define HAS_NO_ERROR(code) ((code) >= 0)

/**
 * Cette fonction compare deux chaînes de caractères.       
 * @param p1 la première chaîne
 * @param p2 la deuxième chaîne
 * @return le résultat de la comparaison entre p1 et p2. Un nombre plus petit que
 * 0 dénote que la première chaîne est lexicographiquement inférieure à la deuxième.
 * Une valeur nulle indique que les deux chaînes sont égales tandis qu'une valeur positive
 * indique que la première chaîne est lexicographiquement supérieure à la deuxième.
 */
int strcmp(char *p1, char *p2) {
    char *s1 = (char *) p1;
    char *s2 = (char *) p2;
    char c1, c2;
    do {
        c1 = (char) *s1++;
        c2 = (char) *s2++;
        if (c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);
    return c1 - c2;
}

/**
 * Ex. 1: Calcul la longueur de la chaîne passée en paramètre selon
 * la spécification de la fonction strlen standard
 * @param s un pointeur vers le premier caractère de la chaîne
 * @return le nombre de caractères dans le code d'erreur, ou une erreur
 * si l'entrée est incorrecte
 */
error_code strlen2(char *s) {
    int i=0;
    while(s[i] != '\0'){
        i++;
    }
    return i;
}

/**
 * Ex.2 :Retourne le nombre de lignes d'un fichier sans changer la position
 * courante dans le fichier.
 * @param fp un pointeur vers le descripteur de fichier
 * @return le nombre de lignes, ou -1 si une erreur s'est produite
 */
error_code no_of_lines(FILE *fp) {

    if (fp == NULL){
        return -1;
    }

    long cursor = ftell(fp); // Locate the current file cursor, memorize it
    fseek(fp, 0, SEEK_SET); // Move cursor back to the start, to count all lines

    int ch = getc(fp);
    int lines = 0;
    while(ch != EOF){
        if((char) ch == '\n'){
            lines++;
        }
        ch = getc(fp);

        // When we reach the end of file, it counts a new line too.
        // No need to consider empty file, as the while loop won't even be executed
        if(ch == EOF){
            lines++;
        }
    }

    // Move cursor back to the original position
    fseek(fp, cursor, SEEK_SET);

    return lines;
}

/**
 * Ex.3: Lit une ligne au complet d'un fichier
 * @param fp le pointeur vers la ligne de fichier
 * @param out le pointeur vers la sortie
 * @param max_len la longueur maximale de la ligne à lire
 * @return le nombre de caractère ou ERROR si une erreur est survenue
 */

error_code readline(FILE *fp, char **out, size_t max_len) {

    // Because our goal is to eventually transfer the char* (string) to another pointer.
    // So we would need to add the null character add the end so we can mark the end of the char* (string)
    // Also if we need to have absolutely maximum max_len character in the string, we need to allocate max_len + 1
    // to accommodate the \0 symbol
    char *output = malloc(max_len + 1);

    if (fp == NULL || output == NULL){
        return ERROR;
    } else{
        int current = fgetc(fp);
        int index = 0;
        int length = 0;

        while(current != EOF && index < max_len + 1){

            char c = (char) current;
            if( current != '\n' && current != '\0') {
                length++;
                output[index] = (char) current;
            } else if(current == '\n'){
                break;
            }

            index++;

            current = fgetc(fp);
        }

        // Add null character at the end to mark the end of string, as we need to transfer it to another pointer
        output[index] = '\0';

        *out = output;

        return length;
    }

}

/**
 * Ex.4: Copie un bloc mémoire vers un autre
 * @param dest la destination de la copie
 * @param src  la source de la copie
 * @param len la longueur (en byte) de la source
 * @return nombre de bytes copiés ou une erreur s'il y a lieu
 */
error_code memcpy2(void *dest, void *src, size_t len) {
    if(dest == NULL || src == NULL)
        return ERROR;

    // Always safe to cast to a char pointer
    char *destC = (char *) dest;
    char *srcC = (char *) src;

    int i;
    for(i = 0; i < len; i++){
        destC[i] = srcC[i];
    }

    return i;
}

/**
 * Ex.5: Analyse une ligne de transition
 * @param line la ligne à lire
 * @param len la longueur de la ligne
 * @return la transition ou NULL en cas d'erreur
 */
transition *parse_line(char *line, size_t len) {
    transition *t = malloc(sizeof(transition));

    char *lineParse = malloc(len);

    if (lineParse == NULL){
        free(t);
        return NULL;
    }


    int i = 0;
    while(line[i] != '\0'){
        lineParse[i] = line[i];
        i++;
    }

    char *currentState = malloc(5);
    char *nextState = malloc(5);

    if (currentState == NULL || nextState == NULL){
        free(t);
        free(lineParse);
        free(currentState);
        free(nextState);
        return NULL;
    }

    i=1;
    int current = 0;

    // Extract Current State
    while(i < (int) len && lineParse[i] != ','){
        currentState[current] = lineParse[i];
        i++;
        current++;
    }

    i++;
    current = 0;

    t->current_state = currentState;
    t->read = lineParse[i];

    i+=5; // Start of next state symbol

    //Extract Next State
    while(i < (int) len && lineParse[i] != ','){
        nextState[current] = lineParse[i];
        i++;
        current++;
    }

    i++;

    t->next_state = nextState;
    t->write = lineParse[i];

    i+=2;
    switch (lineParse[i]) {
        case 'G': t->movement = -1; break;
        case 'R': t->movement = 0; break;
        default: t->movement = 1; break;
    }

    free(lineParse);


    return t;

}

/**
 * Ex.6: Execute la machine de turing dont la description est fournie
 * @param machine_file le fichier de la description
 * @param input la chaîne d'entrée de la machine de turing
 * @return le code d'erreur
 */
error_code execute(char *machine_file, char *input) {
    FILE *file = fopen(machine_file, "r");
    if(file == NULL) return ERROR;

    int noLines = no_of_lines(file);

    transition transitions[noLines-3];
    char *initial, *accept, *reject;
    for(int i = 0; i< noLines; i++){
        char **line = malloc(sizeof (char *));
        if (line == NULL){
            free(line);
            free(file);
            return ERROR;
        }

        int len = readline(file, line, 1024);
        switch (i) {
            case 0: initial = *line; break;
            case 1: accept = *line; break;
            case 2: reject = *line; break;
            default: {
                transition *t = parse_line(*line, len);
                if(t == NULL){
                    free(line);
                    free(file);
                    return ERROR;
                } else{
                    transitions[i-3] = *t;
                }
            }
        }
    }

    char *current = initial;

    int expand = 2;

    // Initiate turing machine tape
    int inputSize = strlen2(input) == 0 ? noLines : strlen2(input) ;
    char *tape = malloc(inputSize * expand);
    if (tape == NULL){
        free(file);
        free(initial);
        free(accept);
        free(reject);
        return ERROR;
    }

    for(int i = 0; i < inputSize * expand; i++){
        tape[i] = ' ';
    }

    memcpy2(tape, input, strlen2(input));

    int j = 0;

    while(1){
        char currentCh = tape[j];
        int found = 0;

        for(int i = 0; i < noLines - 3; i++){
            transition currentT = transitions[i];

            if(strcmp(currentT.current_state, current) == 0 && (currentT.read == currentCh)){
                found = 1;
                current = currentT.next_state;
                tape[j] = currentT.write;
                switch (currentT.movement) {
                    case -1: {
                        if (j == 0) {
                            free(initial);
                            free(accept);
                            free(reject);
                            free(tape);
                            return ERROR;
                        }
                        else j--;
                        break;
                    }
                    case 1: j++; break;
                    default: break;
                }

                if(strcmp(accept, current) == 0){
                    return 1;
                } else if (strcmp(reject, current) == 0){
                    return 0;
                }
            }

            if(j > inputSize * expand){
                    char *newTape = realloc(tape, inputSize * expand * 2);
                    expand *= 2;
                    if(newTape == NULL){
                        free(tape);
                        free(initial);
                        free(accept);
                        free(reject);
                        return ERROR;
                    }

                    for(int index = inputSize * expand / 2; i < inputSize * expand; i++){
                        newTape[index] = ' ';
                    }

                    tape = newTape;
                }
                if(found) break;
            }

            if(!found) {
                free(tape);
                free(initial);
                free(accept);
                free(reject);
                return ERROR;
            }
        }
    }

// ATTENTION! TOUT CE QUI EST ENTRE LES BALISES ༽つ۞﹏۞༼つ SERA ENLEVÉ! N'AJOUTEZ PAS D'AUTRES ༽つ۞﹏۞༼つ

// ༽つ۞﹏۞༼つ

int main() {
// ous pouvez ajouter des tests pour les fonctions ici

    errno = 0;

//    char **read = malloc(sizeof(char *));
//
//    FILE *test_file = fopen("../five_lines", "r");
//    readline(test_file, read, 1024);
//
//    char *line = *read;
//
//    printf("%s\n",line);
//    free(*read);
//
//    free(read);
//    fclose(test_file);
    printf("%d\n",execute("../youre_gonna_go_far_kid", "STARING AT THE SUN"));
    return 0;
}
//    printf("%s\n",initial);
//    printf("%s\n",accept);
//    printf("%s\n",reject);
//
//
//    for(int k = 0; k < noLines-3;k++){
//        transition t = transitions[k];
//        printf("Transition %d is: \n", k);
//        printf("current: %s\n", t.current_state);
//        printf("next: %s\n", t.next_state);
//        printf("movement: %c\n", t.movement);
//        printf("read: %c\n", t.read);
//        printf("write: %c\n", t.write);
//        printf("------------------------\n");
//
//    }

// ༽つ۞﹏۞༼つ