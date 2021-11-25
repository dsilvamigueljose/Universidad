/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: Miguel Jose Da Silva Araujo
 *
 * Created on 8 de junio de 2021, 14:46
 */

#include<stdio.h>
#include<stdlib.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <time.h>

//Constantes
#define BLOCK_DIM 48
#define LOCAL_DIM 4 //Area local a buscar
#define VISUALIZAR 1 //Boton para indicar si se quiere visualizar el proceso de encontrar la mariquita (1 = encendido, 0 = apagado)

//Coordenadas y diferencia del bloque
int nuevoY, nuevoX, diff = INT_MAX;
int ultimoY;
int ultimoX;

//Prototipos de funciones
int compararBloques(int i, int j, IplImage* imagen1, IplImage* imagen2, IplImage* Mascara);
void ponerMarco(IplImage* frame, int ultimoX, int ultimoY);
void buscarProximidad (IplImage* frame, IplImage* Mask, IplImage* bugEnmascarado);
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
  
    // Create a window to display the video
    cvNamedWindow( argv[1], CV_WINDOW_AUTOSIZE );
  
    while( key != 'x' ) 
    {
        // get the image frame 
        frame = cvQueryFrame( capture );
  
        // exit if unsuccessful
        if( !frame ) break;
        
        //Iniciamos el reloj
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        
        buscarProximidad(frame, Mask, bugEnmascarado);
              
        //Volvemos a indicar la diferencia maxima para la proxima busqueda
        diff = INT_MAX;
        
        //Actualizamos a la nueva posicion del bloque
        ultimoY = nuevoY;
        ultimoX = nuevoX;
        
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
    int i, j, c1, c2, c3;
    
    for (i=0; i < Mask->height; i++){
        unsigned char *pMask = (unsigned char *) Mask->imageData + (i * Mask->widthStep);
        unsigned char *pImg = (unsigned char *) img->imageData + (i * img->widthStep);
        for (j=0; j < Mask->width; j++){
            c1 = *pImg++;
            c2 = *pImg++;
            c3 = *pImg++;
            if(c1==0xFF && c2==0xFF && c3==0xFF){
                *pMask++=0x00;
                *pMask++=0x00;
                *pMask++=0x00;
            }
            else{
                *pMask++=0xFF;
                *pMask++=0xFF;
                *pMask++=0xFF;
            }
        }
    }
    return Mask;
}

//Aplica una imagen mascara a otra imagen
void aplicarMascara(IplImage* img, IplImage* Mask){
    
    int i, j;
    
    for (i=0; i < img->height; i++){
        unsigned char *pMask = (unsigned char *) (Mask->imageData + (i * Mask->widthStep));
        unsigned char *pImg = (unsigned char *) (img->imageData + (i * img->widthStep));
        for (j=0; j < ((BLOCK_DIM*(img->nChannels))/16); j++){
            *pImg = *pImg & *pMask;
            pImg++; pMask++;
        }
    }
}

//Aplica una mascara Mascara al bloque i,j de imagen1 y lo compara con el bloque k,l de imagen2
int compararBloques(int i, int j, IplImage* imagenf, IplImage* imagenb, IplImage* Mascara){
    int fila, columna, iter, sumatorio = 0, aux;
    for(fila = 0; fila < BLOCK_DIM; fila++) {
        unsigned char *pImagf = (unsigned char *) ((imagenf->imageData + (fila+i) * imagenf->widthStep) + (j * (imagenf->nChannels)));
        unsigned char *pImagb = (unsigned char *) (imagenb->imageData + (fila) * imagenb->widthStep);
        unsigned char *pMascara = (unsigned char *)  (Mascara->imageData + (fila) * Mascara->widthStep);
        for(columna = 0; columna < BLOCK_DIM; columna++) {
            for(iter = 0; iter < 3; iter++){
                aux = *pImagf++ & *pMascara++;
                sumatorio += abs(aux - *pImagb++);
            }
        }
    }
    return sumatorio;
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
void buscarProximidad (IplImage* frame, IplImage* Mask, IplImage* bugEnmascarado){
    int i,j,aux;
    for(i = ultimoY - LOCAL_DIM; i < ultimoY + LOCAL_DIM; i++){
        for(j = ultimoX - LOCAL_DIM; j < ultimoX + LOCAL_DIM; j++){
            if(diff != 0){ //Si ya se ha encontrado el bloque igual, no se realizaran mas calculos
                aux = compararBloques(i, j, frame, bugEnmascarado, Mask);
                if(aux < diff){
                    //Actualizamos la ultima posicion del bloque mas parecido
                    nuevoY = i;
                    nuevoX = j;
                    diff = aux;
                }
            }
        }
    }
}
