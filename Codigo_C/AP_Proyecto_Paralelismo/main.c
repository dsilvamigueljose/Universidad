/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: Miguel Jose Da Silva Araujo
 *
 * Created on 1 de mayo de 2021, 16:44
 */

#include<stdio.h>
#include<stdlib.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <emmintrin.h>

#include <pthread.h> 

#include <time.h>

//Constantes
#define BLOCK_DIM 48
#define LOCAL_DIM 4 //Area local a buscar
#define NTHREADS 2
#define VISUALIZAR 1 //Boton para indicar si se quiere visualizar el proceso de encontrar la mariquita (1 = encendido, 0 = apagado)

//Coordenadas y diferencia del bloque
int diff = INT_MAX;
int ultimoY;
int ultimoX;

//Estructura de paso de datos al thread
struct estructura {
    IplImage *frame;
    IplImage *bugEnmascarado;
    IplImage *Mask;
    int fila;
    int copiaY;
    int copiaX;
};

//Prototipos de funciones
int compararBloques(int i, int j, IplImage* imagen1, IplImage* imagen2, IplImage* Mascara);
void ponerMarco(IplImage* frame, int ultimoX, int ultimoY);
void buscarProximidad (void* ptr);
IplImage* realizarMascara(IplImage* img);
void aplicarMascara(IplImage* img, IplImage* mask);


int main(int argc, char** argv) {
    
    //Variables de medicion de tiempo
    struct timespec start, finish;
    float elapsed, total = 0.0;
    
    //INICIALIZAMOS EL VIDEO Y LA IMAGEN/////////////////////////////////////////////////////
    
    int key = 0;
    
    // Initialize camera or OpenCV image
    CvCapture* capture = cvCaptureFromAVI( argv[1] );  
    
    // Check 
    if ( !capture ) 
    {
        fprintf( stderr, "Cannot open AVI!\n" );
        return EXIT_FAILURE;
    }
    
    //Frame del video
    IplImage *frame = cvQueryFrame( capture );
    
    //Check
    if(!frame) {
        printf("Error: frame no cargado");
        return EXIT_FAILURE;
    }
    
    //Imagen de la mariquita
    IplImage *bug = cvLoadImage(argv[2], CV_LOAD_IMAGE_UNCHANGED);

    //Check
    if(!bug) {
        printf("Error: fichero %s no leido\n", argv[2]);
        return EXIT_FAILURE;
    }
    
    //Inicializamos en la primera posicion del objeto (mariquita)
    ultimoY = ((frame->height/2)-23);
    ultimoX = ((frame->width/2)-25);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    
    
    
    
    //AQUI HAGO LA MASCARA/////////////////////////////////////////////////////////////////////////////////////////
     
    IplImage *Mask = realizarMascara(bug);
    
    //Creo una imagen de la mariquita con la mascara aplicada para no tener que volversela a aplicar cada vez que realize la comparacion del bloque
    IplImage *bugEnmascarado = cvCloneImage(bug);
    aplicarMascara(bugEnmascarado, Mask);
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    //CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO 
    
    int i,j;
    
    // Get the fps, needed to set the delay
    int fps = ( int )cvGetCaptureProperty( capture, CV_CAP_PROP_FPS );
    
    //Inicializamos variables y estructura de control de los threads
    pthread_t threads[NTHREADS];
    struct estructura miEstructura[NTHREADS];
  
    // Create a window to display the video
    cvNamedWindow( argv[1], CV_WINDOW_AUTOSIZE );
    
    int copiaY;
    int copiaX;
  
    while( key != 'x' ) 
    {
        // get the image frame 
        frame = cvQueryFrame( capture );
  
        // exit if unsuccessful
        if( !frame ) break;
        
        copiaY = ultimoY;
        copiaX = ultimoX;
        
        //Iniciamos el reloj
        clock_gettime(CLOCK_MONOTONIC, &start);

        //THREADS///////////////////////////////////////////////////////////////////////////
        for(i = 0; i < NTHREADS; i++) {
            miEstructura[i].Mask = Mask;
            miEstructura[i].bugEnmascarado = bugEnmascarado;
            miEstructura[i].frame = frame;
            miEstructura[i].fila = i;
            miEstructura[i].copiaX = copiaX;
            miEstructura[i].copiaY = copiaY;
            
            pthread_create(&threads[i], NULL, (void*) &buscarProximidad, (void*) &miEstructura[i]);
        }
        for(i = 0; i < NTHREADS; i++) {
            pthread_join(threads[i], NULL);
        }
        ////////////////////////////////////////////////////////////////////////////////////
        
        //Volvemos a indicar la diferencia maxima para la proxima busqueda
        diff = INT_MAX;
        
        //Actualizamos a la nueva posicion del bloque
/*
        ultimoY = nuevoY;
        ultimoX = nuevoX;
*/
        
        //Señalamos la posicion del objeto con un marco
        ponerMarco(frame, ultimoX, ultimoY);
        
        //Paramos el reloj y sumamos el tiempo al total
        clock_gettime(CLOCK_MONOTONIC, &finish);
        elapsed = (finish.tv_sec - start.tv_sec);
        total += elapsed + ((finish.tv_nsec - start.tv_nsec) / 1000000000.0);
        
        
        if(VISUALIZAR){
            // display current frame
            cvShowImage( argv[1], frame );

            // exit if user presses 'x'        
            key = cvWaitKey( 1000 / fps );
        }

    }
    
    //Mostramos el tiempo transcurrido en los calculos para encontrar el bloque
    printf("Tiempo transcurrido en calculos: %f\n", total);
    
    
    //CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO CODIGO 
    
    cvWaitKey(0);
    
    //Destruimos la ventana del video
    cvDestroyWindow(argv[1]);
    
    //Liberamos la memoria asignada a las imagenes
    cvReleaseImage(&frame);
    cvReleaseImage(&bug);
    cvReleaseImage(&Mask);
    cvReleaseImage(&bugEnmascarado);
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    return(EXIT_SUCCESS);
    
}

//Crea una imagen mascara (convierte pixeles blancos a negros)
IplImage* realizarMascara(IplImage* img){
    IplImage *Mask = cvCloneImage(img);
    int i, j;
    
    for (i=0; i < img->height; i++){
        __m128i *pMask = (__m128i *) (Mask->imageData + (i * Mask->widthStep));
        __m128i registro;
        __m128i BLANCO = _mm_set1_epi8(0xFF);
        for (j=0; j < ((BLOCK_DIM*(img->nChannels))/16); j++){
            registro = _mm_load_si128(pMask);
            registro = _mm_andnot_si128(registro,BLANCO);
            _mm_store_si128(pMask++,registro);
        }
    }
    return Mask;
}

//Aplica una imagen mascara a otra imagen
void aplicarMascara(IplImage* img, IplImage* Mask){
    
    int i, j;
    
    for (i=0; i < img->height; i++){
        __m128i *pMask = (__m128i *) (Mask->imageData + (i * Mask->widthStep));
        __m128i *pImg = (__m128i *) (img->imageData + (i * img->widthStep));
        __m128i registro1, registro2;
        for (j=0; j < ((BLOCK_DIM*(img->nChannels))/16); j++){
            registro1 = _mm_loadu_si128(pImg);
            registro2 = _mm_loadu_si128(pMask++);
            registro1 = _mm_and_si128(registro1,registro2);
            _mm_storeu_si128(pImg++,registro1);
        }
    }
}

//Aplica una mascara Mascara al bloque i,j de imagen1 y lo compara con el bloque k,l de imagen2
int compararBloques(int i, int j, IplImage* imagen1, IplImage* imagen2, IplImage* Mascara){
    int fila, columna, iter, sad = 0;
    __m128i rA, rB, rC, rD, rE, rF, rM;
    rD = _mm_set1_epi8(0); //Setear a 0 todos los bytes
    for(fila = 0; fila < BLOCK_DIM; fila++) {
        __m128i *pImag1 = (__m128i*) ((imagen1->imageData + (fila+i) * imagen1->widthStep) + (j * (imagen1->nChannels)));
        __m128i *pImag2 = (__m128i*) (imagen2->imageData + (fila) * imagen2->widthStep);
        __m128i *pMascara = (__m128i*)  (Mascara->imageData + (fila) * Mascara->widthStep);
        for(columna = 0; columna < ((BLOCK_DIM*(imagen2->nChannels))/16); columna++) {
            rM = _mm_load_si128(pMascara++);
            rA = _mm_loadu_si128(pImag1++);
            rA = _mm_and_si128(rA,rM);
            rB = _mm_load_si128(pImag2++);
            rC = _mm_sad_epu8(rA, rB);
            rD = _mm_add_epi32(rD, rC);
        }
    }
    rE = _mm_srli_si128(rD,8);
    rF = _mm_add_epi32(rD,rE);
    sad = _mm_cvtsi128_si32(rF);
    return sad;
}

//Pone el marco verde en el bloque perteneciente al objeto (mariquita)
//Param: frame -> imagen a aplicar el marco; ultimoX -> coordenadas x (columna) del bloque; ultimoY -> coordenadas y (fila) del bloque
void ponerMarco(IplImage* frame, int ultimoX, int ultimoY){

    int i,j;
    
    for (i=0; i < BLOCK_DIM; i++){
        unsigned char *pImg2 = (unsigned char *) frame->imageData + ((i+ultimoY) * frame->widthStep) + (ultimoX * (frame->nChannels));
        for (j=0; j < BLOCK_DIM; j++){
            if(i==0 || i == BLOCK_DIM - 1 || j == 0 || j == BLOCK_DIM - 1){
                //Se le coloca el color verde para que sea de facil visualizacion
                *pImg2++=0x00;
                *pImg2++=0xFF;
                *pImg2++=0x00;
            }
            else{
                pImg2++;
                pImg2++;
                pImg2++;
            }             
        }
    }
}

//Barrido de la malla local al ultimo bloque encontrado buscando el bloque mas parecido
void buscarProximidad (void* ptr){
    int i,j,aux;
    struct estructura estr = (*(struct estructura *) ptr); 
    for(i = estr.fila + (estr.copiaY - LOCAL_DIM); i < estr.copiaY + LOCAL_DIM && diff !=0; i+=NTHREADS){
        for(j = estr.copiaX - LOCAL_DIM; j < estr.copiaX + LOCAL_DIM && diff !=0; j++){
            if(diff != 0){ //Si ya se ha encontrado el bloque igual, no se realizaran mas calculos
                aux = compararBloques(i, j, estr.frame, estr.bugEnmascarado, estr.Mask);
                if(aux == 0){
                    //Actualizamos la ultima posicion del bloque igual
                    ultimoY = i;
                    ultimoX = j;
                    diff = aux;
                }
            }
        }
    }
}