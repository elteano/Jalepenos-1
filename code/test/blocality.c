/*
 * blocality.c
 *
 * A simple program that refernces pages with bad locality
 */

unsigned int seed;

void
irand_init()
{
  seed = 93186752;
}

/* a very poor random number generator */
unsigned int
irand(int l, int h)
{
  unsigned int a = 1588635695, m = 429496U, q = 2, r = 1117695901;
  unsigned int val;

  seed = a*(seed % q) - r*(seed / q);
  val = (seed / m) % (h - l) + l;
 
  return val;
}

int Exec(char *, int, char **, int);
int Exit(int);
int inc = 0;

/*int array[48] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
		 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
		 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		 41, 42, 43, 44, 45, 46, 47};
*/

int array[2000];


void
rnd_array(int n){

    int j=0;
    int k=0;


    for(k=0; k< 10; k++){
    for(j=0; j< n; j++){
       array[j] = j;
    }
    }

}

void
verify_array(int n)
{
    int i;

    for (i = 0; i < n; i++) {
	if (array[i] != i) {
	    Exit(-1);
	}
    }
}

int
add_array(int n)
{
    int i, sum;

    sum = 0;
    for (i = 0; i < n; i++) {
	sum += array[i];
    }
    return sum;
}

int
main ()
{
    int sum;
    
    rnd_array(200);

    verify_array(200);
    sum = add_array(200);
    verify_array(200);

    Exit(sum);
}
