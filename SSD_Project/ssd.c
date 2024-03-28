#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init(){
    FILE *file = fopen("./nand.txt", "w");
    for (int i = 0; i < 100; i++) {
            fprintf(file, "00000000");
        }
    fclose(file);
}

void read(int idx){
    FILE *file = fopen("./nand.txt", "r+");
    FILE *file2 = fopen("./result.txt", "w");
    char tmp[9];
    fseek(file, (idx*8), SEEK_SET); 
    fgets(tmp,9,file);
    fprintf(file2,"0x%s", tmp);
    fclose(file);
    fclose(file2);
}

void fullread(){
    FILE *file = fopen("./nand.txt", "r+");
    FILE *file2 = fopen("./result.txt", "w");
    char tmp[9];
    while(fgets(tmp,9,file)!=NULL){
        fprintf(file2,"0x%s\n", tmp);
    }
    fclose(file);
    fclose(file2);
}

void write(int idx, char *txt){
    FILE *file = fopen("./nand.txt", "r+");
    fseek(file, (idx*8), SEEK_SET); 
    fprintf(file,"%s", txt);
    fclose(file);
}

void fullwrite(char *txt){
    FILE *file = fopen("./nand.txt", "w");
    for (int i = 0; i < 100; i++) {
            fprintf(file,"%s", txt);
        }
    fclose(file);
}

int main(int argc, char *argv[]){
    int idx;
    char *txt;

    if(*argv[1]=='W'){
        idx =atoi(argv[2]);
        if(idx<0 ||idx>99){
            printf("OUT OF INDEX\n");
            return 0;
        } 
        txt=argv[3];
        if(strlen(txt)!=10){
            printf("INVALID VALUE\n");
            return 0;
        }
        if(strncmp(txt,"0x",2)){
            printf("INVALID VALUE\n");
            return 0;
        }
        for(int i=2; i<10; i++){
            if((argv[3][i]<'0' || argv[3][i]>'9')&&(argv[3][i]<'A' || argv[3][i]>'F')){
                printf("INVALID VALUE\n");
                return 0;
            }
        }

        strcpy(txt, txt + 2);
        write(idx,txt);
    }

    else if(!strcmp(argv[1],"FW")){
        txt=argv[2];
        if(strlen(txt)!=10){
            printf("INVALID VALUE\n");
            return 0;
        }
        if(strncmp(txt,"0x",2)){
            printf("INVALID VALUE\n");
            return 0;
        }
        for(int i=2; i<10; i++){
            if((argv[3][i]<'0' || argv[3][i]>'9')&&(argv[3][i]<'A' || argv[3][i]>'F')){
                printf("INVALID VALUE\n");
                return 0;
            }
        }
        strcpy(txt, txt + 2);
        fullwrite(txt);
    }

    else if(*argv[1]=='R'){
        idx =atoi(argv[2]);
        if(idx<0 ||idx>99){            
            printf("OUT OF INDEX\n");
            return 0;
        }
        read(idx);
    }

    else if(!strcmp(argv[1],"FR")){
        fullread();
    }
    else{
        printf("INVALID COMMAND\n");
    }
}