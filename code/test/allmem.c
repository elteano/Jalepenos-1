/*
 * allmem.c
 *
 * A simple program that refernces all pages in memory.
 */


int Exec(char *, int, char **, int);
int Exit(int);
const int size = 1000;
int array[size];


void
inc_array(int n){

    int j=0;
    for(j=0; j< n; j++){
       array[j] = j;
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
    
    inc_array(size);

    verify_array(size);
    sum = add_array(size);
    verify_array(size);

    Exit(sum);
}