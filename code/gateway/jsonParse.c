//test.c
#include <stdio.h>
#include <string.h>
#define MCCount 51

void split(char *lineConst, char status[MCCount][20], char lot[MCCount][20]){
  char line[25600];  // where we will put a copy of the input
  char *subString; // the "result"
  //char *final;
  strcpy(line, lineConst);
  int b = 0, next = 1;
  printf("original line is: [%s]\n",lineConst);
  do{
  subString = strtok(line, "\""); // find the first double quote
  lineConst+=strlen(subString)+1;
  //printf("Rest of the string is: [%s]\n",subString);
  subString=strtok(NULL, "\"");   // find the second double quote
   
   //may have to comment out botom two lines
   //strcpy(line,subString);
    //subString = strtok(NULL,"\"");
  
  if(subString == NULL){
    printf("DONE\n");
    break;
  }
  int len;

  //printf("Key/Value: [%s]\n",subString);
  len = strlen(subString);
  if(strcmp(subString,"status") !=0 && strcmp(subString,"ok") != 0){
    
    //strcpy(final,subString);
    //printf("Len is   %i",len);
    if(next%2 == 0){
      strncpy(status[b],subString,len);
      //printf("Value - status: [%s]\n",status[b]);

      ++b; 
      
    }else{
       strncpy(lot[b],subString,len);
      // printf("Key - Lot: [%s]\n",lot[b]);
      // lot[b][len]=0;
        //++b;
    }
    ++next;   
    //memset(final,0,strlen(final));
  }
 // lineConst+=strlen(subString);
 lineConst+=len+1; //plus 1 to bypass quote
 strcpy(line, lineConst);
 //printf("SECOND TIME: [%s]\n",lot[b]);
  //printf("new line is: [%s]\n",line);
  
}while(subString != NULL);
printf("-----------------------------------------\n");
int ls;
for(ls = 0; ls<tMAX;++ls){
  if(strlen(status[ls]) == 0){
       break;
  }
    printf("   statusF[%i] =[%s]",ls,status[ls]);
    printf("   lot[%i] =[%s]\n",ls,lot[ls]);
  }
  printf("-------END------------------------\n");

}


// int main(void) {
//   char* lineConst = "{\"status\": \"ok\", \"NorthRemote:1\": \"busy\", \"EastRemote:1\": \"closed\"}"; // the "input string"
//   char lot[tMAX][tMAX];
//   char status[tMAX][tMAX];
//   split(lineConst,status,lot);
//   printf("Printing lot: [%s] \n",lot[0]);
//   return 0;
// }