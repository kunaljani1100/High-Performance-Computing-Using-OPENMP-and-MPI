/*
   Calculating the value of pi using reduction : Serial Implementation
Author : Omkar Damle.
Date : August 2016.
 */

#include<stdio.h>
#include<math.h>
#include<omp.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>

//  Using the MONOTONIC clock 
#define CLK CLOCK_MONOTONIC

/* Function to compute the difference between two points in time */
struct timespec diff(struct timespec start, struct timespec end);

/* 
   Function to computes the difference between two time instances

   Taken from - http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/ 

   Further reading:
http://stackoverflow.com/questions/6749621/how-to-create-a-high-resolution-timer-in-linux-to-measure-program-performance
http://stackoverflow.com/questions/3523442/difference-between-clock-realtime-and-clock-monotonic
 */
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


unsigned char bi(unsigned char r1,unsigned char r2,unsigned char r3,unsigned char r4) 
{
   return (r1+r2+r3+r4)/4;
    
}

static PPMImage *readPPM(const char *filename)
{
  char buff[16];
  PPMImage *img;
  FILE *fp;
  int c, rgb_comp_color;
  //open PPM file for reading
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
int cmpfunc (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}
int max(int a, int b)
{
  if(a > b) return a;
  return b;
}
int min(int a, int b)
{
  if(a < b) return a;
  return b;
}
PPMImage* filterImage(PPMImage *im, int n)
{
  int rows = im->x;
  int cols = im->y;
  int i,j;
  int tx = rows / 2, ty = cols / 2;
  float PI = 3.141527f;
  float DRAD = PI / 180.0f;
  PPMImage *im2 = (PPMImage *) malloc(sizeof(PPMImage));
  im2->x = rows;
  im2->y = cols;
  im2->data = (PPMPixel*) malloc(rows*cols*sizeof(PPMPixel));
  int x = (2 * n  + 1) * (2 * n + 1) + 3;
  int r,g,b;
  #pragma omp parallel for private(i,j)
  for(i=0;i<rows;i++)
  {
    for(j=0; j<cols; j++)
    {

      int tmp1[150], tmp2[150], tmp3[150];
      int index = j + i * cols;
      int x1 = i - n;
      int y1 = j - n;
      int x2 = i + n;
      int y2 = j + n;
      int ni, nj, k = 0;
      
      for(ni = x1; ni <= x2; ni++)
      {
        for(nj = y1; nj <= y2; nj++)
        {
          int ii, jj;
          ii = ni; jj = nj;
          if(ii < 0) ii = 0;
          if(jj < 0) jj = 0;
          if(ii >= rows) ii = rows - 1;
          if(jj >= cols) jj = cols - 1;
          int in = ii * cols + jj;
          PPMPixel *t = im->data + in;
          tmp1[k] = t->red;
          tmp2[k] = t->green;
          tmp3[k++] = t->blue;
        }
      }
      qsort(tmp1, k, sizeof(int), cmpfunc);
      int med1 = tmp1[k / 2];
      qsort(tmp3, k, sizeof(int), cmpfunc);
      int med3 = tmp3[k / 2];
      qsort(tmp2, k, sizeof(int), cmpfunc);
      int med2 = tmp2[k / 2];
      PPMPixel *c = im2->data + index;
      c->red = med1;
      c->green = med2;
      c->blue = med3;
    } 
  }
  return im2;
}

int main(int argc, char* argv[])
{
	struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;
	/* Should start before anything else */
	clock_gettime(CLK, &start_e2e);

	/* Check if enough command-line arguments are taken in. */
	if(argc < 3){
		printf( "Usage: %s n p \n", argv[0] );
		return -1;
	}

	int n=atoi(argv[1]);	/* size of input array */
	int p=atoi(argv[2]);	/* number of processors*/
	char *problem_name = "median_filtering";
	char *approach_name = "qsort";
//	char buffer[10];
//	FILE* inputFile;
	FILE* outputFile;
	//	inputFile = fopen(argv[3],"r");

	char outputFileName[100];		
	sprintf(outputFileName,"output/%s_%s_%s_%s_output.txt",problem_name,approach_name,argv[1],argv[2]);	
	
	int number_of_threads = p;
    omp_set_num_threads(number_of_threads);
    char filename[1024];
    filename[0] ='\0';
    strcat(filename, argv[1]);
    strcat(filename, ".ppm");
    PPMImage *image;
	
	image = readPPM(filename);
	
	clock_gettime(CLK, &start_alg);	/* Start the algo timer */

	/*----------------------Core algorithm starts here----------------------------------------------*/

	 PPMImage *x = filterImage(image, p);

	/*----------------------Core algorithm finished--------------------------------------------------*/

	clock_gettime(CLK, &end_alg);	/* End the algo timer */
	/* Ensure that only the algorithm is present between these two
	   timers. Further, the whole algorithm should be present. */


	/* Should end before anything else (printing comes later) */
	/*char outputfilename[1024];
    outputfilename[0] ='\0';
    strcat(outputfilename, argv[1]);
    strcat(outputfilename, "_filtered");
    strcat(outputfilename, ".ppm");
    writePPM(outputfilename,x);
	*/
	clock_gettime(CLK, &end_e2e);
	e2e = diff(start_e2e, end_e2e);
	alg = diff(start_alg, end_alg);

	outputFile = fopen(outputFileName,"w");

	//fprintf(outputFile,"%.8f\n",pi);		

	/* problem_name,approach_name,n,p,e2e_sec,e2e_nsec,alg_sec,alg_nsec
	   Change problem_name to whatever problem you've been assigned
	   Change approach_name to whatever approach has been assigned
	   p should be 0 for serial codes!! 
	 */
	printf("%s,%s,%d,%d,%d,%ld,%d,%ld\n", problem_name, approach_name, n, p, e2e.tv_sec, e2e.tv_nsec, alg.tv_sec, alg.tv_nsec);

	return 0;

}

