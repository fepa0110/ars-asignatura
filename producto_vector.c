# include <math.h>
# include <mpi.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>

int main ( int argc, char *argv[] );
void timestamp ( );

int main ( int argc, char *argv[] ){
    double escalar = 100.0;

    MPI_Status status;
    int ierr;
    int tag;
    int tag_done;
    int my_id;
    int master = 0;
    int num_procs;
    int dest;
    int k;
    int num_rows;
    int num_workers;
    int array_size;
    // int cant_cols;
    int dummy;
    double ans;

    double *array_row;
    double *array;
    double *result;

    ierr = MPI_Init ( &argc, &argv );

  if ( ierr != 0 ){
    printf ( "\n" );
    printf ( "MATVEC_MPI - Fatal error!\n" );
    printf ( "  MPI_Init returns nonzero IERR.\n" );
    exit ( 1 );
  }

/*
  Get this processor's ID.
*/
  ierr = MPI_Comm_rank ( MPI_COMM_WORLD, &my_id );
/*
  Get the number of processors.
*/
  ierr = MPI_Comm_size ( MPI_COMM_WORLD, &num_procs );

  if ( my_id == 0 ) 
  {
    timestamp ( );
    printf ( "\n" );
    printf ( "--> Autor: Fabricio Pafumi <--\n\n" );
    printf ( "-- Master process --\n" );
    printf ( "  Un programa para calcular la \n" );
    printf ( "  multiplicacion de una matrix por un escalar: \n" );
    printf ( "  result = escalar * array\n" );
    printf ( "\n" );
    printf ( "  Compilado en %s a las %s.\n", __DATE__, __TIME__ );
    printf ( "\n" );
    printf ( "  El numero de procesos es de %d.\n", num_procs );
  }
  printf ( "\n" );
  printf ( "Proceso %d esta activo.\n", my_id );

  array_size = 10;
  // cant_cols = 1;
  tag_done = array_size + 1;

  if ( my_id == 0 ) 
  {
    printf ( "\n" );
    printf ( "  El tamaño del vector es de %d.\n", array_size );
  }

  if ( my_id == master )
  {
    array = ( double * ) malloc ( array_size * sizeof ( double ) );
    result = ( double * ) malloc ( array_size * sizeof ( double ) );

    // Lleno el array con valores secuenciales
    k = 0;
    for (int indice = 1; indice <= array_size; indice++ ) 
    {
        array[k] = indice;
        k = k + 1;
    }


    printf ( "\n" );
    printf ( "-- Master process --\n" );
    printf ( "  Escalar: %f\n",escalar );
    printf ( "\n" );

  }
  else
  {
    array_row = ( double * ) malloc ( sizeof ( double ) );
  }

  // Envio el escalar a todos los procesos
  ierr = MPI_Bcast ( &escalar, 1, MPI_DOUBLE, master, MPI_COMM_WORLD );


  if ( my_id == master ){
    num_rows = 0;

    for (int process = 1; process <= num_procs-1; process++ )
    {
      dest = process;
      tag = num_rows;
      k = num_rows;

      ierr = MPI_Send ( array+k, 1, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD );

      num_rows = num_rows + 1;
    }

    num_workers = num_procs-1;

    for ( ; ; )
    {
      ierr = MPI_Recv ( &ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE,
        MPI_ANY_TAG, MPI_COMM_WORLD, &status );

      tag = status.MPI_TAG;
      result[tag] = ans;

      if ( num_rows < array_size ){
        num_rows = num_rows + 1;
        dest = status.MPI_SOURCE;
        tag = num_rows;
        k = num_rows;

        ierr = MPI_Send ( array+k, 1, MPI_DOUBLE, dest, tag, MPI_COMM_WORLD );
      }
      else{
        num_workers = num_workers - 1;
        dummy = 0;
        dest = status.MPI_SOURCE;
        tag = tag_done;

        ierr = MPI_Send ( &dummy, 1, MPI_INT, dest, tag, MPI_COMM_WORLD );

        if ( num_workers == 0 )
        {
          break;
        }
      }

    }

    free ( array );
  }
  else{
    for ( ; ; ){
      ierr = MPI_Recv ( array_row, 1, MPI_DOUBLE, master, MPI_ANY_TAG,
        MPI_COMM_WORLD, &status );

      tag = status.MPI_TAG;

      if ( tag == tag_done ) {
        printf ( "  Process %d shutting down.\n\n", my_id );
        break;
      }

      ans = 0.0;
      ans = escalar * array_row[0];

      ierr = MPI_Send ( &ans, 1, MPI_DOUBLE, master, tag, MPI_COMM_WORLD );

    }

    free ( array_row );
  }

  /*
    Print out the answer.
  */
  if ( my_id == master ) {
    printf ( "\n" );
    printf ( "-- Master process --\n" );
    printf ( "  Product vector result = escalar * array\n\n" );
    printf ( " - Escalar: %f -\n\n",escalar );
    printf ( " Fila\t| Resultado\n" );
    printf ( "\n" );
    for (int row = 0; row < array_size; row++ )
    {
      printf ( "%d\t| %f\n", row+1, result[row] );
    }

    free ( result );
  }

  ierr = MPI_Finalize ( );
/*
  Terminate.
*/
  if ( my_id == master ) {
    printf ( "\n" );
    printf ( "-- Master process --\n" );
    printf ( "  Normal end of execution.\n" );
    printf ( "\n" );
    timestamp ( );
  }
  return 0;

/*   for(int index = 0; index < 4; index++){
    result[index] = escalar * array[index]; 

    printf("indice %d: %d\n",index, result[index]);
  } */

}

void timestamp(){
  # define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  time_t now;

  now = time ( NULL );
  tm = localtime ( &now );

  strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm );

  printf ( "%s\n", time_buffer );

  return;
  # undef TIME_SIZE
}
