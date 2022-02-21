#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define MAX_LENGHT 256
#define ROOM 0.4

int tablesize;
char* normalize (char* word);
int* t9(char* word);

/*
 * Entradas da hash_table -
 * Usar a codificação t9 para a hashtable
 */
typedef struct entry_t {
  //Palavra lida
  char word[40];
  
  //Codificação em t9
  int* cod;
  size_t size;

  //Ocorrências
  int occ;

  //Em caso de colisões
  struct entry_t* next;
  int shift;

}entry_t;

//Struct da hash_table
typedef struct {
  entry_t** entries;
} ht_t;

//Exponenciação
int power(int base, int exponent) {
  int result=1;
  for (; exponent>0; exponent--) {
    result = result * base;
  }
  return result;
}

//Hash function
unsigned int hash(int* cod, int size) {
  int i, h = cod[0];
  for(i = 1; i<size; i++) {
    h = h * 256 + cod[i];
  }
  return h % tablesize;
}

//Criar e inicializar a hashtable
ht_t* create(void) {

  ht_t *hashtable = malloc(sizeof(ht_t) * 1);

  hashtable->entries = malloc(sizeof(entry_t*) * (tablesize));

  //Definir como NULL cada entrada
  for(int i = 0; i < (tablesize); i++) {
    hashtable->entries[i] = NULL;
  }
  return hashtable;
}

//Trocar as strings para manter a ordem
void swap(char* str1, char* str2) {
  char *temp = (char *)malloc((strlen(str1) + 1) * sizeof(char));
  strcpy(temp, str1);
  strcpy(str1, str2);
  strcpy(str2, temp);
  free(temp);
}

//Comparar as duas codificações
int codcmp(int* cod1, int* cod2, int size1, int size2) {
  if(size1 != size2) return 1;
  for(int i = 0; i<size1; i++) {
    if(cod1[i] != cod2[i]) return 1;
  }
  return 0;
}

//Devolve a entrada com base no hashcode
int getSlot(ht_t *hashtable, int slot, int* cod, int size) {
  entry_t *entry = hashtable -> entries[slot];
  if(entry == NULL){
    return -1;
  }
  while(codcmp(entry->cod,cod,entry->size,size) != 0) {
    //printf("slot: %d\n",slot);
    slot++;
    entry = hashtable -> entries[slot];
    if(entry == NULL){
      return -1;
    }
  }
  return slot;
}

//Anda pelas palavras com a mesma codificação
char* circle_words(ht_t *hashtable, int slot) {
  entry_t *entry = hashtable->entries[slot];
  entry_t *first = entry;
  if (entry == NULL) {
    printf("Não existe qualquer palavra com essa configuração\n");
    return NULL;
  }
  for(int i = 1; entry != NULL; i++) {
    printf("%d: %s\n",i,entry->word);
    entry = entry->next;

  }
  entry = first;
  size_t size;
  char* line = NULL;
  size = getline(&line,&size,stdin);
  int n = atoi(line);

  for(int i = 1; i<n; i++) entry = entry->next;
  return entry->word;
}

//Devolve a letra equivalente na tabela ASCII
int getASCII(int c) {
  //A
  if((c >= -95 && c <= -93) || (c >= -127 && c <= -125 )) return 97;
  //C
  if(c == -89 || c == -121) return 99;
  //E
  else if( c == -87 || c == -86 || c == -119 || c == -119 ) return 101;
  //I
  else if( c == -83 || c == -115) return 105;
  //O
  else if((c >= -77 && c <= -75) || (c >= -109 && c <= -107)) return 111;
  //U
  else if( c == -70 || c == -102) return 117;
  else return -1;
}

//Devolve a palavra equivalente sem caracteres especiais
char* normalize(char* word) {
  int size = strlen(word);
  for(int i = 0; i<strlen(word); i++) {
    if(word[i] == -61) {
      word[i] = getASCII(word[i+1]);
    }
  }
  for(int i = 0; i<strlen(word); i++) {
    if(word[i] < 0) {
        int temp = word[i];
        word[i] = word[i+1];
        word[i+1] = temp;
    }
  }
  word[size] = '\0';
  return word;
}

//Codificação t9 - passar a palavra já normalizada
int* t9(char* word) {
  int size = strlen(word);
  int* cod = malloc(sizeof(int)*size);
  for(int i = 0; i<size; i++) {
    switch (word[i]) {
      case 'a': case 'b': case 'c':
          cod[i] = 2;
          break;
      case 'd': case 'e': case 'f':
          cod[i] = 3;
          break;
      case 'g': case 'h': case 'i': 
          cod[i] = 4;
          break;
      case 'j': case 'k': case 'l':
          cod[i] = 5;
          break;
      case 'm': case 'n': case 'o':
          cod[i] = 6;
          break;
      case 'p': case 'q': case 'r': case 's':
          cod[i] = 7;
          break;
      case 't': case 'u': case 'v':
        cod[i] = 8;
        break;
      case 'w': case 'x': case 'y': case 'z':
        cod[i] = 9;
         break;
    }
  }
  return cod;
}

//Alocar memória para cada componente da entrada
entry_t *ht_pair(const char *word, char* wordnorm, int shift) {
  // allocate the entry
  entry_t *entry = malloc(sizeof(entry_t) * 1);

  entry->size = strlen(wordnorm);
  entry->cod = t9(wordnorm);
  entry->occ = 1;
  entry->shift = shift;

  // copy the word and cod in place
  strcpy(entry->word, word);
  return entry;
}

// Inserir quando o hashcode da tabela está ocupado
void insert_shift(entry_t *entry, int slot, char* word, char* wordnorm, int shift) {
    entry_t* prev = entry;
    int occtemp = prev->occ;
  
    //Se for a primeira
    if(strcmp(entry->word,word) == 0) {
      entry->occ++;
      return;
    }

    entry = entry->next;

    //Verificar se exsite a palavra
    while(entry != NULL) {
      // Comparar com as restantes palavras da linked list, e ver se já existe
      // Se não existir inserir no fim
      // Se existir, mudar a posição para manter a lista ordenada, se necessário
    
      //Existe, incrementar e mudar mudar posição se necessário 
      if(strcmp(entry->word,word) == 0) {
        entry->occ++;
        //Se as ocorrências da palavra anterior forem maior, trocar a posição
        if(entry->occ > occtemp) {
          swap(entry->word,prev->word);
          int temp = entry->occ;
          entry->occ = prev->occ;
          prev->occ= temp;
        }
        return;
      }
      //Percorrer a lista
      prev = entry;
      entry = prev->next;
    }
    //Não existe inserir no fim
    prev->next = ht_pair(word,wordnorm,shift);
  }

//Inserir um valor
void ht_set(ht_t *hashtable, char* word) {

  char* wordnorm = malloc(sizeof(char)*strlen(word));
  strcpy(wordnorm,word);
  normalize(wordnorm);
  unsigned int slot = hash(t9(wordnorm),strlen(wordnorm));

  //Verificar se já não existe
  entry_t *entry = hashtable -> entries[slot];

  //Não existe inserir
  if(entry == NULL) {
    hashtable->entries[slot] = ht_pair(word,wordnorm,0);
    hashtable->entries[slot]->occ = 1;
    return;
  }
  // Se a slot é correcta então shift é 0
  //Ver se a slot é a correcta
  if(entry->shift == 0 && codcmp(entry->cod,t9(wordnorm),entry->size,strlen(wordnorm)) == 0) {
    insert_shift(entry, slot, word, wordnorm, 0);
    return;
  }
  else{
    int shift = 0, tempslot = slot+1;
    while(entry != NULL) {
      shift++;
      if(entry->shift == shift && codcmp(entry->cod,t9(wordnorm),entry->size,strlen(wordnorm)) == 0) {
        insert_shift(entry, slot, word, wordnorm, shift);
        return;
      }
      entry = hashtable -> entries[tempslot++];
    }
    if(entry == NULL) {
      hashtable->entries[slot] = ht_pair(word,wordnorm,0);
      hashtable->entries[slot]->occ = 1;
      return;
    }
  }
}

//Imprimir a hastable
void ht_print(ht_t *hashtable) {
  FILE *fp = fopen("dict.txt","w");

  for (int i = 0; i < tablesize; ++i) {
    entry_t *entry = hashtable->entries[i];
    if (entry == NULL) {
      continue;
    }
    else {
      //printf("slot[%d]: ",i);
      fprintf(fp,"%d-",i);
      fflush(stdout);
      
      //printf("{");
      for(int i = 0; i<entry->size; i++) {
        fprintf(fp,"%d",entry->cod[i]);
        //printf("%d",entry->cod[i]);
      }
      fprintf(fp,".%ld-",entry->size);
      //printf("} -> ");
      for(;;){
        if(entry->next == NULL){
          fprintf(fp,"%s.%d$",entry->word,entry->occ);
          //printf("%s (occ: %d)\n",entry->word,entry->occ);
          break;
        }
        else {
          fprintf(fp,"%s.%d-",entry->word,entry->occ);
          //printf("%s (occ: %d) -> ",entry->word,entry->occ);
          //fflush(stdout);
          entry = entry->next;
         }
      }
    }
  }
  fclose(fp);
}

//Contar as palavras
void count(const char* fname) {
  FILE *fp = fopen(fname,"r");
  int i = 0;
  tablesize= 0;
  char* word = malloc(sizeof(char)*MAX_LENGHT);
  char c;
  while((c = fgetc(fp)) != EOF) {
    if((c >= 65 && c <= 122) || (c >= -128 && c <= -70) || c == -61) {
      if((c>=65 && c<= 90) || ( c>= -128 && c <= -102)) c+=32;
      word[i] = c;
      i++;
    }
    else {
      if(i>0) {
        i = 0;
        tablesize++;
      }
      memset(word, 0, MAX_LENGHT);
    }
  }
  tablesize += (tablesize*(ROOM));
  free(word);
  fclose(fp);  
}

//Ler e acrescentar as palavras
void read(FILE *fp,ht_t *ht){
  int i = 0;
  char c;
  char* word = malloc(sizeof(char)*MAX_LENGHT);
  do {
    c = fgetc(fp);
    if((c >= 65 && c <= 122) || (c >= -128 && c <= -70) || c == -61) {
      if((c>=65 && c<= 90) || ( c>= -128 && c <= -102)) c+=32;
      word[i] = c;
      i++;
    }
    else {
      i = 0;
      if(strlen(word) > 0) {
        //printf("%s\n",word);
        ht_set(ht,word);
      }
      memset(word, 0, MAX_LENGHT);
    }
  } while (c != EOF);
  free(word);
}

//Adiciona uma nova palavra ao dicionário
void addNewWord(ht_t *ht) {
  printf("+-----------------------+\n");
  printf("|   1   |   2   |   3   |\n");
  printf("|       | a b c | d e f |\n");
  printf("+-----------------------+\n");
  printf("|   4   |   5   |   6   |\n");
  printf("| g h i | j k l | m n o |\n");
  printf("+-----------------------+\n");
  printf("|   7   |   8   |   9   |\n");
  printf("|p q r s| t u v |w x y z|\n");
  printf("+-----------------------+\n");
  printf("|       |   0   |     ^ |\n");
  printf("|  *  + |  ___  | #   | |\n");
  printf("+-----------------------+\n");
  
  char phone[8][4] = {
  {'a','b','c','.'},
  {'d','e','f','.'},
  {'g','h','i','.'},
  {'j','k','l','.'},
  {'m','n','o','.'},
  {'p','q','r','s'},
  {'t','u','v','.'},
  {'w','x','y','z'}
  };
  int count = 0,k = 0;
  char c = getchar();
  fflush(stdin);
  char* word = malloc(sizeof(char)*MAX_LENGHT);
  while(c != '\n') {
    char c1 = getchar();
    fflush(stdin);
    if(c != c1) {
      word[k] = phone[c-'0'-2][count];
      k++;
      count = 0;
      c = c1;
    }
    else count++;
  }
  ht_set(ht,word);
  printf("Adicionado '%s' ao dicionário, enter para continuar\n",word);
  getchar();
  fflush(stdin);
}

//Escrever usando o t9
void type(ht_t *ht) {
  int marker = 0;
  char* string = malloc(sizeof(char)*MAX_LENGHT);
  while(1) {
    printf("+----------------------+\n");
    printf("|  t9 predictive test  |\n");
    printf("| Introduza os numeros |\n");
    printf("+----------------------+\n");
    printf("|   's' para sair      |\n");
    printf("+----------------------+\n");
    printf("|  'Enter' termina o   |\n");
    printf("|   input da palavra   |\n");
    printf("+----------------------+\n");
    printf("|   '1' para ver as    |\n");
    printf("| diferentes sugestões |\n");
    printf("+----------------------+\n");
    printf("|  'e' para escolher   |\n");
    printf("|      a palavra       |\n");
    printf("+----------------------+\n");

    size_t size;
    char* line = NULL;
    size = getline(&line,&size,stdin);
    int *cod = malloc(sizeof(int)*size);
    for(int i = 0; i<size-1; i++) {
      cod[i] = line[i]-'0';
    }
    int slot = getSlot(ht,hash(cod,size-1),cod,size-1);
    if(slot == -1) {
      printf("Não existe palavra com essa configuração\n");
      continue;
    }
    char* word = circle_words(ht,slot);
    
    if(word == NULL) break;
    for(int i = 0; i<size; i++) {
      string[marker] = word[i];
      marker++;
    }
    string[marker+1] = ' ';
    marker++;
    free(line);

    printf("Se não quiser escrever mais escreva s, caso contrário carregue enter\n");
    char c = getchar();
    if(c == 's'){
      fflush(stdin);
      printf("Mensagem: %s\n",string);
      free(string);
      c = fgetc(stdin);
      return;
    }
  }
}

//Correr o programa
void run(ht_t *ht) {
  int mode = 0;

  while(1) {
    printf("Pretende escrever ou inserir uma nova palavra no dicionário ?\n");
    printf("1. Inserir uma nova palavra\n");
    printf("2. Escrever\n");
    printf("3. Sair\n");
    char* line = NULL;
    size_t size = getline(&line,&size,stdin);
    mode = atoi(line);
    if(mode == 1){
      addNewWord(ht);
      continue;
    }
    else if(mode == 2) {
      type(ht);
      continue;
    }
    else if(mode == 3){
      printf("bye!\n");
      return;
    }
  }
}

//Main
int main(int argc, char const *argv[]) {
  count(argv[1]);
  tablesize+=26;
  ht_t* ht = create();
  FILE* fp = fopen(argv[1],"r");

  read(fp,ht);

  run(ht);

  free(ht);
  fclose(fp);

  return 0;
}