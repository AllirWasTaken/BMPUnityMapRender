#pragma once

#pragma pack(1)

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <time.h>


struct header{
    uint16_t signature;
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t data_offset;
    uint32_t header_size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size_bytes;
    uint32_t x_resolution;
    uint32_t y_resolution;
    uint32_t number_of_colors;
    uint32_t important_colors;
};

struct pixel{
    uint8_t blue,green,red;
};

struct image{
    struct header *header;
    struct pixel *data;
};

struct render_data{
    int amount_of_provinces;
    struct optimized_province *optimized_provinces;
};

struct province{
    int amount_of_rectangles;
    struct rectangle *rectangles;
};

struct rectangle{
    int x1,y1;
};

struct stack{
    int size_of_stack;
    int allocated_int;
    int *stack_data;
};

struct render_data_pack{
    int number_of_data_slots;
    struct render_data *data;
};

struct multi_thread_pixels{
    struct image *img;
    int number_of_threads;
    int slots_per_province; //slots*number_of_threads=max province count
    int finished;
    int freeID;
    struct pixel *colors;
    int *positions;
    int *processed_pixels;
    struct render_data_pack *pack;
};

struct optimized_province{
    int width;
    int height;
    int middleX;
    int middleY;
    int amount_of_vertices;
    int amount_of_triangles;
    int *vertices;
    int *triangles;
};


struct raw_vertices{
    int allocated_ver;
    int ver;
    int* data;
};


//They Work
struct image *loadBmp(char* filename, int* error_code);
void freeImage(struct image **img);
void freeStack(struct stack **stk);
void saveBmp(struct image *img, char* filename,int *error_code);
int checkIfPixelsAreSame(struct pixel *pix1,struct pixel *pix2);
int scanPixelsAroundPosition(struct image *img,int position,struct stack **stk1,struct stack **stk2,struct pixel colors);
void turnPixelBlack(struct image *img, int position);
void *showStats(struct multi_thread_pixels *threading);
void findPixelsToProcess(struct multi_thread_pixels *threading);
void *singleThreadProvinceRender(struct multi_thread_pixels *threads);
struct render_data_pack *makeRenderData(struct image *img,int *error, struct multi_thread_pixels *threading);
void freeThreading(struct multi_thread_pixels **threads);
struct rectangle *translateToRectangles(struct stack **stk, struct image *img);
struct province *makeProvinceData(struct image *img,int *error_code,int position,int *amount_of_processed_pixels);


//Untested
struct optimized_province *optimizeProvince(struct province **prov);
int calculateSizeVariables(struct province *prov,struct optimized_province *opt, int *maxLHP);
int redrawProvinceAndCalculateVerticesTriangles(struct province* prov, struct optimized_province *opt,int *max_left_height);
int increaseValuesAwayFromBorder(int **array,int height,int width);
int checkValuesAround(int **array,int y,int x,int value,int height,int width);
int checkPixel(int **array,int x,int y);
int checkLine(int **array,int x,int y,int type,int length);
int *generateSingleVerticesPack(int **array,int x,int y, int max_x,int max_y);
void clearSpaceInsideRectangle(int **array,int x,int y,int width,int height);
int findMaxValueInArray(int **array,int max_x,int max_y,int *return_x,int *return_y);
int addVertices(struct raw_vertices **ver,int *new_ver);
int generateTriangles(struct optimized_province *opt,int number_of_vertices);
void freeRenderDataPack(struct render_data_pack **data);
int saveRenderData(struct render_data_pack **data,char* filename,int *error_code);


//Rewrite them




//Needs Optimization - trying to optimize broke code 3 times :) I give up
int addToStack(struct stack **stk,int *number);
int removeFromStack(struct stack **stk,int *number);


//Undone


//Not working


//debug functions
void testSaveProvince(struct province *prov);
void showArray(int **array,int size_x,int size_y);
