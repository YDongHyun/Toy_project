#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char input_[30];
char txt[11];
int help_color[7] ={2,4,7,11,13,15,20};

int main(){
    int idx;
    char tmp[1000];
    char result[1000];
    while(1){
        printf("SSD Shell : ");
        scanf("%s",input_);

        if(!strcmp(input_,"read")){
            scanf("%d",&idx);
            sprintf(result, "./ssd R %d", idx);
            system(result);
            FILE *file = fopen("./result.txt", "r");
            fgets(tmp,11,file);
            printf("%s\n",tmp);
            fclose(file);
        }

        else if(!strcmp(input_,"fullread")){
            system("./ssd FR");
            FILE *file = fopen("./result.txt", "r");
            while(fgets(tmp,11,file)!=NULL){
                printf("%s",tmp);
            }
            fclose(file);
        }

        else if(!strcmp(input_,"write")){
            scanf("%d %s",&idx, txt);
            sprintf(result, "./ssd W %d %s", idx,txt);
            system(result);
        }

        else if(!strcmp(input_,"fullwrite")){
            scanf("%s",txt);
            sprintf(result, "./ssd FW %s",txt);
            system(result);
        }

        else if (!strcmp(input_,"exit")){
            break;
        }

        else if(!strcmp(input_, "help")){
            FILE *file = fopen("./help.txt", "r");
            
            int count=1;
            int flag=0;
            while(fgets(tmp,1000,file)!=NULL){
                for (int i =0; i<7;i++){
                    if(help_color[i]==count){
                        printf("\x1b[31m%s\x1b[0m",tmp);
                        flag=1;
                        break;
                    }
                }
                if(flag==0){
                    printf("%s",tmp);
                }
                else{
                    flag=0;
                }
                count++;
            }
            fclose(file);
        }
        else {
            printf("INVALID COMMAND\n");
        }
    }
}