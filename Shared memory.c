#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#define BUFFER_SIZE 1280
#define SHM_SIZE 129

typedef struct {
    char buffer[BUFFER_SIZE][SHM_SIZE];
    int in;
    int out;
    int count;
    sem_t mutex;    // Semaphore for mutual exclusion
    sem_t full;     // Semaphore to track buffer fullness
    sem_t empty;    // Semaphore to track buffer emptiness
} BoundedBuffer;

BoundedBuffer shared_buffer;

typedef struct {
    int degree;
    int* coefficients;
} Polynomial;

void freeMemory(Polynomial *p1,Polynomial *p2,Polynomial *p3);
int splitString(Polynomial* poly1, Polynomial* poly2, char* input);
void addAndSubPolynomials(Polynomial poly1, Polynomial poly2, Polynomial *result, char operator);
void multiplyPolynomials(Polynomial poly1, Polynomial poly2, Polynomial *result);
void printPolynomial(Polynomial poly);


int splitString(Polynomial* poly1, Polynomial* poly2, char* input) {
    input++;

    // Save the first degree
    char* firstDots = strchr(input, ':');
    if (firstDots == NULL) {
        freeMemory(poly1, poly2, poly2);
        return -1;
    } else {
        *firstDots = '\0';
        firstDots++;
        // Save the first degree
        poly1->degree = atoi(input);
        input = firstDots;
    }

    // Find the closing parenthesis for the first polynomial
    char* firstCloser = strchr(input, ')');
    if (firstCloser == NULL) {
        freeMemory(poly1, poly2, poly2);
        return -1;
    } else {
        *firstCloser = '\0';
        input = firstCloser + 1;
    }

    char* secondCloser;
    char* secondDots = strchr(input, '(');
    if (secondDots == NULL) {
        freeMemory(poly1, poly2, poly2);
        return -1;
    } else {
        secondDots++;
        // Find the second degree
        secondCloser = strchr(secondDots, ':');
        if (secondCloser == NULL) {
            freeMemory(poly1, poly2, poly2);
            return -1;
        }
        *secondCloser = '\0';
        poly2->degree = atoi(secondDots);
        secondCloser++;
    }

    secondDots = secondCloser;
    // Find the closing parenthesis for the second polynomial
    secondCloser = strchr(secondDots, ')');
    if (secondCloser == NULL) {
        freeMemory(poly1, poly2, poly2);
        return -1;
    }
    *secondCloser = '\0';

    int count1 = 0;
    char* token = strtok(firstDots, ",");

    // Allocate memory for coefficients of the first polynomial
    poly1->coefficients = (int*)malloc(sizeof(int) * (poly1->degree + 1));
    if (poly1->coefficients == NULL) {
        freeMemory(poly1, poly2, poly2);
        return -1;
    }

    // Initialize coefficients of the first polynomial to 0
    for (int i = 0; i <= poly1->degree; i++) {
        poly1->coefficients[i] = 0;
    }

    // Extract coefficients for the first polynomial
    while (token != NULL) {
        poly1->coefficients[count1] = atoi(token);
        count1++;
        token = strtok(NULL, ",");
    }

    int count2 = 0;
    char* token2 = strtok(secondDots, ",");

    // Allocate memory for coefficients of the second polynomial
    poly2->coefficients = (int*)malloc(sizeof(int) * (poly2->degree + 1));
    if (poly2->coefficients == NULL) {
        freeMemory(poly1, poly2, poly2);
        return -1;
    }

    // Extract coefficients for the second polynomial
    while (token2 != NULL) {
        poly2->coefficients[count2] = atoi(token2);
        count2++;
        token2 = strtok(NULL, ",");
    }

    return 0;
}

void addAndSubPolynomials(Polynomial poly1, Polynomial poly2, Polynomial *result, char operator) {
    // Determine the maximum degree between poly1 and poly2
    int maxDegree = poly1.degree > poly2.degree ? poly1.degree : poly2.degree;

    // Set the degree of the result polynomial to the maximum degree
    result->degree = maxDegree;

    // Allocate memory for the result coefficients
    result->coefficients = (int*)malloc(sizeof(int) * (maxDegree + 1));
    if(result->coefficients == NULL){
        freeMemory(&poly1,&poly2,result);
        return;
    }

    int deg = abs(poly1.degree - poly2.degree);

    // Iterate through each coefficient from 0 to the maximum degree
    for (int i = maxDegree, j = 0; i >= 0; i--, j++) {
        if(poly1.degree == 0){
            result->coefficients = poly2.coefficients;
        }else if(poly2.degree == 0){
            result->coefficients = poly1.coefficients;
        } else if(poly1.degree == 1){
            for (int k = 0; k <= poly2.degree; ++k) {
                result->coefficients[k] = poly1.coefficients[0] * poly2.coefficients[k];
            }
        }else if(poly2.degree == 1){
            for (int k = 0; k <= poly1.degree; ++k) {
                result->coefficients[k] = poly2.coefficients[0] * poly1.coefficients[k];
            }
        }

        // Retrieve the corresponding coefficients from poly1 and poly2
        // If the index exceeds the degree of a polynomial, use a coefficient of 0
        int coefficient1 = (i >= deg) ? poly1.coefficients[poly1.degree - j] : 0;
        int coefficient2 = (i >= deg) ? poly2.coefficients[poly2.degree - j] : 0;
        if(i < deg){
            if(poly1.degree > poly2.degree){
                coefficient1 = poly1.coefficients[poly1.degree - j];
            }
            else if(poly1.degree < poly2.degree){
                coefficient2 = poly2.coefficients[poly2.degree - j];
            }
        }
        // Add  or sub the coefficients together and store the result in the result polynomial
        if(operator == '+') {
            result->coefficients[i] = coefficient1 + coefficient2;
        } else{
            result->coefficients[i] = coefficient1 - coefficient2;
        }
    }
}

void freeMemory(Polynomial* p1,Polynomial *p2,Polynomial *p3){
    free(p1->coefficients);
    free(p2->coefficients);
    free(p3->coefficients);
}

void multiplyPolynomials(Polynomial poly1, Polynomial poly2, Polynomial *result) {
    // Calculate the degree of the resulting polynomial
    int resultDegree = poly1.degree + poly2.degree;
    // Set the degree of the result polynomial
    result->degree = resultDegree;
    // Allocate memory for the result coefficients
    result->coefficients = (int*)malloc(sizeof(int) * (resultDegree + 1));
    if(result->coefficients == NULL){
        return;
    }

    //creat a temporary polynomials and make them zero
    int p1[poly1.degree + 1], p2[poly2.degree + 1], res[resultDegree + 1];
    for (int i = 0; i <= poly1.degree; ++i) {
        p1[i] = 0;
    }
    for (int i = 0; i <= poly2.degree; ++i) {
        p2[i] = 0;
    }
    for (int i = 0; i <= resultDegree; ++i) {
        res[i] = 0;
    }
    //initialize them to the revers of the original polynomials
    for (int i = 0; i <= poly1.degree; ++i) {
        p1[i] = poly1.coefficients[poly1.degree - i];
    }
    for (int i = 0; i <= poly2.degree; ++i) {
        p2[i] = poly2.coefficients[poly2.degree - i];
    }
    //calculate them
    for (int i = 0; i <= poly1.degree; ++i) {
        for (int j = 0; j <= poly2.degree; ++j) {
            res[i + j] += p1[i] * p2[j];
        }
    }
    //go back to the original
    for (int i = 0; i <= resultDegree; ++i) {
        result->coefficients[i] = res[resultDegree -i];
    }
}

void printPolynomial(Polynomial poly) {
    if(poly.coefficients[0] != 0) {
        // Print the highest degree term
        printf("%dx^%d", poly.coefficients[0], poly.degree);
    }

    // Iterate through each term from the next highest degree to 0
    for (int i = 1; i <= poly.degree ; i++) {
        // Check if the coefficient is not zero
        if (poly.coefficients[i] != 0) {
            if(poly.coefficients[i] > 0) {
                if(poly.coefficients[0] != 0) {
                    // Print the term with a plus sign
                    printf(" + %d", poly.coefficients[i]);
                }else{
                    // Print the term with a plus sign
                    printf("%d", poly.coefficients[i]);
                }
            } else if(poly.coefficients[i] < 0){
                poly.coefficients[i] = -1*poly.coefficients[i];
                // Print the term with a plus sign
                printf(" - %d", poly.coefficients[i]);
            }
            if (poly.degree - i == 1){
                printf("x");
            }
            else if (poly.degree - i != 0){
                printf("x^%d", poly.degree - i);
            }
        }
    }

    // Print a newline character at the end
    printf("\n");
}

void init_buffer(BoundedBuffer* buffer) {
    buffer->in = 0;
    buffer->out = 0;
    buffer->count = 0;
    sem_init(&(buffer->mutex), 0, 1);   // Initialize mutex semaphore to 1
    sem_init(&(buffer->full), 0, 0);    // Initialize full semaphore to 0
    sem_init(&(buffer->empty), 0, BUFFER_SIZE);   // Initialize empty semaphore to BUFFER_SIZE
}

void* producer(void* arg) {
    // Cast the argument to a BoundedBuffer* type to access the shared buffer
    BoundedBuffer* buffer = (BoundedBuffer*)arg;

    // Declare a character array to store user input
    char input[SHM_SIZE];

    while (1) {
        // Read a line of input from the user, with a maximum length of SHM_SIZE characters
        fgets(input, SHM_SIZE, stdin);

        // Wait for an empty slot in the buffer by decrementing the empty semaphore
        sem_wait(&(buffer->empty));

        // Acquire the mutex semaphore to ensure exclusive access to the buffer
        sem_wait(&(buffer->mutex));

        // Copy the input data into the buffer at the current position (buffer->in) and update the position
        strcpy(buffer->buffer[buffer->in], input);
        buffer->in = (buffer->in + 1) % BUFFER_SIZE;

        // Increase the count of items in the buffer
        buffer->count++;

        // Release the mutex semaphore
        sem_post(&(buffer->mutex));

        // Increment the full semaphore to indicate that new data is available in the buffer
        sem_post(&(buffer->full));
    }
}

void* consumer(void* arg) {
    // Cast the argument to a BoundedBuffer* type to access the shared buffer
    BoundedBuffer* buffer = (BoundedBuffer*)arg;

    while (1) {
        // Wait for the full semaphore to indicate that the buffer has some data
        sem_wait(&(buffer->full));

        // Acquire the mutex semaphore to ensure exclusive access to the buffer
        sem_wait(&(buffer->mutex));

        // Read data from the buffer at the current position (buffer->out) and update the position
        char data[SHM_SIZE];
        strcpy(data, buffer->buffer[buffer->out]);
        buffer->out = (buffer->out + 1) % BUFFER_SIZE;

        // Decrease the count of items in the buffer
        buffer->count--;

        // Declare variables for polynomials, operator, and result
        Polynomial poly1, poly2, result;
        char operator;

        // Check if the data contains "END" or exceeds 128 characters
        if (strstr(data, "END") != NULL || strlen(data) > 128) {
            // Free the allocated memory for polynomials
            freeMemory(&poly1, &poly2, &result);
            // Release the mutex semaphore
            sem_post(&(buffer->mutex));
            // Increment the empty semaphore to indicate a free slot in the buffer
            sem_post(&(buffer->empty));
            // If either condition is true, exit the loop and return from the thread
            exit(0);
        }

        // Find the operator in the data
        if (strstr(data, "ADD") != NULL) {
            operator = '+';
        } else if (strstr(data, "SUB") != NULL) {
            operator = '-';
        } else if (strstr(data, "MUL") != NULL) {
            operator = '*';
        } else {
            printf("Invalid operator.\n");
            return NULL;
        }

        // Split the input data into two polynomials
        if (splitString(&poly1, &poly2, data) != 0) {
            printf("Invalid input format.\n");
            freeMemory(&poly1, &poly2, &result);
            return NULL;
        }

        // Calculate the degree of the result polynomial
        result.degree = poly1.degree * poly2.degree;

        // Allocate memory for the coefficients of the result polynomial
        result.coefficients = malloc(sizeof(int) * (result.degree + 1));
        if (result.coefficients == NULL) {
            freeMemory(&poly1, &poly2, &result);
            exit(1);
        }

        // Perform addition, subtraction, or multiplication based on the operator
        if (operator == '+' || operator == '-') {
            addAndSubPolynomials(poly1, poly2, &result, operator);
            printPolynomial(result);
        } else {
            multiplyPolynomials(poly1, poly2, &result);
            printPolynomial(result);
        }

        // Free the allocated memory for polynomials
        freeMemory(&poly1, &poly2, &result);

        // Release the mutex semaphore
        sem_post(&(buffer->mutex));

        // Increment the empty semaphore to indicate a free slot in the buffer
        sem_post(&(buffer->empty));
    }
}


int main() {
    int shm_id;
    key_t key;
    char *shm_ptr;

    // Generate a key for the shared memory
    key = ftok(".", 'S');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    // Create the shared memory segment
    shm_id = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    // Attach the shared memory segment
    shm_ptr = (char *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (char *)(-1)) {
        perror("shmat");
        exit(1);
    }

    // Initialize the bounded buffer
    init_buffer(&shared_buffer);

    // Create producer and consumer threads
    pthread_t producer_thread, consumer_thread;
    pthread_create(&producer_thread, NULL, producer, (void*)&shared_buffer);
    pthread_create(&consumer_thread, NULL, consumer, (void*)&shared_buffer);

    // Wait for threads to finish
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    // Detach and remove the shared memory segment
    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}



