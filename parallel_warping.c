#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define CLK CLOCK_MONOTONIC

struct timespec diff(struct timespec start, struct timespec end){
    struct timespec temp;
    if((end.tv_nsec-start.tv_nsec)<0){
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    }
    else{
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

typedef struct {
    unsigned char red,green,blue;
} PPMPixel;

typedef struct {
    int x, y;
    PPMPixel *data;
} PPMImage;

typedef struct {
    unsigned char gs;
} PPMPixelGS;


typedef struct {
    int x, y;
    PPMPixelGS *data;
} PPMImageGS;



#define RGB_COMPONENT_COLOR 255


void writePPMGS(const char *filename, PPMImageGS *img)
{
    FILE *fp;
    //open file for output
    fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //write the header file
    //image format
    fprintf(fp, "P5\n");



    //image size
    fprintf(fp, "%d %d\n",img->x,img->y);

    // rgb component depth
    fprintf(fp, "%d\n",RGB_COMPONENT_COLOR);

    // pixel data
    fwrite(img->data, img->x, img->y, fp);
    fclose(fp);
}


static PPMImage *readPPM(const char *filename)
{
    char buff[16];
    PPMImage *img;
    FILE *fp;
    int c, rgb_comp_color;
    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //read image format
    if (!fgets(buff, sizeof(buff), fp)) {
        perror(filename);
        exit(1);
    }

    //check the image format
    if (buff[0] != 'P' || buff[1] != '6') {
        fprintf(stderr, "Invalid image format (must be 'P6')\n");
        exit(1);
    }

    //alloc memory form image
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //check for comments
    c = getc(fp);
    while (c == '#') {
        while (getc(fp) != '\n') ;
        c = getc(fp);
    }

    ungetc(c, fp);
    //read image size information
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
        fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
        exit(1);
    }

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
        fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
        exit(1);
    }

    //check rgb component depth
    if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
        fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
        exit(1);
    }

    while (fgetc(fp) != '\n') ;
    //memory allocation for pixel data
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
        fprintf(stderr, "Unable to allocate memory\n");
        exit(1);
    }

    //read pixel data from file
    if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
        fprintf(stderr, "Error loading image '%s'\n", filename);
        exit(1);
    }

    fclose(fp);
    return img;
}

void writePPM(const char *filename, PPMImage *img)
{
    FILE *fp;
    //open file for output
    fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Unable to open file '%s'\n", filename);
        exit(1);
    }

    //write the header file
    //image format
    fprintf(fp, "P6\n");

    //comments


    //image size
    fprintf(fp, "%d %d\n",img->x,img->y);

    // rgb component depth
    fprintf(fp, "%d\n",255);

    // pixel data
    fwrite(img->data, 3 * img->x, img->y, fp);
    fclose(fp);
}

void BilinearlyInterpolate(PPMPixel *temp,PPMImage * im , int rows,int cols, float x, float y) 
{   
    if(x>=rows || x<0 || y>=cols || y<0)
        {
        return ;
    }
     
    unsigned char q11,q12,q21,q22;      
             
    float x1=floor(x),x2=ceil(x);
    float y1=floor(y),y2=ceil(y);
     
    float x2x1, y2y1, x2x, y2y, yy1, xx1;
    x2x1 = x2 - x1;
    y2y1 = y2 - y1;
    x2x = x2 - x;
    y2y = y2 - y;
    yy1 = y - y1;
    xx1 = x - x1;
    q11 = (im->data + (int)(floor(x)*rows + floor(y)))->red;
    q12 = (im->data + (int)(floor(x)*rows + ceil(y)))->red;
    q21 = (im->data + (int)(ceil(x)*rows + floor(y)))->red;
    q22 = (im->data + (int)(ceil(x)*rows + ceil(y)))->red;    
    temp->red= (unsigned char)1.0 / (x2x1 * y2y1) * (q11 * x2x * y2y + q21 * xx1 * y2y + q12 * x2x * yy1 + q22 * xx1 * yy1);
    q11 = (im->data + (int)(floor(x)*rows + floor(y)))->green;
    q12 = (im->data + (int)(floor(x)*rows + ceil(y)))->green;
    q21 = (im->data + (int)(ceil(x)*rows + floor(y)))->green;
    q22 = (im->data + (int)(ceil(x)*rows + ceil(y)))->green;
    temp->green= (unsigned char)1.0 / (x2x1 * y2y1) * (q11 * x2x * y2y + q21 * xx1 * y2y + q12 * x2x * yy1 + q22 * xx1 * yy1);
    q11 = (im->data + (int)(floor(x)*rows + floor(y)))->blue;
    q12 = (im->data + (int)(floor(x)*rows + ceil(y)))->blue;
    q21 = (im->data + (int)(ceil(x)*rows + floor(y)))->blue;
    q22 = (im->data + (int)(ceil(x)*rows + ceil(y)))->blue;
    temp->blue= (unsigned char)1.0 / (x2x1 * y2y1) * (q11 * x2x * y2y + q21 * xx1 * y2y + q12 * x2x * yy1 + q22 * xx1 * yy1);
     
}

PPMImage * transform(PPMImage * im)
{
 
  int rows = im->x;
  int cols = im->y;
  int i,j;
  int tx =rows/2 ,ty =cols/2;
  float radius ,theta ,PI = 3.141527f,DRAD = PI/180.0f;
 
  PPMImage *im2 = (PPMImage *) malloc(sizeof(PPMImage));
  im2->x = rows;
  im2->y = cols;
  im2->data = (PPMPixel *) malloc(rows*cols*sizeof(PPMPixel));
  int r,g,b,idx;
  float x,y;
 
  #pragma omp parallel for \
  shared(rows,cols,tx,ty,PI,DRAD)   \
  private(i,j,x,y,idx,radius,theta)
  for(i=0;i<rows;i++)
    {
      for(j=0; j<cols; j++)
    {
        idx = rows*i + j;
     
        radius = sqrtf((i - tx)*(i - tx) + (j - ty)*(j - ty));
        theta = (radius / 2)*DRAD;
        x = cos(theta) * (i - tx) - sin(theta) * (j - ty) + tx;
        y = sin(theta) * (i - tx) + cos(theta) * (j - ty) + ty;
        int idx2 = ceil(x)*rows + ceil(y);
        PPMPixel *temp2 = im2->data + idx;
        PPMPixel *temp = im->data + idx2;
        BilinearlyInterpolate(temp2,im,rows,cols,x,y);
    }   
    }
     
  return im2;
}

int main(int argc, char* argv[]) {

    struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;
    clock_gettime(CLK, &start_e2e);

    /* Check if enough command-line arguments are taken in. */
    if(argc < 3){
        printf( "Usage: %s n p \n", argv[0] );
        return -1;
    }

    int n = atoi(argv[1]);                  /* size of input array */
    int p = atoi(argv[2]);                  /* number of processors*/
    char *problem_name = "image_warping";
    char *approach_name = "data_divison";

    FILE* outputFile;
    char outputFileName[100];
    sprintf(outputFileName, "output/%s_%s_%s_%s_output.txt", problem_name, approach_name, argv[1], argv[2]);

    int number_of_threads = p;
    omp_set_num_threads(number_of_threads);
    char filename[1024];
    filename[0] ='\0';
    strcat(filename, argv[1]);
    strcat(filename, ".ppm");
    PPMImage *image;

    image = readPPM(filename);



    clock_gettime(CLK, &start_alg);                 /* Start the algo timer */
    PPMImageGS* x;// = change_image_warping(image);
    //----------------------------------------Algorithm Here------------------------------------------
	
	PPMImage * y = transform(image);

    //-----------------------------------------------------------------------------------------
    clock_gettime(CLK, &end_alg); /* End the algo timer */
    char outputfilename[1024];
    outputfilename[0] ='\0';
    strcat(outputfilename, argv[1]);
    strcat(outputfilename, "_warped");
    strcat(outputfilename, ".ppm");
    writePPM(outputfilename,y);

    clock_gettime(CLK, &end_e2e);
    e2e = diff(start_e2e, end_e2e);
    alg = diff(start_alg, end_alg);
    outputFile = fopen(outputFileName,"w");
    printf("%s,%s,%d,%d,%ld,%ld,%ld,%ld\n", problem_name, approach_name, n, p, e2e.tv_sec, e2e.tv_nsec, alg.tv_sec, alg.tv_nsec);
    return 0;
}

