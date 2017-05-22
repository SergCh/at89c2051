//-----------bin2h-------------
#include <stdio.h>

int main(int argc, char* argv[] ){

  int i,l;

  int first=1; 

  FILE *stream;

  unsigned char buf[16];

  if (argc<2) {
     printf("Need parater for file with data\n");
     return 1;
  }

  if ((stream=fopen(argv[1],"rb"))==NULL) {
     printf("Error open file %s",argv[1]);
     return 1;
  }


  do {
     l=fread(buf, 1, sizeof(buf), stream);
     for (i=0; i<l; i++) {

        if (!first) {
           printf(",");
             
	   if (i==0)   
              printf ("\n");
        } 

        printf("0x%02X", buf[i]);

        first=0;
     }

  } while (l!=0);

  printf ("\n");
  fclose(stream);
  return 0;
}
