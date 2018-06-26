//list.c
//This program represents a singlly linked list in C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// A node in a singly-linked list
struct node {
    char *value;
    struct node *next;
};

// Head of the linked list

//declaration of variables

// Insert a value into linked list
// void list_insert(char* v,struct node *head, int *nelms) {
//     struct node *n;
//     n = (struct node *)malloc(sizeof(struct node));
//     n->value = malloc(strlen(v)+1);
//     strncpy(n->value,v,strlen(v));

//     printf("Just inserted:[%s]\n",n->value);
//     n->next = head;
//     head = n;
//     *nelms+=1;
//    // n = head;
//      printf("head->value is:[%s]\n",head->value);
// }
void list_insert(char* v,struct node *head, int *nelms) {
    //struct node *n;
   // n = (struct node *)malloc(sizeof(struct node));
    struct node *current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    /* now we can add a new variable */
    current->next = malloc(sizeof(struct node));

    current->next->value = malloc(strlen(v)+1);
    strcpy(current->next->value,v);
    current->next->next = NULL;
    printf("Just inserted:[%s]\n",current->next->value);
    //n->next = head;
    //head = n;
    *nelms+=1;
   // n = head;
     printf("head->value is:[%s]\n",head->value);
}

// Insert two values at once into linked list
void list_insert2(char* a, char* b,struct node *head, int *nelms) {
    struct node *v, *u;

    u =(struct node *)malloc(sizeof(struct node));
    v =(struct node *)malloc(sizeof(struct node));
    strcpy(u->value,a);
    u->next = v;
    strcpy(v->value, b);
    printf("Just inserted:[%s]\n",u->value);
    printf("Just inserted:[%s]\n",v->value);
    v->next = head;
    head = u; 
    *nelms +=2;
        
}

// Remove an element from linked list
void list_remove(char* v, struct node *head, int *nelms) {
  struct node *n=head;
  struct node *previous = head;
  //continue if value is not 
  //found here
  printf("HERE1\n");

  //n->value is declared as "char value[50]"
  //this error started occuring when I put
  // int *nelms as a pramter to the function...
  printf("n->value is %s\n",n->value);
   while(strcmp(n->value,v) != 0) {//0
      printf("Here 2\n");
      previous = n;
      n = n->next;
    }
    if(n == head) {
      printf("here!!!\n");
        head = head->next;
          }
   else if(n->next == NULL){
     
      previous->next = NULL;
       }
   else{
         previous->next =n->next;
 }

    *nelms -=1;

     free(n);
  //    free(previous);

  }

// Print out all values in linked list
void list_printall(struct node *head, int *nelms) {
    struct node *p= head;
   int n = 0;
    while(n< *nelms) {
        printf("%s ", p->value);
        p=p->next;
        n++;
        if(n > 100) break;
    }
    printf("\n");
    p =head;
    }


// Deallocate all memory used in linked list
void list_destroy(struct node *head) {
   struct node *temp;
   while(head != NULL){
   temp = head->next;
   free(head);
   head = temp;  
  }
}

int main(int argc, char *argv[]) {
    printf("Test linked lists\n");
    //struct node *u, *v;
    //struct node *n;
    int nelms=0;
    struct node head;
    //head =  NULL;
    list_printall(&head,&nelms); // Should print nothing
    list_insert("42",&head,&nelms);
    printf("head->value immidiatley after insert function: [%s]\n",head.value);
    printf("Nelms is: %i\n",nelms);
    list_insert2("17", "10",&head,&nelms);
    printf("head value immidiatley after insert function: [%s]\n",head.value);
    printf("Nelms is: %i\n",nelms);

    list_insert("18",&head,&nelms);
     printf("Nelms is: %i\n",nelms);
     printf("-----------------\n");
     list_printall(&head, &nelms); // Should print 18 17 42
     printf("------------------\n");
    list_remove("10",&head,&nelms);
     printf("Nelms is: %i\n",nelms);
    list_printall(&head, &nelms); // Should print 18 17 42
    // Cleanup memory
    list_destroy(&head);
    return 0;
}
