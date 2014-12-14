/*
 * glocality.c
 *
 * A simple program that refernces to a small subset of pages.
 */


int Exec(char *, int, char **, int);
int Exit(int);

const int size = 2000;
int array[20000];



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


void
rnd_array(int n){

    int j=0;
    for(j=0; j< n; j++){
       array[j] = irand(0,20);
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
    
    rnd_array(size);

    verify_array(size);
    sum = add_array(size);
    verify_array(size);

    Exit(sum);
}
