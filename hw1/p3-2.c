#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 
int main(){
       int i = 0;
       for (i = 0; i < 2; i++)
               if(fork() == 0){
                       fork();
                       printf("1");
                       printf("1");
               }
               else{
                       fork();
                       printf("2");
               }
       return 0;
}
