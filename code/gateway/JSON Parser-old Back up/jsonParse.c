//test.c
#include <stdio.h>
#include <string.h>
//#include "list.c"
#define tMAX 10

void split(char *lineConst, char status[tMAX][tMAX], char lot[tMAX][tMAX]){
   char line[256];  // where we will put a copy of the input
  char *subString; // the "result"

  strcpy(line, lineConst);
  int c = 1;
  do{
  subString = strtok(line, "\""); // find the first double quote
  lineConst+=strlen(subString)+1;
  //printf("Rest of the string is: [%s]\n",subString);
  subString=strtok(NULL, "\"");   // find the second double quote
   //strcpy(line,subString);
  //subString = strtok(NULL,"\"");
  
  if(subString == NULL){
    printf("DONE\n");
    break;
  }
  //printf("Key/Value: [%s]\n",subString);
  if(strcmp(subString,"status") !=0 && strcmp(subString,"ok") != 0){
    if(c%2 == 0){
      strcpy(status[c-1],subString);
      printf("Value - status: [%s]\n",status[c-1]);
    }else{
       strcpy(lot[c-1],subString);
       printf("Key - Lot: [%s]\n",lot[c-1]);
    }
    ++c;
  }
 // lineConst+=strlen(subString);
 lineConst+=strlen(subString)+1; //plus 1 to bypass quote
 strcpy(line, lineConst);

  //printf("new line is: [%s]\n",line);
  
}while(subString != NULL);
}


int main(void) {
  char* lineConst = "{\"status\": \"ok\", \"NorthRemote:1\": \"busy\", \"EastRemote:1\": \"closed\"}"; // the "input string"
  char lot[tMAX][tMAX];
  char status[tMAX][tMAX];
  split(lineConst,status,lot);
  printf("Printing lot: [%s] \n",lot[0]);
  return 0;
}