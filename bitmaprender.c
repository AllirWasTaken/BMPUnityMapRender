#pragma pack(1)

#include "bitmaprender.h"

#define ERR(typ){ int errorcode = typ; if(error_code)*error_code = errorcode;if( typ >2)fclose(f);printf("Error occurred in function %s, error code is %d \n",__func__, errorcode );return NULL;}
#define ER(typ){ int errorcode = typ; if(error_code)*error_code = errorcode;printf("Error occurred in function %s, error code is %d \n",__func__, errorcode );return NULL;}

struct image *loadBmp(char* filename, int* error_code){

    if(error_code){
        if(*error_code)return NULL;
    }


    FILE *f=NULL;
    if(!filename)ERR(1);

    f=fopen(filename,"rb");
    if(!f)ERR(2);

    struct header *temp_header=malloc(sizeof(struct header));
    if(!temp_header){
        ERR(4);
    }

    if(!fread(temp_header,sizeof(struct header),1,f)){
        free(temp_header);
        ERR(3);
    };

    if(temp_header->signature!=0x4D42){
        free(temp_header);
        ERR(3);
    }


    if(temp_header->image_size_bytes<1){
        temp_header->image_size_bytes=temp_header->width*temp_header->height*temp_header->bits_per_pixel/8;
    }

    struct pixel *data=NULL;
    data=malloc(temp_header->width*temp_header->height/8*temp_header->bits_per_pixel);
    if(!data){
        free(temp_header);
        ERR(4);
    }

    if (!fread(data, temp_header->width*temp_header->height/8*temp_header->bits_per_pixel, 1, f)) {
        free(temp_header);
        free(data);
        ERR(3);
    }


    struct image *value=malloc(sizeof(struct image));
    if(!value){
        free(temp_header);
        free(data);
        ERR(4);
    }

    value->data=data;
    value->header=temp_header;
    fclose(f);

    return value;

}
void saveBmp(struct image *img, char* filename,int *error_code){
    if(!img||!filename){
        *error_code=1;
        return;
    }
    FILE *f=fopen(filename,"wb");
    if(!f){
        *error_code=2;
        return;
    }
    if(!fwrite(img->header,sizeof(struct header),1,f)){
        *error_code=3;
    }
    fwrite(img->data,img->header->image_size_bytes,1,f);
    fclose(f);
    return;
}


void freeImage(struct image **img){
    if(!img)return;
    if(!*img)return;
    if((*img)->header==NULL);
    else{
        free((*img)->header);
    }
    if((*img)->data==NULL);
    else{
        free((*img)->data);
    }
    free(*img);
    *img=NULL;
    return;
}


int addToStack(struct stack **stk,int *number){
    if(!stk||!number)return 1;
    if(*stk==NULL){
        *stk=malloc(sizeof(struct stack));
        if(!*stk){
            return 2;
        }
        (*stk)->size_of_stack=1;
        (*stk)->stack_data=malloc(sizeof(int));
        if((*stk)->stack_data==NULL){
            free(*stk);
            return 2;
        }
        *(*stk)->stack_data=*number;
        return 0;
    }
    else{
        int *temp=NULL;
        temp= realloc((*stk)->stack_data,((*stk)->size_of_stack+1)*sizeof(int));
        if(temp!=NULL){
            (*stk)->stack_data=temp;
            *((*stk)->stack_data+((*stk)->size_of_stack))=*number;
            (*stk)->size_of_stack++;
            return 0;
        }
        else{
            return 2;
        }
    }
}


int removeFromStack(struct stack **stk,int *number){
    if(!stk||!*stk||!number)return 1;

    if((*stk)->size_of_stack==1){
        *number=*(*stk)->stack_data;
        free((*stk)->stack_data);
        free(*stk);
        *stk=NULL;
        return 0;
    }
    else{
        int *temp=NULL;
        int n=*((*stk)->stack_data+(*stk)->size_of_stack-1);
        temp=realloc((*stk)->stack_data,((*stk)->size_of_stack-1)*sizeof(int));
        if(!temp)return 2;
        else{
            *number=n;
            (*stk)->stack_data=temp;
            (*stk)->size_of_stack--;
            return 0;
        }
    }
}



void freeStack(struct stack **stk){
    if(*stk==NULL)return;
    free((*stk)->stack_data);
    free(*stk);
    *stk=NULL;
    return;
}


int checkIfPixelsAreSame(struct pixel *pix1,struct pixel *pix2){
    if(!pix1||!pix2)return 0;
    if(pix1->blue==pix2->blue)
        if(pix1->green==pix2->green)
            if(pix1->red==pix2->red)
                return 1;
    return 0;
}


struct province *makeProvinceData(struct image *img,int *error_code,int position,int *amount_of_processed_pixels){
    if(!img||position<0||position>img->header->image_size_bytes*8/img->header->bits_per_pixel||!amount_of_processed_pixels)ER(1);
    if(((img->data+position)->green==0)&&((img->data+position)->red==0)&&((img->data+position)->blue==0))ER(3);



    struct stack *stk=NULL,*result_stk=NULL;

    if(addToStack(&stk,&position)==2||addToStack(&result_stk,&position)==2){
        freeStack(&stk);
        freeStack(&result_stk);
        ER(2);
    }

    struct pixel pixel_colors;
    pixel_colors.blue=(img->data+position)->blue;
    pixel_colors.red=(img->data+position)->red;
    pixel_colors.green=(img->data+position)->green;

    (img->data+position)->blue=0;
    (img->data+position)->red=0;
    (img->data+position)->green=0;



    int current_position;



    while(stk!=NULL){

        if(removeFromStack(&stk,&current_position)==2){
            freeStack(&stk);
            freeStack(&result_stk);
            ER(2);
        }
        if(scanPixelsAroundPosition(img,current_position,&stk,&result_stk,pixel_colors)){
            freeStack(&stk);
            freeStack(&result_stk);
            ER(5);
        }

    }

    struct province *result=NULL;
    struct rectangle *result_data=NULL;
    result=malloc(sizeof(struct province));
    if(!result){
        freeStack(&result_stk);
        ER(2);
    }

    result->amount_of_rectangles=result_stk->size_of_stack;

    *amount_of_processed_pixels+=result_stk->size_of_stack;

    result_data= translateToRectangles(&result_stk,img);
    if(!result_data){
        freeStack(&result_stk);
        free(result);
        *amount_of_processed_pixels=0;
        ER(2);
    }
    else{
        result->rectangles=result_data;
        if(error_code)*error_code=0;
        return result;
    }


}


void turnPixelBlack(struct image *img, int position){
    (img->data+position)->green=0;
    (img->data+position)->red=0;
    (img->data+position)->blue=0;

}


int scanPixelsAroundPosition(struct image *img,int position,struct stack **stk1,struct stack **stk2,struct pixel colors){
    if(!img)return 1;

    int width=img->header->width;
    int max_position=img->header->width*img->header->height-1;

    int new_position;

    if(position>max_position||position<0)return 2;

    //up
    if(position+width<=max_position){
        if(checkIfPixelsAreSame(&colors,img->data+position+width)) {
            new_position=position+width;
            turnPixelBlack(img,new_position);
            if (addToStack(stk1, &new_position))return 3;
            if (addToStack(stk2, &new_position))return 3;
        }
    }
    //down
    if(position-width>-1){
        if(checkIfPixelsAreSame(&colors,img->data+position-width)) {
            new_position=position-width;
            turnPixelBlack(img,new_position);
            if (addToStack(stk1, &new_position))return 3;
            if (addToStack(stk2, &new_position))return 3;
        }
    }
    //right
    if((position+1)%width!=0&&(position+1)<=max_position){
        if(checkIfPixelsAreSame(&colors,img->data+position+1)) {
            new_position=position+1;
            turnPixelBlack(img,new_position);
            if (addToStack(stk1, &new_position))return 3;
            if (addToStack(stk2, &new_position))return 3;
        }
    }
    //left
    if(position%width!=0&&position>0){
        if(checkIfPixelsAreSame(&colors,img->data+position-1)) {
            new_position=position-1;
            turnPixelBlack(img,new_position);
            if (addToStack(stk1, &new_position))return 3;
            if (addToStack(stk2, &new_position))return 3;
        }
    }

    return 0;

}


struct rectangle *translateToRectangles(struct stack **stk, struct image *img){
    if(!stk||!img)return NULL;
    struct rectangle *result=malloc(sizeof(struct rectangle)*(*stk)->size_of_stack);
    if(!result)return NULL;


    int width=img->header->width;
    int position;


    for(int i=0;*stk;i++) {
        removeFromStack(stk,&position);
        if(position<0){
            free(result);
            return NULL;
        }
        (result+i)->x1 = position % width;
        (result+i)->y1 = position / width;
    }

    return result;

}


void freeRenderDataPack(struct render_data_pack **data){
    if(!data||!*data)return;

    for(int i=0;i<(*data)->number_of_data_slots;i++){
        for(int j=0;j<((*data)->data+i)->amount_of_provinces;j++){
            free((((*data)->data+i)->optimized_provinces+j)->vertices);
            free((((*data)->data+i)->optimized_provinces+j)->triangles);
        }
        free(((*data)->data+i)->optimized_provinces);
    }
    free((*data)->data);

    free(*data);
    *data=NULL;
}


void freeThreading(struct multi_thread_pixels **threads){
    if(!threads||!*threads)return;

    if((*threads)->positions)free((*threads)->positions);
    if((*threads)->colors)free((*threads)->colors);
    if((*threads)->processed_pixels)free((*threads)->processed_pixels);

    if((*threads)->pack)freeRenderDataPack(&((*threads)->pack));

    free(*threads);
    *threads=NULL;
}


struct render_data_pack *makeRenderData(struct image *img,int *error_code, struct multi_thread_pixels *threading){
    if(!img||!threading){
        freeThreading(&threading);
        ER(1);
    }

    threading->img=img;

    threading->colors=malloc(sizeof(struct pixel)*threading->number_of_threads);
    if(!threading->colors){
        freeThreading(&threading);
        ER(2);
    }

    threading->positions=malloc(sizeof(int)*threading->number_of_threads);

    if(!threading->positions){
        freeThreading(&threading);
        ER(2);
    }

    threading->processed_pixels=malloc(sizeof(int)*threading->number_of_threads);

    if(!threading->processed_pixels){
        freeThreading(&threading);
        ER(2);
    }

    for(int i=0;i<threading->number_of_threads;i++)*(threading->processed_pixels+i)=0;

    threading->pack=malloc(sizeof(struct render_data_pack));

    if(!threading->pack){
        freeThreading(&threading);
        ER(3);
    }

    threading->pack->number_of_data_slots=threading->number_of_threads;

    threading->pack->data=malloc(sizeof(struct render_data)*threading->number_of_threads);

    if(!threading->pack->data){
        freeThreading(&threading);
        ER(4);
    }

    for(int i=0;i<threading->number_of_threads;i++){
        (threading->pack->data+i)->optimized_provinces=malloc(sizeof(struct optimized_province)*threading->slots_per_province);
        if(!(threading->pack->data+i)->optimized_provinces){
            freeThreading(&threading);
            ER(4);
        }
    }
    for(int i=0;i<threading->number_of_threads;i++){
        (threading->pack->data+i)->amount_of_provinces=0;

    }

    HANDLE *thread_IDs;
    thread_IDs=malloc(sizeof(HANDLE)*threading->number_of_threads);
    if(!thread_IDs){
        freeThreading(&threading);
        ER(5);
    }

    //allocation is done, so it is time to create our threads

    //Be lost who crosses that line, no errors are even tried to be captured/handled below



    for(int i=0;i<threading->number_of_threads;i++)*(threading->positions+i)=-2;

    HANDLE stat_thread;

    stat_thread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)&showStats,threading,0,0);


    threading->freeID=-1;


    //starting procedure

    for(int i=0;i<threading->number_of_threads;i++){
       *(thread_IDs+i)=CreateThread(0,0,(LPTHREAD_START_ROUTINE)&singleThreadProvinceRender,threading,0,0);
        threading->freeID=i;
        while(threading->freeID!=-1)Sleep(10);
    }


    findPixelsToProcess(threading);  //start the bullshit

    struct render_data_pack *result=threading->pack;
    threading->pack=NULL;

    for(int i=0;i<threading->number_of_threads;i++){
        CloseHandle(*(thread_IDs+i));

    }
    CloseHandle(stat_thread);
    freeThreading(&threading);

    return result;
}


void *showStats(struct multi_thread_pixels *threading){
    int max_pixels=threading->img->header->image_size_bytes*8/threading->img->header->bits_per_pixel;
    int all_pixels;
    float percentage;
    int processing_speed;
    int time_left;
    clock_t start,measure;
    double timing;
    start=clock();

    while(threading->finished!=1){
        //internal calculations
        all_pixels=0;
        for(int i=0;i<threading->number_of_threads;i++)all_pixels+=*(threading->processed_pixels+i);
        percentage=(float)all_pixels/(float)max_pixels*100;
        measure=clock();
        timing=(double)(measure-start)/CLOCKS_PER_SEC;
        processing_speed=all_pixels/timing;
        if(processing_speed==0)time_left=0;
        else time_left=(max_pixels-all_pixels)/processing_speed;


        //gui
        system("cls");
        printf("Processing speed: %10d p/s, ETA time:%4d seconds,Progress: %12d/%d p    %.2f%\n"
               ,processing_speed,time_left,all_pixels,max_pixels,percentage);

        for(int i=0;i<threading->number_of_threads;i++) {
            printf("Thread %2d ",i);
            if(*(threading->positions+i)>-1)printf("Works  ");
            else if (*(threading->positions+i)==-1)printf("Sleeps ");
            else if (*(threading->positions+i)==-2)printf("Starts ");
            else if (*(threading->positions+i)==-3)printf("Crashed ");
            printf(", px: %12d, pv: %4d/%d \n",
                   *(threading->processed_pixels + i), (threading->pack->data + i)->amount_of_provinces,
                   threading->slots_per_province);
        }

        if(all_pixels==max_pixels)threading->finished=1;
        Sleep(500);
    }
    ExitThread(0);
}



void findPixelsToProcess(struct multi_thread_pixels *threading){

    int max_range=threading->img->header->width*threading->img->header->height;
    int max_threads=threading->number_of_threads;
    int current_position=0;
    struct pixel *data=threading->img->data;
    struct pixel black;
    black.green=0;
    black.red=0;
    black.blue=0;
    int ok=0;

    while(threading->finished==0){
        for (int i = 0; i <max_range ;i++) {

            if(checkIfPixelsAreSame(&black,data+i)==0){
                ok=1;
                for(int x=0;x<threading->number_of_threads;x++){
                    if(checkIfPixelsAreSame(data+i,threading->colors+x)){
                        ok=0;
                        break;
                    }
                }
                if(ok){
                    ok=0;
                    while(*(threading->positions+current_position)!=-1||*(threading->positions+current_position)==-3||(threading->pack->data+current_position)->amount_of_provinces==threading->slots_per_province){
                        current_position++;
                        if(current_position==max_threads)current_position=0;
                    }
                    if(!checkIfPixelsAreSame(&black,data+i)) {
                        (threading->colors + current_position)->green = (data + i)->green;
                        (threading->colors + current_position)->blue = (data + i)->blue;
                        (threading->colors + current_position)->red = (data + i)->red;
                        *(threading->positions + current_position) = i;
                        current_position++;
                        if (current_position == max_threads)current_position = 0;
                    }
                }
            }

        }
    }
    Sleep(100);

}


int saveRenderData(struct render_data_pack **data,char* filename,int *error_code){
    if(!filename||!data||!*data)ER(1);

    FILE *f= fopen(filename,"wb");
    if(!f)ERR(2);

    int amount_prov=0;
    int ye=0;

    for(int i=0;i<(*data)->number_of_data_slots;i++){
        amount_prov+=((*data)->data+i)->amount_of_provinces;
    }

    printf("Starting saving\n");
    fwrite(&amount_prov,sizeof(int),1,f);


    for(int i=0;i<(*data)->number_of_data_slots;i++){
        for(int j=0;j<((*data)->data+i)->amount_of_provinces;j++){
            fwrite(&(((*data)->data+i)->optimized_provinces+j)->width,4,1,f);
            fwrite(&(((*data)->data+i)->optimized_provinces+j)->height,4,1,f);
            fwrite(&(((*data)->data+i)->optimized_provinces+j)->middleX,4,1,f);
            fwrite(&(((*data)->data+i)->optimized_provinces+j)->middleY,4,1,f);
            fwrite(&(((*data)->data+i)->optimized_provinces+j)->amount_of_vertices,4,1,f);
            fwrite(&(((*data)->data+i)->optimized_provinces+j)->amount_of_triangles,4,1,f);
            fwrite((((*data)->data+i)->optimized_provinces+j)->vertices,
                   (((*data)->data+i)->optimized_provinces+j)->amount_of_vertices*8,1,f);
            fwrite((((*data)->data+i)->optimized_provinces+j)->triangles,
                   (((*data)->data+i)->optimized_provinces+j)->amount_of_triangles*4,1,f);
            ye++;
        }
    }


    printf("\33[2K\r"),printf("Saving %5d/%d",ye,amount_prov);
    fclose(f);
    printf("\nSaving done!\n");
    return 0;
}


void *singleThreadProvinceRender(struct multi_thread_pixels *threads){

    int my_ID=-1;
    int error;

    while(my_ID==-1){
        my_ID=threads->freeID;
        threads->freeID=-1;
        *(threads->positions+my_ID)=-1;
        Sleep(10);
    }

    struct province *current_province;
    int my_pixels=0;
    struct optimized_province *current_optimized_province;
    struct render_data *my_data=threads->pack->data+my_ID;

    while(threads->finished==0){
        if(*(threads->positions+my_ID)!=-1){
            current_province= makeProvinceData(threads->img,&error,*(threads->positions+my_ID),&my_pixels);
            if(error){
                *(threads->positions+my_ID)=-3;
                ExitThread(0);
            }
            (threads->colors+my_ID)->green=0;
            (threads->colors+my_ID)->red=0;
            (threads->colors+my_ID)->blue=0;


            current_optimized_province= optimizeProvince(&current_province);

            free(current_province->rectangles);
            free(current_province);

            if(!current_optimized_province){
                *(threads->positions+my_ID)=-3;
                ExitThread(0);
            }

            (my_data->optimized_provinces+my_data->amount_of_provinces)->height=current_optimized_province->height;
            (my_data->optimized_provinces+my_data->amount_of_provinces)->width=current_optimized_province->width;
            (my_data->optimized_provinces+my_data->amount_of_provinces)->middleX=current_optimized_province->middleX;
            (my_data->optimized_provinces+my_data->amount_of_provinces)->middleY=current_optimized_province->middleY;
            (my_data->optimized_provinces+my_data->amount_of_provinces)->amount_of_vertices=current_optimized_province->amount_of_vertices;
            (my_data->optimized_provinces+my_data->amount_of_provinces)->amount_of_triangles=current_optimized_province->amount_of_triangles;
            (my_data->optimized_provinces+my_data->amount_of_provinces)->vertices=current_optimized_province->vertices;
            (my_data->optimized_provinces+my_data->amount_of_provinces)->triangles=current_optimized_province->triangles;
            free(current_optimized_province);
            my_data->amount_of_provinces++;


            *(threads->processed_pixels+my_ID)=my_pixels;
            *(threads->positions+my_ID)=-1;
        }
        else Sleep(10);
    }
    ExitThread(0);

}

struct optimized_province *optimizeProvince(struct province **prov){
    if(!prov||!*prov)return NULL;


    int *maxLH=malloc(sizeof(int)*3);
    if(!maxLH)return NULL;
    struct optimized_province *result=malloc(sizeof(struct optimized_province));
    if(!result)return NULL;
    if(calculateSizeVariables(*prov,result,maxLH)){
        free(result);
        free(maxLH);
        return NULL;
    }

    if(redrawProvinceAndCalculateVerticesTriangles(*prov,result,maxLH)){
        free(result);
        free(maxLH);
        return NULL;
    }
    free(maxLH);


    return result;

}


int calculateSizeVariables(struct province *prov,struct optimized_province *opt, int *maxLHP){
    if(!opt||!prov)return 1;
    int maxR,maxL,maxU,maxD,maxLH;

    maxR=prov->rectangles->x1;
    maxL=prov->rectangles->x1;
    maxU=prov->rectangles->y1;
    maxD=prov->rectangles->y1;
    maxLH=prov->rectangles->y1;


    for(int i = 0; i<prov->amount_of_rectangles;i++){
        if((prov->rectangles+i)->x1>maxR)maxR=(prov->rectangles+i)->x1;
        if((prov->rectangles+i)->x1<maxL){
            maxL=(prov->rectangles+i)->x1;
            maxLH=(prov->rectangles+i)->y1;
        }
        if((prov->rectangles+i)->y1>maxU)maxU=(prov->rectangles+i)->y1;
        if((prov->rectangles+i)->y1<maxD)maxD=(prov->rectangles+i)->y1;
    }

    opt->width=maxR-maxL+2;
    opt->height=maxU-maxD+2;

    opt->middleX=opt->width/2+maxL;
    opt->middleY=opt->height/2+maxD;

    *maxLHP=maxLH;
    *(maxLHP+1)=maxL;
    *(maxLHP+2)=maxU;
    return 0;
}


int **create2DArray(int width,int height){
    if(width<1||height<1)return NULL;

    int **result=malloc(sizeof(int *)*height);
    if(!result)return NULL;

    for(int i=0;i<height;i++){
        *(result+i)=malloc(sizeof(int)*width);
        if(!*(result+i)){
            for(int j=0;j<i;j++){
                free(*(result+j));
                free(result);
                return NULL;
            }
        }
    }

    return result;
}


void free2DArray(int ***array,int height){
    if(!array||!*array)return;

    if(*array) {
        for (int i = 0; i < height; i++) {
            if (*(*array + i))free(*(*array + i));
        }
        free(*array);
        *array = NULL;
    }
}


void showArray(int **array,int size_x,int size_y){
    for(int y=0;y<size_y;y++){
        for(int x=0;x<size_x;x++){
            if(*(*(array+y)+x))printf("%2d",*(*(array+y)+x));
            else printf("  ");

        }
        printf("\n");
    }
}


int checkValuesAround(int **array,int y,int x,int value,int height,int width){
    if(x+1<width&&x-1>=0&&y-1>=0&&y+1<height) {
        if (*(*(array + y) + x + 1) >= value&&*(*(array + y) + x - 1) >= value&&*(*(array + y+1) + x) >= value&&*(*(array + y-1) + x) >= value) {
            *(*(array + y) + x) = value + 1;
            return 1;
        }
    }
    return  0;
}


int increaseValuesAwayFromBorder(int **array,int height,int width){
    int work=1;
    int value=1;

    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){
            if(*(*(array+y)+x))*(*(array+y)+x)=1;
        }
    }

    while(work){
        work=0;
        for(int y=0;y<height;y++){
            for(int x=0;x<width;x++){
                if((*(*array+y)+x)!=0)if(checkValuesAround(array,y,x,value,height,width))work=1;
            }
        }
        value++;
    }
    return value;
}


int checkLine(int **array,int x,int y,int type,int length){
    if(type) {
        for (int i = 0; i < length; i++) {
            if(!checkPixel(array,x,y-i))return 0;
        }
    }
    else{
        for (int i = 0; i < length; i++) {
            if(!checkPixel(array,x+i,y))return 0;
        }
    }
    return 1;
}


int checkPixel(int **array,int x,int y){
    if(*(*(array+y)+x))return 1;
    return 0;
}


void clearSpaceInsideRectangle(int **array,int x,int y,int width,int height){
    y-=height;
    y++;
    for(int i=0;i<height;i++){
        for(int j=0;j<width;j++){
            *(*(array+y+i)+x+j)=0;
        }
    }
}


int *generateSingleVerticesPack(int **array,int x,int y, int max_x,int max_y){

    int *result=malloc(sizeof(int)*8);
    if(!result)return NULL;

    int width=1,height=1;

    int length;

    int r=1,l=1,d=1,u=1;

    int work=1;

    while(work){
        work=0;
        //right
        if(r) {
            length = height;
            if (x + width < max_x) {
                if (checkLine(array, x + width, y, 1, length)) {
                    width++;
                }
                else r=0;
            }
            else r=0;
        }
        //down
        if(d) {
            length = width;
            if (y + 1 < max_y) {
                if (checkLine(array, x , y + 1 , 0, length)) {
                    height++;
                    y++;
                }
                else d=0;
            }
            else d=0;
        }
        //left
        if(l){
            length=height;
            if(x-1>=0){
                if(checkLine(array,x-1,y,1,length)){
                    width++;
                    x--;
                }
                else l=0;
            }
            else l=0;
        }
        //up
        if(u){
            length=width;
            if(y-height>=0){
                if(checkLine(array,x,y-height,0,length)){
                    height++;
                }
                else u=0;
            }
            else u=0;
        }
        if(u||l||d||r)work=1;
    }

    int middle_x=max_x/2,middle_y=max_y/2;

    //ld
    *(result+0)=x-middle_x;
    *(result+1)=(max_y-y)-middle_y-1;
    //lu
    *(result+2)=x-middle_x;
    *(result+3)=(max_y-y)-middle_y+height-1;
    //ru
    *(result+4)=x+width-middle_x;
    *(result+5)=(max_y-y)-middle_y+height-1;
    //rd
    *(result+6)=x+width-middle_x;
    *(result+7)=(max_y-y)-middle_y-1;


    clearSpaceInsideRectangle(array,x,y,width,height);

    return result;
}


int findMaxValueInArray(int **array,int max_x,int max_y,int *return_x,int *return_y){
    int max=**array;
    int found=0;
    for(int y=0;y<max_y;y++){
        for(int x=0;x<max_x;x++){
            if(*(*(array+y)+x)>max){
                max=*(*(array+y)+x);
                *return_x=x;
                *return_y=y;
                found=1;
            }
        }
    }
    return found;
}


int redrawProvinceAndCalculateVerticesTriangles(struct province* prov, struct optimized_province *opt,int *max_left_height){
    if(!prov||!opt)return 1;

    opt->amount_of_triangles=0;
    opt->amount_of_vertices=0;


    int **array= create2DArray(opt->width,opt->height);
    if(!array)return 1;


    int max_left_x=*(max_left_height+1),m_left_y=*max_left_height,m_high=*(max_left_height+2);

    int relative_y=m_high+1,relative_x=max_left_x;

    for(int y=0;y<opt->height;y++){
        for(int x=0;x<opt->width;x++){
            *(*(array+y)+x)=0;
        }
    }


    for(int i=0,x,y;i<prov->amount_of_rectangles;i++){

        x=(prov->rectangles+i)->x1-relative_x;
        y=relative_y-(prov->rectangles+i)->y1;
        *(*(array+y)+x)=1;

    }


    increaseValuesAwayFromBorder(array, opt->height, opt->width);

    //showArray(array,opt->width,opt->height);


    int found_x,found_y;

    struct raw_vertices *ver=NULL;

    while(findMaxValueInArray(array,opt->width,opt->height,&found_x,&found_y)) {

        if(addVertices(&ver,generateSingleVerticesPack(array, found_x, found_y, opt->width, opt->height))){
            free(ver->data);
            free(ver);
            free2DArray(&array,opt->height);
            return 1;
        }

        increaseValuesAwayFromBorder(array, opt->height, opt->width);

    }
    free2DArray(&array,opt->height);

    int *temp=realloc(ver->data,sizeof(int)*ver->ver);
    if(!temp){
        free(ver->data);
        free(ver);
        return 1;
    }
    else{
        opt->vertices=temp;
        opt->amount_of_vertices=ver->ver;
        opt->amount_of_vertices/=2;
        ver->data=NULL;
        free(ver);
    }

    if(generateTriangles(opt,opt->amount_of_vertices)){
        return 1;
    }

    return 0;
}


int generateTriangles(struct optimized_province *opt,int number_of_vertices){
    opt->amount_of_triangles=number_of_vertices/4*6;
    opt->triangles=malloc(sizeof(int)*opt->amount_of_triangles);
    if(!opt->triangles)return 1;

    int *temp=opt->triangles;

    for(int i=0,x=0;i<(opt->amount_of_triangles)/6;i++,x+=4){
        *(temp+(i*6)+0)=x+0;
        *(temp+(i*6)+1)=x+1;
        *(temp+(i*6)+2)=x+3;

        *(temp+(i*6)+3)=x+1;
        *(temp+(i*6)+4)=x+2;
        *(temp+(i*6)+5)=x+3;
    }
    return 0;
}


int addVertices(struct raw_vertices **ver,int *new_ver){
    if(!ver||!new_ver)return 1;
    if(!*ver){
        *ver=malloc(sizeof(struct raw_vertices));
        if(!*ver)return 1;
        (*ver)->ver=0;
        (*ver)->data=malloc(sizeof(int)*200);
        if(!(*ver)->data){
            free(*ver);
            return 1;
        }
        (*ver)->allocated_ver=200;
    }

    int *temp;

    if((*ver)->ver==(*ver)->allocated_ver){
        temp=realloc((*ver)->data,sizeof(int)*((*ver)->allocated_ver+200));
        if(!temp){
            free((*ver)->data);
            free(*ver);
            return 1;
        }
        else{
            (*ver)->data=temp;
            (*ver)->allocated_ver+=200;
        }
    }
    for(int i=0;i<8;i++) {
        *((*ver)->data + (*ver)->ver+i) = *(new_ver + i);
    }

    (*ver)->ver+=8;
    return 0;
}