/*
 * somemem.c
 *
 * A simple program that refernces some pages in memory.
 */


int Exec(char *, int, char **, int);
int Exit(int);
const int size = 480;
int array[480];


void
fib_array(int n){

    int j=0;
    for(j=0; j< n; j++){
       if(j>2)
         array[j] = array[j- 1] + array[j-2];
       else if (j == 2)
         array[j] = 2;
       else if (j == 1)
         array[j] = 1;
       else
         array[j] = 0;
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
    
    fib_array(size);

    verify_array(size);
    sum = add_array(size);
    verify_array(size);

    Exit(sum);
}
