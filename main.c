#include "bitmaprender.h"



void clear_buff(){
    while(getchar()!='\n');
}

int main(void){

    int error=0;
    struct image *img;
    struct render_data_pack *data;
    struct multi_thread_pixels *threading_pack=malloc(sizeof(struct multi_thread_pixels));
    char *filename=malloc(101);
    if(!threading_pack||!filename){
        printf("Can't assign memory for program");
        return 1;
    }
    threading_pack->slots_per_province=1000;
    threading_pack->number_of_threads=8;
    threading_pack->positions=NULL;
    threading_pack->colors=NULL;
    threading_pack->pack=NULL;
    threading_pack->processed_pixels=NULL;
    threading_pack->img=NULL;
    threading_pack->freeID=-1;
    threading_pack->finished=0;

    while(1) {
        system("cls");
        printf("BMP render to Unity for CoN version 2.0\n");
        printf("Settings\nNumber of Threads:%d\nMax Provinces per thread:%d\nMax amount of provinces:%d\n"
               , threading_pack->number_of_threads,threading_pack->slots_per_province,threading_pack->number_of_threads*threading_pack->slots_per_province);
        printf("type 'settings' to change settings of rendering or type name of the file 'name.bmp'\n>>>");
        scanf("%100[^\n]",filename);
        clear_buff();
        if(strcmp(filename,"settings")==0){
            system("cls");
            printf("Number of Threads:");
            scanf("%d",&(threading_pack->number_of_threads));
            clear_buff();
            printf("Max number of Provinces per thread:");
            scanf("%d",&(threading_pack->slots_per_province));
            clear_buff();
        }
        else {
            img= loadBmp(filename,&error);
            if(error){
                if(error==2){
                    printf("Can't open a file, check if name of the file is right, or if it is in the same folder as .exe program");
                    system("pause");
                    error=0;
                }
                else {
                    printf("Error");
                    return 0;
                }
            }
            else break;
        }
    }



    data=makeRenderData(img,&error,threading_pack);

    if(error){
        printf("Error");
        return 0;
    }

    saveRenderData(&data,"result.bin",&error);


    freeRenderDataPack(&data);
    freeImage(&img);
    free(filename);

    system("pause");

    return 0;

    struct stack *stk=NULL;
    int n;

    for(int i=0;i<200;i++) {
        n=i;
        addToStack(&stk, &n);
    }

    for(int i=0;i<200;i++) {
        removeFromStack(&stk, &n);
        printf("%d\n",n);
    }
}